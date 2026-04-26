#include "ParameterMavlinkCodecTest.h"

#include <QtCore/QRegularExpression>

#include "ParameterMavlinkCodec.h"

void ParameterMavlinkCodecTest::_factTypeRoundTrip_data()
{
    QTest::addColumn<int>("factType");
    QTest::addColumn<int>("expectedMavType");

    QTest::newRow("uint8")  << static_cast<int>(FactMetaData::valueTypeUint8)  << static_cast<int>(MAV_PARAM_TYPE_UINT8);
    QTest::newRow("int8")   << static_cast<int>(FactMetaData::valueTypeInt8)   << static_cast<int>(MAV_PARAM_TYPE_INT8);
    QTest::newRow("uint16") << static_cast<int>(FactMetaData::valueTypeUint16) << static_cast<int>(MAV_PARAM_TYPE_UINT16);
    QTest::newRow("int16")  << static_cast<int>(FactMetaData::valueTypeInt16)  << static_cast<int>(MAV_PARAM_TYPE_INT16);
    QTest::newRow("uint32") << static_cast<int>(FactMetaData::valueTypeUint32) << static_cast<int>(MAV_PARAM_TYPE_UINT32);
    QTest::newRow("int32")  << static_cast<int>(FactMetaData::valueTypeInt32)  << static_cast<int>(MAV_PARAM_TYPE_INT32);
    QTest::newRow("uint64") << static_cast<int>(FactMetaData::valueTypeUint64) << static_cast<int>(MAV_PARAM_TYPE_UINT64);
    QTest::newRow("int64")  << static_cast<int>(FactMetaData::valueTypeInt64)  << static_cast<int>(MAV_PARAM_TYPE_INT64);
    QTest::newRow("float")  << static_cast<int>(FactMetaData::valueTypeFloat)  << static_cast<int>(MAV_PARAM_TYPE_REAL32);
    QTest::newRow("double") << static_cast<int>(FactMetaData::valueTypeDouble) << static_cast<int>(MAV_PARAM_TYPE_REAL64);
}

void ParameterMavlinkCodecTest::_factTypeRoundTrip()
{
    QFETCH(int, factType);
    QFETCH(int, expectedMavType);

    const auto factEnum = static_cast<FactMetaData::ValueType_t>(factType);
    const MAV_PARAM_TYPE mav = ParameterMavlinkCodec::factTypeToMavType(factEnum);
    QCOMPARE(static_cast<int>(mav), expectedMavType);

    QCOMPARE(static_cast<int>(ParameterMavlinkCodec::mavTypeToFactType(mav)), factType);
}

void ParameterMavlinkCodecTest::_factTypeUnsupportedFallsBackToInt32()
{
    expectLogMessage(QtWarningMsg, QRegularExpression("Unsupported fact type"));
    const MAV_PARAM_TYPE mav = ParameterMavlinkCodec::factTypeToMavType(FactMetaData::valueTypeString);
    QCOMPARE(static_cast<int>(mav), static_cast<int>(MAV_PARAM_TYPE_INT32));
}

void ParameterMavlinkCodecTest::_mavTypeUnsupportedFallsBackToInt32()
{
    expectLogMessage(QtWarningMsg, QRegularExpression("Unsupported mav param type"));
    const auto fact = ParameterMavlinkCodec::mavTypeToFactType(static_cast<MAV_PARAM_TYPE>(0xFF));
    QCOMPARE(static_cast<int>(fact), static_cast<int>(FactMetaData::valueTypeInt32));
}

