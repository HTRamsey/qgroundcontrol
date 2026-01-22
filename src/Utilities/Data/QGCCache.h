#pragma once

#include <QtCore/QHash>
#include <QtCore/QDateTime>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <optional>

/// Thread-safe in-memory cache with TTL and LRU eviction.
///
/// Example usage:
/// @code
/// QGCCache<QString, QJsonDocument> cache(100, 60000);  // 100 items, 60s TTL
/// cache.insert("key", jsonDoc);
/// if (auto doc = cache.get("key")) {
///     // Use cached document
/// }
/// @endcode
template<typename Key, typename Value>
class QGCCache
{
public:
    /// Create a cache
    /// @param maxSize Maximum number of items (0 = unlimited)
    /// @param ttlMs Time-to-live in milliseconds (0 = never expires)
    explicit QGCCache(int maxSize = 100, qint64 ttlMs = 0)
        : _maxSize(maxSize)
        , _ttlMs(ttlMs)
    {
    }

    /// Insert or update a value
    void insert(const Key &key, const Value &value)
    {
        QMutexLocker locker(&_mutex);

        // Evict if at capacity
        if (_maxSize > 0 && _entries.size() >= _maxSize && !_entries.contains(key)) {
            _evictOldest();
        }

        CacheEntry entry;
        entry.value = value;
        entry.insertTime = QDateTime::currentMSecsSinceEpoch();
        entry.lastAccess = entry.insertTime;
        entry.accessCount = 0;

        _entries[key] = entry;
        _accessOrder.removeAll(key);
        _accessOrder.append(key);
    }

    /// Get a value (returns std::nullopt if not found or expired)
    std::optional<Value> get(const Key &key)
    {
        QMutexLocker locker(&_mutex);

        auto it = _entries.find(key);
        if (it == _entries.end()) {
            return std::nullopt;
        }

        // Check TTL
        if (_ttlMs > 0) {
            const qint64 age = QDateTime::currentMSecsSinceEpoch() - it->insertTime;
            if (age > _ttlMs) {
                _entries.erase(it);
                _accessOrder.removeAll(key);
                return std::nullopt;
            }
        }

        // Update access info
        it->lastAccess = QDateTime::currentMSecsSinceEpoch();
        it->accessCount++;

        // Move to end of access order (most recently used)
        _accessOrder.removeAll(key);
        _accessOrder.append(key);

        return it->value;
    }

    /// Check if key exists and is not expired
    bool contains(const Key &key)
    {
        return get(key).has_value();
    }

    /// Remove a specific key
    bool remove(const Key &key)
    {
        QMutexLocker locker(&_mutex);

        if (_entries.remove(key) > 0) {
            _accessOrder.removeAll(key);
            return true;
        }
        return false;
    }

    /// Clear all entries
    void clear()
    {
        QMutexLocker locker(&_mutex);
        _entries.clear();
        _accessOrder.clear();
    }

    /// Remove expired entries
    int purgeExpired()
    {
        if (_ttlMs <= 0) {
            return 0;
        }

        QMutexLocker locker(&_mutex);
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        int purged = 0;

        QList<Key> toRemove;
        for (auto it = _entries.begin(); it != _entries.end(); ++it) {
            if ((now - it->insertTime) > _ttlMs) {
                toRemove.append(it.key());
            }
        }

        for (const Key &key : toRemove) {
            _entries.remove(key);
            _accessOrder.removeAll(key);
            purged++;
        }

        return purged;
    }

    // Accessors
    int size() const
    {
        QMutexLocker locker(&_mutex);
        return _entries.size();
    }

    int maxSize() const { return _maxSize; }
    void setMaxSize(int size) { _maxSize = size; }

    qint64 ttlMs() const { return _ttlMs; }
    void setTtlMs(qint64 ms) { _ttlMs = ms; }

    bool isEmpty() const
    {
        QMutexLocker locker(&_mutex);
        return _entries.isEmpty();
    }

    QList<Key> keys() const
    {
        QMutexLocker locker(&_mutex);
        return _entries.keys();
    }

    /// Get cache statistics
    struct Stats {
        int size = 0;
        int maxSize = 0;
        qint64 ttlMs = 0;
        qint64 totalAccesses = 0;
    };

    Stats stats() const
    {
        QMutexLocker locker(&_mutex);

        Stats s;
        s.size = _entries.size();
        s.maxSize = _maxSize;
        s.ttlMs = _ttlMs;

        for (const auto &entry : _entries) {
            s.totalAccesses += entry.accessCount;
        }

        return s;
    }

private:
    struct CacheEntry {
        Value value;
        qint64 insertTime = 0;
        qint64 lastAccess = 0;
        int accessCount = 0;
    };

    void _evictOldest()
    {
        // LRU eviction - remove least recently used
        if (_accessOrder.isEmpty()) {
            return;
        }

        const Key &oldest = _accessOrder.first();
        _entries.remove(oldest);
        _accessOrder.removeFirst();
    }

    mutable QMutex _mutex;
    QHash<Key, CacheEntry> _entries;
    QList<Key> _accessOrder;  // For LRU tracking

    int _maxSize;
    qint64 _ttlMs;
};

/// Convenience type aliases
using QGCStringCache = QGCCache<QString, QString>;
using QGCVariantCache = QGCCache<QString, QVariant>;
