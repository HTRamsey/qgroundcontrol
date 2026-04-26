#include "ParameterCacheTest.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>

#include "Fact.h"
#include "FactMetaData.h"
#include "ParameterCache.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace {

/// Build a small set of heap-allocated Facts parented to owner.
/// Caller must delete owner (or qDeleteAll the returned hash) when done.
QHash<QString, Fact *> makeFacts(QObject *owner)
{
    QHash<QString, Fact *> facts;

    auto *f1 = new Fact(0, "INT_PARAM",   FactMetaData::valueTypeInt32,  owner);
    f1->setRawValue(QVariant(42));
    facts[f1->name()] = f1;

    auto *f2 = new Fact(0, "UINT_PARAM",  FactMetaData::valueTypeUint32, owner);
    f2->setRawValue(QVariant(123u));
    facts[f2->name()] = f2;

    auto *f3 = new Fact(0, "FLOAT_PARAM", FactMetaData::valueTypeFloat,  owner);
    f3->setRawValue(QVariant(3.14f));
    facts[f3->name()] = f3;

    auto *f4 = new Fact(0, "INT16_PARAM", FactMetaData::valueTypeInt16,  owner);
    f4->setRawValue(QVariant(static_cast<short>(-7)));
    facts[f4->name()] = f4;

    return facts;
}

/// isVolatile lambda that never excludes anything
auto noExclusion = [](const QString &, FactMetaData::ValueType_t) { return false; };

} // namespace

// ---------------------------------------------------------------------------
// Per-test isolation: wipe the cache dir before and after every test
// ---------------------------------------------------------------------------

void ParameterCacheTest::init()
{
    UnitTest::init();

    ParameterCache::ensureCacheDirExists();
    QDir dir = ParameterCache::cacheDir();
    for (const QString &entry : dir.entryList(QDir::Files)) {
        dir.remove(entry);
    }
    // Also remove any stray directories we may have planted (OpenFailed test)
    for (const QString &entry : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir(dir.filePath(entry)).removeRecursively();
    }
}

void ParameterCacheTest::cleanup()
{
    QDir dir = ParameterCache::cacheDir();
    for (const QString &entry : dir.entryList(QDir::Files)) {
        dir.remove(entry);
    }
    for (const QString &entry : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir(dir.filePath(entry)).removeRecursively();
    }

    UnitTest::cleanup();
}

// ---------------------------------------------------------------------------
// Test 1: cacheFile path format
// ---------------------------------------------------------------------------

void ParameterCacheTest::_cacheFilePath()
{
    const int vehicleId   = 7;
    const int componentId = 50;
    const QString path    = ParameterCache::cacheFile(vehicleId, componentId);

    // Must be inside cacheDir()
    const QString cacheBase = ParameterCache::cacheDir().absolutePath();
    QVERIFY2(path.startsWith(cacheBase),
             qPrintable(QString("cacheFile path '%1' not under cacheDir '%2'").arg(path, cacheBase)));

    // Filename must contain "%1_%2.v2" formatting
    const QFileInfo fi(path);
    const QString expectedName = QStringLiteral("%1_%2.v2").arg(vehicleId).arg(componentId);
    QCOMPARE(fi.fileName(), expectedName);
}

// ---------------------------------------------------------------------------
// Test 2: write + load round-trip — CrcMatch
// ---------------------------------------------------------------------------

void ParameterCacheTest::_writeLoadRoundTripCrcMatch()
{
    constexpr int vid = 1, cid = 1;

    QObject owner;
    const QHash<QString, Fact *> facts = makeFacts(&owner);

    ParameterCache::write(vid, cid, facts);

    // First load: get the local CRC (remoteCrc=0 → will be CrcMismatch, but populates localCrc)
    const ParameterCache::LoadResult probe = ParameterCache::load(vid, cid, 0u, noExclusion);
    QVERIFY(probe.status == ParameterCache::LoadStatus::CrcMismatch
            || probe.status == ParameterCache::LoadStatus::CrcMatch);
    QVERIFY(probe.localCrc != 0u);
    QVERIFY(!probe.params.isEmpty());

    // Second load: use the captured CRC — must match now
    const ParameterCache::LoadResult result = ParameterCache::load(vid, cid, probe.localCrc, noExclusion);
    QCOMPARE(static_cast<int>(result.status), static_cast<int>(ParameterCache::LoadStatus::CrcMatch));
    QCOMPARE(result.localCrc, probe.localCrc);
    QVERIFY(!result.params.isEmpty());

    // Verify written names and types survive the round-trip
    for (const auto &[name, fact] : facts.asKeyValueRange()) {
        QVERIFY2(result.params.contains(name),
                 qPrintable(QString("Param '%1' missing from loaded cache").arg(name)));
        QCOMPARE(static_cast<int>(result.params[name].type), static_cast<int>(fact->type()));
    }
}

