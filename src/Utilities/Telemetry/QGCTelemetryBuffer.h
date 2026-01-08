#pragma once

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(QGCTelemetryBufferLog)

/// Ring buffer for time-series telemetry data with statistics.
///
/// Features:
/// - Fixed-size ring buffer with automatic overflow
/// - Timestamped entries
/// - Real-time statistics (min, max, avg, stddev)
/// - Thread-safe operations
/// - Downsampling support for display
///
/// Example usage:
/// @code
/// QGCTelemetryBuffer buffer(1000);  // 1000 samples
/// buffer.append(25.5);
/// buffer.append(26.0);
/// auto stats = buffer.statistics();
/// qDebug() << "Average:" << stats.average;
/// @endcode
template<typename T>
class QGCTelemetryBuffer
{
public:
    struct Entry {
        T value;
        qint64 timestamp;  // msecs since epoch
    };

    struct Statistics {
        T min{};
        T max{};
        double average = 0.0;
        double stddev = 0.0;
        int count = 0;
        qint64 firstTimestamp = 0;
        qint64 lastTimestamp = 0;
        double ratePerSecond = 0.0;
    };

    explicit QGCTelemetryBuffer(int capacity = 1000)
        : _capacity(capacity)
    {
        _buffer.reserve(capacity);
    }

    /// Append a new value with current timestamp
    void append(const T &value)
    {
        append(value, QDateTime::currentMSecsSinceEpoch());
    }

    /// Append a new value with specific timestamp
    void append(const T &value, qint64 timestamp)
    {
        QMutexLocker locker(&_mutex);

        Entry entry{value, timestamp};

        if (_buffer.size() < _capacity) {
            _buffer.append(entry);
        } else {
            _buffer[_head] = entry;
        }

        _head = (_head + 1) % _capacity;
        _totalCount++;

        _updateStats(value);
    }

    /// Get the most recent value
    T latest() const
    {
        QMutexLocker locker(&_mutex);
        if (_buffer.isEmpty()) {
            return T{};
        }
        const int lastIdx = (_head - 1 + _buffer.size()) % _buffer.size();
        return _buffer[lastIdx].value;
    }

    /// Get value at index (0 = oldest, size-1 = newest)
    T at(int index) const
    {
        QMutexLocker locker(&_mutex);
        if (index < 0 || index >= _buffer.size()) {
            return T{};
        }
        const int actualIdx = (_head - _buffer.size() + index + _capacity) % _capacity;
        return _buffer[actualIdx].value;
    }

    /// Get entry at index with timestamp
    Entry entryAt(int index) const
    {
        QMutexLocker locker(&_mutex);
        if (index < 0 || index >= _buffer.size()) {
            return {};
        }
        const int actualIdx = (_head - _buffer.size() + index + _capacity) % _capacity;
        return _buffer[actualIdx];
    }

    /// Get all entries as a list (oldest first)
    QVector<Entry> entries() const
    {
        QMutexLocker locker(&_mutex);
        QVector<Entry> result;
        result.reserve(_buffer.size());

        for (int i = 0; i < _buffer.size(); ++i) {
            const int idx = (_head - _buffer.size() + i + _capacity) % _capacity;
            result.append(_buffer[idx]);
        }
        return result;
    }

    /// Get all values as a list (oldest first)
    QVector<T> values() const
    {
        QMutexLocker locker(&_mutex);
        QVector<T> result;
        result.reserve(_buffer.size());

        for (int i = 0; i < _buffer.size(); ++i) {
            const int idx = (_head - _buffer.size() + i + _capacity) % _capacity;
            result.append(_buffer[idx].value);
        }
        return result;
    }

    /// Get downsampled values for display
    QVector<T> downsample(int targetCount) const
    {
        QMutexLocker locker(&_mutex);

        if (_buffer.isEmpty() || targetCount <= 0) {
            return {};
        }

        if (_buffer.size() <= targetCount) {
            return values();
        }

        QVector<T> result;
        result.reserve(targetCount);

        const double step = static_cast<double>(_buffer.size()) / targetCount;
        for (int i = 0; i < targetCount; ++i) {
            const int idx = static_cast<int>(i * step);
            const int actualIdx = (_head - _buffer.size() + idx + _capacity) % _capacity;
            result.append(_buffer[actualIdx].value);
        }

        return result;
    }

