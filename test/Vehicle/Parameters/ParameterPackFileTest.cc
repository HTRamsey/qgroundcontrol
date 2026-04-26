#include "ParameterPackFileTest.h"

#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <cstring>

#include "ParameterPackFile.h"

namespace {

constexpr quint16 kMagicStandard     = 0x671B;
constexpr quint16 kMagicWithDefaults = 0x671C;

enum ApVarType : quint8
{
    AP_PARAM_INT8  = 1,
    AP_PARAM_INT16 = 2,
    AP_PARAM_INT32 = 3,
    AP_PARAM_FLOAT = 4,
};

QByteArray header(quint16 magic, quint16 numParams, quint16 totalParams)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out << magic << numParams << totalParams;
    return bytes;
}

/// Build one record: type byte, name-len byte, name bytes, value bytes.
/// Caller must ensure name fits in 16 chars and uses common_len=0 (no shared prefix).
QByteArray record(ApVarType ptype, const QByteArray& name, const QByteArray& valueBytes,
                  bool withDefault = false, const QByteArray& defaultBytes = {})
{
    Q_ASSERT(name.size() >= 1 && name.size() <= 16);
    QByteArray bytes;
    const quint8 flags  = withDefault ? 0x01 : 0x00;
    const quint8 type   = static_cast<quint8>(ptype);
    bytes.append(static_cast<char>((flags << 4) | (type & 0x0F)));
    const quint8 nameLenField  = static_cast<quint8>(name.size() - 1) & 0x0F;
    const quint8 commonLenField = 0;
    bytes.append(static_cast<char>((nameLenField << 4) | commonLenField));
    bytes.append(name);
    bytes.append(valueBytes);
    if (withDefault) {
        bytes.append(defaultBytes);
    }
    return bytes;
}

QByteArray int32LE(qint32 v)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out << v;
    return bytes;
}

QByteArray floatLE(float v)
{
    qint32 raw = 0;
    std::memcpy(&raw, &v, sizeof(raw));
    return int32LE(raw);
}

QByteArray int16LE(qint16 v)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out << v;
    return bytes;
}

QByteArray int8LE(qint8 v)
{
    return QByteArray(1, static_cast<char>(v));
}

/// Build a record that shares a prefix of @p commonLen bytes with the previous
/// record. The on-wire payload only carries the trailing @p suffix bytes.
QByteArray prefixRecord(ApVarType ptype, quint8 commonLen, const QByteArray& suffix, const QByteArray& valueBytes)
{
    Q_ASSERT(suffix.size() >= 1 && suffix.size() <= 16 - commonLen);
    QByteArray bytes;
    bytes.append(static_cast<char>(static_cast<quint8>(ptype) & 0x0F));
    const quint8 nameLenField = static_cast<quint8>(suffix.size() - 1) & 0x0F;
    bytes.append(static_cast<char>((nameLenField << 4) | (commonLen & 0x0F)));
    bytes.append(suffix);
    bytes.append(valueBytes);
    return bytes;
}

}  // namespace

void ParameterPackFileTest::init()
{
    UnitTest::init();
    QVERIFY(_tempDir.isValid());
}

void ParameterPackFileTest::cleanup()
{
    UnitTest::cleanup();
}

QString ParameterPackFileTest::_writeFixture(const QByteArray& bytes) const
{
    const QString path = QDir(_tempDir.path()).filePath(QStringLiteral("fixture_%1.pck").arg(_fixtureCounter));
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        return {};
    }
    f.write(bytes);
    f.close();
    const_cast<ParameterPackFileTest*>(this)->_fixtureCounter++;
    return path;
}

void ParameterPackFileTest::_parseMissingFile()
{
    const QString path = QDir(_tempDir.path()).filePath(QStringLiteral("does-not-exist.pck"));
    const auto result = ParameterPackFile::parse(path);
    QVERIFY(!result.ok);
    QVERIFY(result.params.isEmpty());
}

void ParameterPackFileTest::_parseInvalidMagic()
{
    QByteArray bytes = header(0x1234, 0, 0);
    const QString path = _writeFixture(bytes);
    const auto result = ParameterPackFile::parse(path);
    QVERIFY(!result.ok);
    QVERIFY(result.params.isEmpty());
}

void ParameterPackFileTest::_parsePartialHeaderRejected()
{
    // Write only 3 bytes — header reader needs 6
    const QString path = _writeFixture(QByteArray("\x1B\x67\x01", 3));
    const auto result = ParameterPackFile::parse(path);
    QVERIFY(!result.ok);
    QVERIFY(result.params.isEmpty());
}

void ParameterPackFileTest::_parseSingleInt32()
{
    QByteArray bytes;
    bytes.append(header(kMagicStandard, 1, 1));
    bytes.append(record(AP_PARAM_INT32, QByteArray("RATE"), int32LE(42)));

    const QString path = _writeFixture(bytes);
    const auto result = ParameterPackFile::parse(path);
    QVERIFY(result.ok);
    QCOMPARE(result.params.size(), 1);
    QCOMPARE(result.params[0].name, QStringLiteral("RATE"));
    QCOMPARE(static_cast<int>(result.params[0].type), static_cast<int>(FactMetaData::valueTypeInt32));
    QCOMPARE(result.params[0].value.toInt(), 42);
    QVERIFY(!result.params[0].defaultValue.isValid());
}