// ---------------------------------------------------------------------------
// Test 3: CrcMismatch
// ---------------------------------------------------------------------------

void ParameterCacheTest::_loadCrcMismatch()
{
    constexpr int vid = 2, cid = 1;

    QObject owner;
    ParameterCache::write(vid, cid, makeFacts(&owner));

    // Load with a deliberately wrong remoteCrc
    constexpr uint32_t wrongCrc = 0xDEADBEEFu;
    const ParameterCache::LoadResult result = ParameterCache::load(vid, cid, wrongCrc, noExclusion);

    QCOMPARE(static_cast<int>(result.status), static_cast<int>(ParameterCache::LoadStatus::CrcMismatch));
    QVERIFY(!result.params.isEmpty());
    QVERIFY(result.localCrc != 0u);
    QVERIFY(result.localCrc != wrongCrc);
}

// ---------------------------------------------------------------------------
// Test 4: FileNotFound
// ---------------------------------------------------------------------------

void ParameterCacheTest::_loadFileNotFound()
{
    constexpr int vid = 99, cid = 99;  // Never written

    const ParameterCache::LoadResult result = ParameterCache::load(vid, cid, 0u, noExclusion);

    QCOMPARE(static_cast<int>(result.status), static_cast<int>(ParameterCache::LoadStatus::FileNotFound));
    QVERIFY(result.params.isEmpty());
    QCOMPARE(result.localCrc, 0u);
}

// ---------------------------------------------------------------------------
// Test 5: OpenFailed — regression guard for the open-failure branch
// ---------------------------------------------------------------------------

void ParameterCacheTest::_loadOpenFailed()
{
    constexpr int vid = 5, cid = 1;

    const QString filePath = ParameterCache::cacheFile(vid, cid);

    // Create a directory at the exact cache-file path so QFile::exists() is
    // true but QFile::open() fails (directory, not a regular file).
    ParameterCache::ensureCacheDirExists();
    QVERIFY2(QDir().mkpath(filePath),
             qPrintable(QString("Failed to create directory at '%1'").arg(filePath)));

    // Suppress the expected warning from ParameterCache
    expectLogMessage(QtWarningMsg, QRegularExpression("Failed to open cache file"));

    const ParameterCache::LoadResult result = ParameterCache::load(vid, cid, 0u, noExclusion);

    QCOMPARE(static_cast<int>(result.status), static_cast<int>(ParameterCache::LoadStatus::OpenFailed));
    QVERIFY(result.params.isEmpty());
    QCOMPARE(result.localCrc, 0u);
}

// ---------------------------------------------------------------------------
// Test 6: Volatile-check CRC exclusion
// ---------------------------------------------------------------------------

void ParameterCacheTest::_loadVolatileCheckExcludes()
{
    constexpr int vid = 6, cid = 1;

    QObject owner;
    ParameterCache::write(vid, cid, makeFacts(&owner));

    // Load 1: nothing excluded — capture CRC1
    const ParameterCache::LoadResult r1 = ParameterCache::load(vid, cid, 0u, noExclusion);
    QVERIFY(!r1.params.isEmpty());
    const uint32_t crc1 = r1.localCrc;

    // Load 2: exclude INT_PARAM — CRC should differ
    auto excludeIntParam = [](const QString &name, FactMetaData::ValueType_t) {
        return name == QStringLiteral("INT_PARAM");
    };
    const ParameterCache::LoadResult r2 = ParameterCache::load(vid, cid, 0u, excludeIntParam);
    QVERIFY(!r2.params.isEmpty());
    const uint32_t crc2 = r2.localCrc;

    QVERIFY2(crc1 != crc2,
             "CRC with volatile exclusion must differ from CRC without exclusion");

    // Load 3: same exclusion + remoteCrc = crc2 → CrcMatch
    const ParameterCache::LoadResult r3 = ParameterCache::load(vid, cid, crc2, excludeIntParam);
    QCOMPARE(static_cast<int>(r3.status), static_cast<int>(ParameterCache::LoadStatus::CrcMatch));
    QCOMPARE(r3.localCrc, crc2);
}

UT_REGISTER_TEST(ParameterCacheTest, TestLabel::Unit)
