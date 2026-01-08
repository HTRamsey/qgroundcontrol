#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMutex>
#include <QtCore/QLoggingCategory>

#include <memory>

Q_DECLARE_LOGGING_CATEGORY(QGCMemoryPoolLog)

/// Fixed-size memory pool for efficient allocation of same-size objects.
///
/// Useful for reducing allocation overhead when creating many small objects
/// of the same type (e.g., telemetry data points, waypoints).
///
/// Example usage:
/// @code
/// QGCMemoryPool pool(sizeof(TelemetryPoint), 1000);  // 1000 pre-allocated slots
///
/// void* mem = pool.allocate();
/// auto* point = new (mem) TelemetryPoint();
/// // Use point...
/// point->~TelemetryPoint();
/// pool.deallocate(mem);
/// @endcode
///
/// Or with the typed wrapper:
/// @code
/// QGCTypedMemoryPool<TelemetryPoint> pool(1000);
/// TelemetryPoint* point = pool.construct(args...);
/// // Use point...
/// pool.destroy(point);
/// @endcode
class QGCMemoryPool : public QObject
{
    Q_OBJECT
    Q_PROPERTY(size_t blockSize READ blockSize CONSTANT)
    Q_PROPERTY(size_t poolSize READ poolSize CONSTANT)
    Q_PROPERTY(size_t freeBlocks READ freeBlocks NOTIFY statsChanged)
    Q_PROPERTY(size_t usedBlocks READ usedBlocks NOTIFY statsChanged)

public:
    struct Stats {
        size_t blockSize;
        size_t poolSize;
        size_t freeBlocks;
        size_t usedBlocks;
        size_t peakUsage;
        size_t totalAllocations;
        size_t totalDeallocations;
    };

    explicit QGCMemoryPool(size_t blockSize, size_t poolSize, QObject *parent = nullptr);
    ~QGCMemoryPool() override;

    // Disable copy
    QGCMemoryPool(const QGCMemoryPool &) = delete;
    QGCMemoryPool &operator=(const QGCMemoryPool &) = delete;

    // Allocation
    void *allocate();
    void deallocate(void *ptr);
    bool contains(void *ptr) const;

    // Pool info
    size_t blockSize() const { return _blockSize; }
    size_t poolSize() const { return _poolSize; }
    size_t freeBlocks() const;
    size_t usedBlocks() const;
    bool isEmpty() const;
    bool isFull() const;

    // Statistics
    Stats stats() const;
    void resetStats();

    // Pool management
    void clear();  // Marks all blocks as free (doesn't call destructors!)
    bool grow(size_t additionalBlocks);  // Expand pool if possible

signals:
    void statsChanged();
    void poolExhausted();
    void poolGrown(size_t newSize);

private:
    void _initFreeList();

    std::unique_ptr<char[]> _memory;
    void *_freeList = nullptr;
    size_t _blockSize;
    size_t _poolSize;
    size_t _freeCount;
    size_t _peakUsage = 0;
    size_t _totalAllocations = 0;
    size_t _totalDeallocations = 0;
    mutable QMutex _mutex;
};

/// Typed memory pool wrapper for type-safe allocation.
template <typename T>
class QGCTypedMemoryPool
{
public:
    explicit QGCTypedMemoryPool(size_t poolSize, QObject *parent = nullptr)
        : _pool(sizeof(T), poolSize, parent)
    {
    }

    template <typename... Args>
    T *construct(Args &&...args)
    {
        void *mem = _pool.allocate();
        if (!mem) {
            return nullptr;
        }
        return new (mem) T(std::forward<Args>(args)...);
    }

    void destroy(T *ptr)
    {
        if (ptr && _pool.contains(ptr)) {
            ptr->~T();
            _pool.deallocate(ptr);
        }
    }

    QGCMemoryPool &pool() { return _pool; }
    const QGCMemoryPool &pool() const { return _pool; }

    size_t freeBlocks() const { return _pool.freeBlocks(); }
    size_t usedBlocks() const { return _pool.usedBlocks(); }
    bool isEmpty() const { return _pool.isEmpty(); }
    bool isFull() const { return _pool.isFull(); }

private:
    QGCMemoryPool _pool;
};