void ParameterMavlinkCodecTest::_fillUnion_data()
{
    QTest::addColumn<int>("valueType");
    QTest::addColumn<QVariant>("rawValue");
    QTest::addColumn<double>("expected");

    QTest::newRow("uint8")  << static_cast<int>(FactMetaData::valueTypeUint8)  << QVariant(static_cast<uint>(200))           << 200.0;
    QTest::newRow("int8")   << static_cast<int>(FactMetaData::valueTypeInt8)   << QVariant(-7)                                << -7.0;
    QTest::newRow("uint16") << static_cast<int>(FactMetaData::valueTypeUint16) << QVariant(static_cast<uint>(60000))         << 60000.0;
    QTest::newRow("int16")  << static_cast<int>(FactMetaData::valueTypeInt16)  << QVariant(-30000)                            << -30000.0;
    QTest::newRow("uint32") << static_cast<int>(FactMetaData::valueTypeUint32) << QVariant(static_cast<uint>(4000000000u))   << 4000000000.0;
    QTest::newRow("int32")  << static_cast<int>(FactMetaData::valueTypeInt32)  << QVariant(-123456789)                        << -123456789.0;
    QTest::newRow("float")  << static_cast<int>(FactMetaData::valueTypeFloat)  << QVariant(3.5f)                              << 3.5;
}

void ParameterMavlinkCodecTest::_fillUnion()
{
    QFETCH(int, valueType);
    QFETCH(QVariant, rawValue);
    QFETCH(double, expected);

    const auto factType = static_cast<FactMetaData::ValueType_t>(valueType);
    const auto paramUnion = ParameterMavlinkCodec::fillUnion(factType, rawValue);
    QVERIFY(paramUnion);

    double actual = 0.0;
    switch (factType) {
        case FactMetaData::valueTypeUint8:  actual = paramUnion->param_uint8;  break;
        case FactMetaData::valueTypeInt8:   actual = paramUnion->param_int8;   break;
        case FactMetaData::valueTypeUint16: actual = paramUnion->param_uint16; break;
        case FactMetaData::valueTypeInt16:  actual = paramUnion->param_int16;  break;
        case FactMetaData::valueTypeUint32: actual = paramUnion->param_uint32; break;
        case FactMetaData::valueTypeInt32:  actual = paramUnion->param_int32;  break;
        case FactMetaData::valueTypeFloat:  actual = paramUnion->param_float;  break;
        default: QFAIL("unreachable in test data");
    }
    QCOMPARE(actual, expected);
}

void ParameterMavlinkCodecTest::_fillUnionRejectsNonNumeric()
{
    expectLogMessage(QtCriticalMsg, QRegularExpression("Fact failed to convert"));
    QVERIFY(!ParameterMavlinkCodec::fillUnion(FactMetaData::valueTypeInt32, QVariant(QStringLiteral("not-a-number"))));
}

void ParameterMavlinkCodecTest::_unionToVariant_data()
{
    QTest::addColumn<int>("mavType");
    QTest::addColumn<double>("inputValue");
    QTest::addColumn<int>("expectedQVariantType");

    // uint8/int8/uint16/int16 widen to int via integer promotion in QVariant ctor.
    QTest::newRow("uint8")  << static_cast<int>(MAV_PARAM_TYPE_UINT8)  << 12.0   << static_cast<int>(QMetaType::Int);
    QTest::newRow("int8")   << static_cast<int>(MAV_PARAM_TYPE_INT8)   << -3.0   << static_cast<int>(QMetaType::Int);
    QTest::newRow("uint16") << static_cast<int>(MAV_PARAM_TYPE_UINT16) << 1234.0 << static_cast<int>(QMetaType::Int);
    QTest::newRow("int16")  << static_cast<int>(MAV_PARAM_TYPE_INT16)  << -42.0  << static_cast<int>(QMetaType::Int);
    QTest::newRow("uint32") << static_cast<int>(MAV_PARAM_TYPE_UINT32) << 3.5e6  << static_cast<int>(QMetaType::UInt);
    QTest::newRow("int32")  << static_cast<int>(MAV_PARAM_TYPE_INT32)  << -7.0   << static_cast<int>(QMetaType::Int);
    QTest::newRow("real32") << static_cast<int>(MAV_PARAM_TYPE_REAL32) << 0.5    << static_cast<int>(QMetaType::Float);
}