void ParameterPackFileTest::_parseMixedTypes()
{
    QByteArray bytes;
    bytes.append(header(kMagicStandard, 4, 4));
    bytes.append(record(AP_PARAM_INT8,  QByteArray("A"),         int8LE(-3)));
    bytes.append(record(AP_PARAM_INT16, QByteArray("HEAD"),      int16LE(-1234)));
    bytes.append(record(AP_PARAM_INT32, QByteArray("FOO_BIG"),   int32LE(987654)));
    bytes.append(record(AP_PARAM_FLOAT, QByteArray("VAL"),       floatLE(2.5f)));

    const QString path = _writeFixture(bytes);
    const auto result = ParameterPackFile::parse(path);
    QVERIFY(result.ok);
    QCOMPARE(result.params.size(), 4);

    QCOMPARE(result.params[0].name, QStringLiteral("A"));
    QCOMPARE(static_cast<int>(result.params[0].type), static_cast<int>(FactMetaData::valueTypeInt8));
    QCOMPARE(result.params[0].value.toInt(), -3);

    QCOMPARE(result.params[1].name, QStringLiteral("HEAD"));
    QCOMPARE(static_cast<int>(result.params[1].type), static_cast<int>(FactMetaData::valueTypeInt16));
    QCOMPARE(result.params[1].value.toInt(), -1234);

    QCOMPARE(result.params[2].name, QStringLiteral("FOO_BIG"));
    QCOMPARE(static_cast<int>(result.params[2].type), static_cast<int>(FactMetaData::valueTypeInt32));
    QCOMPARE(result.params[2].value.toInt(), 987654);

    QCOMPARE(result.params[3].name, QStringLiteral("VAL"));
    QCOMPARE(static_cast<int>(result.params[3].type), static_cast<int>(FactMetaData::valueTypeFloat));
    QCOMPARE(result.params[3].value.toFloat(), 2.5f);
}

void ParameterPackFileTest::_parseWithDefaults()
{
    QByteArray bytes;
    bytes.append(header(kMagicWithDefaults, 2, 2));
    bytes.append(record(AP_PARAM_INT32, QByteArray("WITH_DEF"), int32LE(7),
                        /*withDefault=*/true, int32LE(100)));
    // Second record without per-record default flag — even with the file-level
    // defaults magic, only records that opt in carry one.
    bytes.append(record(AP_PARAM_FLOAT, QByteArray("PLAIN"), floatLE(0.25f)));

    const QString path = _writeFixture(bytes);
    const auto result = ParameterPackFile::parse(path);
    QVERIFY(result.ok);
    QCOMPARE(result.params.size(), 2);

    QCOMPARE(result.params[0].value.toInt(), 7);
    QVERIFY(result.params[0].defaultValue.isValid());
    QCOMPARE(result.params[0].defaultValue.toInt(), 100);

    QCOMPARE(result.params[1].value.toFloat(), 0.25f);
    QVERIFY(!result.params[1].defaultValue.isValid());
}

void ParameterPackFileTest::_parseSharedNamePrefix()
{
    // ArduPilot encodes consecutive params that share a leading substring with a
    // (common_len, name_len) pair: only the differing tail is on the wire and the
    // first common_len bytes of name_buffer must persist from the previous record.
    QByteArray bytes;
    bytes.append(header(kMagicStandard, 3, 3));
    bytes.append(record(AP_PARAM_INT32, QByteArray("RC1_MIN"),  int32LE(1100)));
    // common_len=4 ("RC1_") + suffix "MAX" -> "RC1_MAX"
    bytes.append(prefixRecord(AP_PARAM_INT32, 4, QByteArray("MAX"),  int32LE(1900)));
    // common_len=4 ("RC1_") + suffix "TRIM" -> "RC1_TRIM"
    bytes.append(prefixRecord(AP_PARAM_INT32, 4, QByteArray("TRIM"), int32LE(1500)));

    const QString path = _writeFixture(bytes);
    const auto result = ParameterPackFile::parse(path);
    QVERIFY(result.ok);
    QCOMPARE(result.params.size(), 3);
    QCOMPARE(result.params[0].name, QStringLiteral("RC1_MIN"));
    QCOMPARE(result.params[1].name, QStringLiteral("RC1_MAX"));
    QCOMPARE(result.params[2].name, QStringLiteral("RC1_TRIM"));
    QCOMPARE(result.params[0].value.toInt(), 1100);
    QCOMPARE(result.params[1].value.toInt(), 1900);
    QCOMPARE(result.params[2].value.toInt(), 1500);
}

void ParameterPackFileTest::_parseTruncatedValueRejected()
{
    QByteArray bytes;
    bytes.append(header(kMagicStandard, 1, 1));
    // Build a partial int32 record: claim INT32, name "TRUNC", but write only 2 bytes of value.
    bytes.append(static_cast<char>(0x03));                        // type=INT32, no flags
    bytes.append(static_cast<char>((4 << 4) | 0));                // name_len=5, common_len=0
    bytes.append(QByteArray("TRUNC"));
    bytes.append(QByteArray("\x01\x02", 2));                      // only 2/4 value bytes

    const QString path = _writeFixture(bytes);
    const auto result = ParameterPackFile::parse(path);
    QVERIFY(!result.ok);
}

void ParameterPackFileTest::_parseNameLengthOverflowRejected()
{
    QByteArray bytes;
    bytes.append(header(kMagicStandard, 1, 1));
    // Force name_len + common_len > 16: name_len_field=15 -> name_len=16, common_len=1, sum=17.
    bytes.append(static_cast<char>(0x03));                        // type=INT32, no flags
    bytes.append(static_cast<char>((15 << 4) | 1));               // (16-1)<<4 | common_len=1
    bytes.append(QByteArray(16, 'X'));
    bytes.append(int32LE(0));

    const QString path = _writeFixture(bytes);
    const auto result = ParameterPackFile::parse(path);
    QVERIFY(!result.ok);
}

UT_REGISTER_TEST(ParameterPackFileTest, TestLabel::Unit)
