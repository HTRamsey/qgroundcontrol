#include "QGCCacheTest.h"
#include "QGCCache.h"

#include <QtTest/QTest>

void QGCCacheTest::_testInsertAndGet()
{
    QGCCache<QString, int> cache(100, 0);  // No TTL

    cache.insert("key1", 42);
    cache.insert("key2", 100);

    auto value1 = cache.get("key1");
    auto value2 = cache.get("key2");
    auto value3 = cache.get("key3");

    QVERIFY(value1.has_value());
    QCOMPARE(value1.value(), 42);

    QVERIFY(value2.has_value());
    QCOMPARE(value2.value(), 100);

    QVERIFY(!value3.has_value());
}

void QGCCacheTest::_testCapacity()
{
    QGCCache<QString, int> cache(3, 0);  // Max 3 items

    cache.insert("key1", 1);
    cache.insert("key2", 2);
    cache.insert("key3", 3);

    QCOMPARE(cache.size(), 3);

    // Adding a 4th should evict the oldest
    cache.insert("key4", 4);
    QCOMPARE(cache.size(), 3);

    // key1 should be evicted (LRU)
    QVERIFY(!cache.get("key1").has_value());
    QVERIFY(cache.get("key4").has_value());
}

void QGCCacheTest::_testTTLExpiration()
{
    QGCCache<QString, int> cache(100, 50);  // 50ms TTL

    cache.insert("key1", 42);
    QVERIFY(cache.get("key1").has_value());

    // Wait for expiration
    QTest::qWait(100);

    // Should be expired now
    QVERIFY(!cache.get("key1").has_value());
}

void QGCCacheTest::_testLRUEviction()
{
    QGCCache<QString, int> cache(3, 0);

    cache.insert("key1", 1);
    cache.insert("key2", 2);
    cache.insert("key3", 3);

    // Access key1 to make it recently used
    cache.get("key1");

    // Add key4, should evict key2 (least recently used)
    cache.insert("key4", 4);

    QVERIFY(cache.get("key1").has_value());  // Still there (recently used)
    QVERIFY(!cache.get("key2").has_value()); // Evicted
    QVERIFY(cache.get("key3").has_value());  // Still there
    QVERIFY(cache.get("key4").has_value());  // Newly added
}

void QGCCacheTest::_testRemove()
{
    QGCCache<QString, int> cache(100, 0);

    cache.insert("key1", 42);
    QVERIFY(cache.contains("key1"));

    bool removed = cache.remove("key1");
    QVERIFY(removed);
    QVERIFY(!cache.contains("key1"));

    // Removing non-existent key
    bool removedAgain = cache.remove("key1");
    QVERIFY(!removedAgain);
}

void QGCCacheTest::_testClear()
{
    QGCCache<QString, int> cache(100, 0);

    cache.insert("key1", 1);
    cache.insert("key2", 2);
    cache.insert("key3", 3);
    QCOMPARE(cache.size(), 3);

    cache.clear();
    QCOMPARE(cache.size(), 0);
    QVERIFY(cache.isEmpty());
}

void QGCCacheTest::_testPurgeExpired()
{
    QGCCache<QString, int> cache(100, 50);  // 50ms TTL

    cache.insert("key1", 1);
    cache.insert("key2", 2);
    QCOMPARE(cache.size(), 2);

    QTest::qWait(100);

    // Add a fresh entry
    cache.insert("key3", 3);

    // Purge expired entries
    int purged = cache.purgeExpired();
    QCOMPARE(purged, 2);  // key1 and key2 expired
    QCOMPARE(cache.size(), 1);
    QVERIFY(cache.get("key3").has_value());  // Fresh entry still there
}