void ParameterMavlinkCodecTest::_unionToVariant()
{
    QFETCH(int, mavType);
    QFETCH(double, inputValue);
    QFETCH(int, expectedQVariantType);

    mavlink_param_union_t paramUnion{};
    paramUnion.type = static_cast<uint8_t>(mavType);
    switch (static_cast<MAV_PARAM_TYPE>(mavType)) {
        case MAV_PARAM_TYPE_UINT8:  paramUnion.param_uint8  = static_cast<uint8_t>(inputValue);  break;
        case MAV_PARAM_TYPE_INT8:   paramUnion.param_int8   = static_cast<int8_t>(inputValue);   break;
        case MAV_PARAM_TYPE_UINT16: paramUnion.param_uint16 = static_cast<uint16_t>(inputValue); break;
        case MAV_PARAM_TYPE_INT16:  paramUnion.param_int16  = static_cast<int16_t>(inputValue);  break;
        case MAV_PARAM_TYPE_UINT32: paramUnion.param_uint32 = static_cast<uint32_t>(inputValue); break;
        case MAV_PARAM_TYPE_INT32:  paramUnion.param_int32  = static_cast<int32_t>(inputValue);  break;
        case MAV_PARAM_TYPE_REAL32: paramUnion.param_float  = static_cast<float>(inputValue);    break;
        default: QFAIL("unreachable in test data");
    }

    const auto out = ParameterMavlinkCodec::unionToVariant(paramUnion);
    QVERIFY(out);
    QCOMPARE(out->userType(), expectedQVariantType);
    QCOMPARE(out->toDouble(), inputValue);
}

void ParameterMavlinkCodecTest::_unionToVariantRejectsUnsupportedType()
{
    expectLogMessage(QtCriticalMsg, QRegularExpression("Unsupported MAV_PARAM_TYPE"));
    mavlink_param_union_t paramUnion{};
    paramUnion.type = static_cast<uint8_t>(MAV_PARAM_TYPE_REAL64);
    QVERIFY(!ParameterMavlinkCodec::unionToVariant(paramUnion));
}

void ParameterMavlinkCodecTest::_variantUnionRoundTrip_data()
{
    QTest::addColumn<int>("valueType");
    QTest::addColumn<QVariant>("rawValue");

    QTest::newRow("uint8 mid")    << static_cast<int>(FactMetaData::valueTypeUint8)  << QVariant(static_cast<uint>(200));
    QTest::newRow("uint8 zero")   << static_cast<int>(FactMetaData::valueTypeUint8)  << QVariant(static_cast<uint>(0));
    QTest::newRow("uint8 max")    << static_cast<int>(FactMetaData::valueTypeUint8)  << QVariant(static_cast<uint>(255));
    QTest::newRow("int8 neg")     << static_cast<int>(FactMetaData::valueTypeInt8)   << QVariant(-128);
    QTest::newRow("int8 pos")     << static_cast<int>(FactMetaData::valueTypeInt8)   << QVariant(127);
    QTest::newRow("uint16 max")   << static_cast<int>(FactMetaData::valueTypeUint16) << QVariant(static_cast<uint>(65535));
    QTest::newRow("int16 min")    << static_cast<int>(FactMetaData::valueTypeInt16)  << QVariant(-32768);
    QTest::newRow("uint32 max")   << static_cast<int>(FactMetaData::valueTypeUint32) << QVariant(static_cast<uint>(4000000000u));
    QTest::newRow("int32 min")    << static_cast<int>(FactMetaData::valueTypeInt32)  << QVariant(-2147483647 - 1);
    QTest::newRow("float zero")   << static_cast<int>(FactMetaData::valueTypeFloat)  << QVariant(0.0f);
    QTest::newRow("float small")  << static_cast<int>(FactMetaData::valueTypeFloat)  << QVariant(1.5f);
    QTest::newRow("float neg")    << static_cast<int>(FactMetaData::valueTypeFloat)  << QVariant(-3.25f);
}

void ParameterMavlinkCodecTest::_variantUnionRoundTrip()
{
    QFETCH(int, valueType);
    QFETCH(QVariant, rawValue);

    const auto factType = static_cast<FactMetaData::ValueType_t>(valueType);
    const auto un = ParameterMavlinkCodec::fillUnion(factType, rawValue);
    QVERIFY(un);
    const auto round = ParameterMavlinkCodec::unionToVariant(*un);
    QVERIFY(round);
    QCOMPARE(round->toDouble(), rawValue.toDouble());
}

UT_REGISTER_TEST(ParameterMavlinkCodecTest, TestLabel::Unit)