    /// Get entries within a time range
    QVector<Entry> entriesInRange(qint64 startTime, qint64 endTime) const
    {
        QMutexLocker locker(&_mutex);
        QVector<Entry> result;

        for (int i = 0; i < _buffer.size(); ++i) {
            const int idx = (_head - _buffer.size() + i + _capacity) % _capacity;
            const Entry &entry = _buffer[idx];
            if (entry.timestamp >= startTime && entry.timestamp <= endTime) {
                result.append(entry);
            }
        }

        return result;
    }

    /// Get current statistics
    Statistics statistics() const
    {
        QMutexLocker locker(&_mutex);

        Statistics stats;
        stats.count = _buffer.size();

        if (_buffer.isEmpty()) {
            return stats;
        }

        // Calculate all stats from scratch for accuracy
        double sum = 0.0;
        stats.min = _buffer[0].value;
        stats.max = _buffer[0].value;
        stats.firstTimestamp = _buffer[0].timestamp;
        stats.lastTimestamp = _buffer[0].timestamp;

        for (const Entry &entry : _buffer) {
            sum += static_cast<double>(entry.value);
            if (entry.value < stats.min) stats.min = entry.value;
            if (entry.value > stats.max) stats.max = entry.value;
            if (entry.timestamp < stats.firstTimestamp) stats.firstTimestamp = entry.timestamp;
            if (entry.timestamp > stats.lastTimestamp) stats.lastTimestamp = entry.timestamp;
        }

        stats.average = sum / _buffer.size();

        // Standard deviation
        double sumSqDiff = 0.0;
        for (const Entry &entry : _buffer) {
            const double diff = static_cast<double>(entry.value) - stats.average;
            sumSqDiff += diff * diff;
        }
        stats.stddev = std::sqrt(sumSqDiff / _buffer.size());

        // Rate calculation
        const qint64 durationMs = stats.lastTimestamp - stats.firstTimestamp;
        if (durationMs > 0) {
            stats.ratePerSecond = static_cast<double>(_buffer.size() - 1) * 1000.0 / durationMs;
        }

        return stats;
    }

    /// Clear all data
    void clear()
    {
        QMutexLocker locker(&_mutex);
        _buffer.clear();
        _head = 0;
        _totalCount = 0;
        _runningSum = 0.0;
    }

    /// Resize the buffer capacity
    void setCapacity(int capacity)
    {
        QMutexLocker locker(&_mutex);

        if (capacity <= 0 || capacity == _capacity) {
            return;
        }

        // Get current entries in order
        QVector<Entry> current;
        current.reserve(_buffer.size());
        for (int i = 0; i < _buffer.size(); ++i) {
            const int idx = (_head - _buffer.size() + i + _capacity) % _capacity;
            current.append(_buffer[idx]);
        }

        // Keep only the most recent entries if shrinking
        _buffer.clear();
        _capacity = capacity;
        _buffer.reserve(capacity);
        _head = 0;

        const int start = (current.size() > capacity) ? current.size() - capacity : 0;
        for (int i = start; i < current.size(); ++i) {
            _buffer.append(current[i]);
            _head = (_head + 1) % _capacity;
        }
    }

    // Accessors
    int size() const
    {
        QMutexLocker locker(&_mutex);
        return _buffer.size();
    }

    int capacity() const { return _capacity; }

    bool isEmpty() const
    {
        QMutexLocker locker(&_mutex);
        return _buffer.isEmpty();
    }

    bool isFull() const
    {
        QMutexLocker locker(&_mutex);
        return _buffer.size() >= _capacity;
    }

    qint64 totalCount() const { return _totalCount; }

private:
    void _updateStats(const T &value)
    {
        _runningSum += static_cast<double>(value);
    }

    mutable QMutex _mutex;
    QVector<Entry> _buffer;
    int _capacity;
    int _head = 0;  // Next write position
    qint64 _totalCount = 0;  // Total samples ever added
    double _runningSum = 0.0;
};

/// Convenience type aliases
using QGCDoubleBuffer = QGCTelemetryBuffer<double>;
using QGCFloatBuffer = QGCTelemetryBuffer<float>;
using QGCIntBuffer = QGCTelemetryBuffer<int>;
