#include "QGCMemoryPool.h"

#include <QtCore/QMutexLocker>

#include <algorithm>
#include <cstring>

Q_LOGGING_CATEGORY(QGCMemoryPoolLog, "Utilities.QGCMemoryPool")

QGCMemoryPool::QGCMemoryPool(size_t blockSize, size_t poolSize, QObject *parent)
    : QObject(parent)
    , _blockSize(std::max(blockSize, sizeof(void *)))  // Minimum size to hold a pointer
    , _poolSize(poolSize)
    , _freeCount(poolSize)
{
    // Allocate the pool
    _memory = std::make_unique<char[]>(_blockSize * _poolSize);

    // Initialize free list
    _initFreeList();

    qCDebug(QGCMemoryPoolLog) << "Created pool: blockSize=" << _blockSize
                               << "poolSize=" << _poolSize
                               << "totalBytes=" << (_blockSize * _poolSize);
}

QGCMemoryPool::~QGCMemoryPool()
{
    qCDebug(QGCMemoryPoolLog) << "Destroyed pool: totalAllocs=" << _totalAllocations
                               << "totalDeallocs=" << _totalDeallocations;
}

void *QGCMemoryPool::allocate()
{
    QMutexLocker locker(&_mutex);

    if (_freeList == nullptr) {
        qCWarning(QGCMemoryPoolLog) << "Pool exhausted";
        emit poolExhausted();
        return nullptr;
    }

    // Pop from free list
    void *block = _freeList;
    void *next = nullptr;
    std::memcpy(&next, block, sizeof(void *));
    _freeList = next;

    --_freeCount;
    ++_totalAllocations;

    const size_t used = _poolSize - _freeCount;
    if (used > _peakUsage) {
        _peakUsage = used;
    }

    emit statsChanged();
    return block;
}

void QGCMemoryPool::deallocate(void *ptr)
{
    if (!ptr) {
        return;
    }

    QMutexLocker locker(&_mutex);

    if (!contains(ptr)) {
        qCWarning(QGCMemoryPoolLog) << "Attempted to deallocate pointer not from this pool";
        return;
    }

    // Push onto free list
    std::memcpy(ptr, &_freeList, sizeof(void *));
    _freeList = ptr;

    ++_freeCount;
    ++_totalDeallocations;

    emit statsChanged();
}

bool QGCMemoryPool::contains(void *ptr) const
{
    if (!ptr || !_memory) {
        return false;
    }

    const char *charPtr = static_cast<char *>(ptr);
    const char *poolStart = _memory.get();
    const char *poolEnd = poolStart + (_blockSize * _poolSize);

    if (charPtr < poolStart || charPtr >= poolEnd) {
        return false;
    }

    // Check alignment
    const size_t offset = static_cast<size_t>(charPtr - poolStart);
    return (offset % _blockSize) == 0;
}

size_t QGCMemoryPool::freeBlocks() const
{
    QMutexLocker locker(&_mutex);
    return _freeCount;
}

size_t QGCMemoryPool::usedBlocks() const
{
    QMutexLocker locker(&_mutex);
    return _poolSize - _freeCount;
}

bool QGCMemoryPool::isEmpty() const
{
    QMutexLocker locker(&_mutex);
    return _freeCount == _poolSize;
}

bool QGCMemoryPool::isFull() const
{
    QMutexLocker locker(&_mutex);
    return _freeCount == 0;
}

QGCMemoryPool::Stats QGCMemoryPool::stats() const
{
    QMutexLocker locker(&_mutex);

    Stats s;
    s.blockSize = _blockSize;
    s.poolSize = _poolSize;
    s.freeBlocks = _freeCount;
    s.usedBlocks = _poolSize - _freeCount;
    s.peakUsage = _peakUsage;
    s.totalAllocations = _totalAllocations;
    s.totalDeallocations = _totalDeallocations;

    return s;
}

void QGCMemoryPool::resetStats()
{
    QMutexLocker locker(&_mutex);
    _peakUsage = _poolSize - _freeCount;
    _totalAllocations = 0;
    _totalDeallocations = 0;
}

void QGCMemoryPool::clear()
{
    QMutexLocker locker(&_mutex);

    _initFreeList();
    _freeCount = _poolSize;

    emit statsChanged();
    qCDebug(QGCMemoryPoolLog) << "Pool cleared";
}

bool QGCMemoryPool::grow(size_t additionalBlocks)
{
    if (additionalBlocks == 0) {
        return false;
    }

    QMutexLocker locker(&_mutex);

    const size_t newPoolSize = _poolSize + additionalBlocks;
    auto newMemory = std::make_unique<char[]>(_blockSize * newPoolSize);

    // Copy existing data
    std::memcpy(newMemory.get(), _memory.get(), _blockSize * _poolSize);

    // Build free list for new blocks
    char *newBlockStart = newMemory.get() + (_blockSize * _poolSize);
    for (size_t i = 0; i < additionalBlocks - 1; ++i) {
        char *current = newBlockStart + (i * _blockSize);
        char *next = current + _blockSize;
        std::memcpy(current, &next, sizeof(void *));
    }

    // Last new block points to old free list
    char *lastNew = newBlockStart + ((additionalBlocks - 1) * _blockSize);

    // Recalculate old free list pointers to new memory
    if (_freeList) {
        const ptrdiff_t offset = newMemory.get() - _memory.get();
        void *current = _freeList;

        while (current) {
            void *next = nullptr;
            std::memcpy(&next, current, sizeof(void *));

            if (next) {
                char *adjustedNext = static_cast<char *>(next) + offset;
                std::memcpy(current, &adjustedNext, sizeof(void *));
            }

            current = next;
        }

        // Adjust freeList pointer
        char *adjustedFreeList = static_cast<char *>(_freeList) + offset;
        std::memcpy(lastNew, &adjustedFreeList, sizeof(void *));
    } else {
        void *null = nullptr;
        std::memcpy(lastNew, &null, sizeof(void *));
    }

    // Update free list to start with new blocks
    _freeList = newBlockStart;

    _memory = std::move(newMemory);
    _poolSize = newPoolSize;
    _freeCount += additionalBlocks;

    emit poolGrown(newPoolSize);
    emit statsChanged();

    qCInfo(QGCMemoryPoolLog) << "Pool grown by" << additionalBlocks << "blocks to" << newPoolSize;
    return true;
}

void QGCMemoryPool::_initFreeList()
{
    _freeList = _memory.get();

    for (size_t i = 0; i < _poolSize - 1; ++i) {
        char *current = _memory.get() + (i * _blockSize);
        char *next = current + _blockSize;
        std::memcpy(current, &next, sizeof(void *));
    }

    // Last block points to null
    char *last = _memory.get() + ((_poolSize - 1) * _blockSize);
    void *null = nullptr;
    std::memcpy(last, &null, sizeof(void *));
}
