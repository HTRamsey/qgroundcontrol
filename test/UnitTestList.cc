#include "UnitTestList.h"
#include "UnitTest.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QSet>

QGC_LOGGING_CATEGORY(UnitTestListLog, "Test.UnitTestList")

// ADSB
#include "ADSBTest.h"

// AnalyzeView
#include "ExifParserTest.h"
#include "GeoTagControllerTest.h"
// #include "MavlinkLogTest.h"
#include "LogDownloadTest.h"
#include "PX4LogParserTest.h"
#include "ULogParserTest.h"

// AutoPilotPlugins
// #include "RadioConfigTest.h"

// Camera
#include "QGCCameraManagerTest.h"

// Comms
#include "QGCSerialPortInfoTest.h"

// FactSystem
#include "FactSystemTestGeneric.h"
#include "FactSystemTestPX4.h"
#include "ParameterManagerTest.h"

// FollowMe
#include "FollowMeTest.h"

// GPS
#include "GpsTest.h"

// MAVLink
#include "StatusTextHandlerTest.h"
#include "SigningTest.h"

// MissionManager
#include "CameraCalcTest.h"
#include "CameraSectionTest.h"
#include "CorridorScanComplexItemTest.h"
// #include "FWLandingPatternTest.h"
// #include "LandingComplexItemTest.h"
// #include "MissionCommandTreeEditorTest.h"
#include "MissionCommandTreeTest.h"
#include "MissionControllerManagerTest.h"
#include "MissionControllerTest.h"
#include "MissionItemTest.h"
#include "MissionManagerTest.h"
#include "MissionSettingsTest.h"
#include "PlanMasterControllerTest.h"
#include "QGCMapPolygonTest.h"
#include "QGCMapPolylineTest.h"
// #include "SectionTest.h"
#include "SimpleMissionItemTest.h"
#include "SpeedSectionTest.h"
#include "StructureScanComplexItemTest.h"
#include "SurveyComplexItemTest.h"
#include "TransectStyleComplexItemTest.h"
// #include "VisualMissionItemTest.h"

// qgcunittest
#include "ComponentInformationCacheTest.h"
#include "ComponentInformationTranslationTest.h"

// QmlControls

// Terrain
#include "TerrainQueryTest.h"
#include "TerrainTileTest.h"

// UnitTestFramework
#include "MultiSignalSpyTest.h"

// UI

// Utilities
// Audio
#include "AudioOutputTest.h"
// Compression
#include "QGCArchiveModelTest.h"
#include "QGCCompressionTest.h"
#include "QGCStreamingDecompressionTest.h"
// FileSystem
#include "QGCArchiveWatcherTest.h"
#include "QGCFileDownloadTest.h"
#include "QGCFileHelperTest.h"
#include "QGCFileWatcherTest.h"
// Geo
#include "GeoTest.h"
// Platform
#include "PlatformTest.h"
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
#include "RunGuardTest.h"
#include "SignalHandlerTest.h"
#endif
// Shape
#include "ShapeTest.h"

// Vehicle
// Components
#include "ComponentInformationCacheTest.h"
#include "ComponentInformationTranslationTest.h"
#include "FTPManagerTest.h"
// #include "InitialConnectTest.h"
#include "MAVLinkLogManagerTest.h"
// #include "RequestMessageTest.h"
// #include "SendMavCommandWithHandlerTest.h"
// #include "SendMavCommandWithSignalingTest.h"
#include "VehicleLinkManagerTest.h"

// Missing
// #include "FlightGearUnitTest.h"
// #include "LinkManagerTest.h"
// #include "SendMavCommandTest.h"
// #include "TCPLinkTest.h"

namespace {

/// Registers all unit tests. Called once at startup.
void registerAllTests()
{
    static bool registered = false;
    if (registered) {
        return;
    }
    registered = true;

    // ADSB
    UT_REGISTER_TEST(ADSBTest)

    // AnalyzeView
    UT_REGISTER_TEST(ExifParserTest)
    UT_REGISTER_TEST(GeoTagControllerTest)
    // UT_REGISTER_TEST(MavlinkLogTest)
    UT_REGISTER_TEST(LogDownloadTest)
    UT_REGISTER_TEST(PX4LogParserTest)
    UT_REGISTER_TEST(ULogParserTest)

    // Camera
    UT_REGISTER_TEST(QGCCameraManagerTest)

    // Comms
    UT_REGISTER_TEST(QGCSerialPortInfoTest)

    // FactSystem
    UT_REGISTER_TEST(FactSystemTestGeneric)
    UT_REGISTER_TEST(FactSystemTestPX4)
    UT_REGISTER_TEST(ParameterManagerTest)

    // FollowMe
    UT_REGISTER_TEST(FollowMeTest)

    // GPS
    // UT_REGISTER_TEST(GpsTest)

    // MAVLink
    UT_REGISTER_TEST(StatusTextHandlerTest)
    UT_REGISTER_TEST(SigningTest)

    // MissionManager
    UT_REGISTER_TEST(CameraCalcTest)
    UT_REGISTER_TEST(CameraSectionTest)
    UT_REGISTER_TEST(CorridorScanComplexItemTest)
    // UT_REGISTER_TEST(FWLandingPatternTest)
    // UT_REGISTER_TEST(LandingComplexItemTest)
    // UT_REGISTER_TEST_STANDALONE(MissionCommandTreeEditorTest)
    UT_REGISTER_TEST(MissionCommandTreeTest)
    UT_REGISTER_TEST(MissionControllerManagerTest)
    UT_REGISTER_TEST(MissionControllerTest)
    UT_REGISTER_TEST(MissionItemTest)
    UT_REGISTER_TEST(MissionManagerTest)
    UT_REGISTER_TEST(MissionSettingsTest)
    UT_REGISTER_TEST(PlanMasterControllerTest)
    UT_REGISTER_TEST(QGCMapPolygonTest)
    UT_REGISTER_TEST(QGCMapPolylineTest)
    // UT_REGISTER_TEST(SectionTest)
    UT_REGISTER_TEST(SimpleMissionItemTest)
    UT_REGISTER_TEST(SpeedSectionTest)
    UT_REGISTER_TEST(StructureScanComplexItemTest)
    UT_REGISTER_TEST(SurveyComplexItemTest)
    UT_REGISTER_TEST(TransectStyleComplexItemTest)
    // UT_REGISTER_TEST(VisualMissionItemTest)

    // Terrain
    UT_REGISTER_TEST(TerrainQueryTest)
    UT_REGISTER_TEST(TerrainTileTest)

    // UnitTestFramework
    UT_REGISTER_TEST(MultiSignalSpyTest)

    // Utilities - Audio
    UT_REGISTER_TEST(AudioOutputTest)

    // Utilities - Compression
    UT_REGISTER_TEST(QGCArchiveModelTest)
    UT_REGISTER_TEST(QGCCompressionTest)
    UT_REGISTER_TEST(QGCStreamingDecompressionTest)

    // Utilities - FileSystem
    UT_REGISTER_TEST(QGCArchiveWatcherTest)
    UT_REGISTER_TEST(QGCFileDownloadTest)
    UT_REGISTER_TEST(QGCFileHelperTest)
    UT_REGISTER_TEST(QGCFileWatcherTest)

    // Utilities - Geo
    UT_REGISTER_TEST(GeoTest)

    // Utilities - Platform
    UT_REGISTER_TEST(PlatformTest)
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    UT_REGISTER_TEST(RunGuardTest)
    UT_REGISTER_TEST(SignalHandlerTest)
#endif

    // Utilities - Shape
    UT_REGISTER_TEST(ShapeTest)

    // Vehicle
    UT_REGISTER_TEST(ComponentInformationCacheTest)
    UT_REGISTER_TEST(ComponentInformationTranslationTest)
    UT_REGISTER_TEST(FTPManagerTest)
    // UT_REGISTER_TEST(InitialConnectTest)
    UT_REGISTER_TEST(MAVLinkLogManagerTest)
    // UT_REGISTER_TEST(RequestMessageTest)
    // UT_REGISTER_TEST(SendMavCommandWithHandlerTest)
    // UT_REGISTER_TEST(SendMavCommandWithSignalingTest)
    UT_REGISTER_TEST(VehicleLinkManagerTest)
}

} // anonymous namespace

namespace QGCUnitTest {

int runTests(bool stress, const QStringList& unitTests, const QString& outputFile)
{
    registerAllTests();

    if (unitTests.isEmpty()) {
        qCWarning(UnitTestListLog) << "No tests specified";
        return -1;
    }

    // Validate all test names before running
    const QStringList invalid = validateTestNames(unitTests);
    if (!invalid.isEmpty()) {
        qCWarning(UnitTestListLog) << "Unknown test(s):" << invalid.join(", ");
        qCWarning(UnitTestListLog) << "Available tests:" << UnitTest::registeredTests().join(", ");
        return -static_cast<int>(invalid.size());
    }

    const int iterations = stress ? kStressIterations : 1;
    int result = 0;

    for (int i = 0; i < iterations; ++i) {
        int failures = 0;

        for (const QString& test : unitTests) {
            failures += UnitTest::run(test, outputFile);
        }

        if (failures == 0) {
            if (stress) {
                qCDebug(UnitTestListLog).noquote() << QString("ALL TESTS PASSED (iteration %1/%2)").arg(i + 1).arg(iterations);
            } else {
                qCDebug(UnitTestListLog) << "ALL TESTS PASSED";
            }
        } else {
            qCWarning(UnitTestListLog) << failures << "TESTS FAILED!";
            result = -failures;
            break;
        }
    }

    return result;
}

QStringList registeredTestNames()
{
    registerAllTests();
    return UnitTest::registeredTests();
}

int registeredTestCount()
{
    registerAllTests();
    return UnitTest::testCount();
}

bool isTestRegistered(const QString& testName)
{
    registerAllTests();
    return UnitTest::registeredTests().contains(testName);
}

QStringList validateTestNames(const QStringList& testNames)
{
    registerAllTests();
    const QStringList registered = UnitTest::registeredTests();

    // Use QSet for O(1) lookup instead of O(n) QStringList::contains()
    const QSet<QString> registeredSet(registered.cbegin(), registered.cend());

    QStringList invalid;
    invalid.reserve(testNames.size());

    for (const QString& name : testNames) {
        if (!registeredSet.contains(name)) {
            invalid.append(name);
        }
    }

    return invalid;
}

int handleTestOptions(const QGCCommandLineParser::CommandLineParseResult& args)
{
    if (args.listTests) {
        const QStringList tests = registeredTestNames();
        qCInfo(UnitTestListLog) << "Available unit tests:" << tests.count();
        for (const QString& test : tests) {
            qInfo().noquote() << "  " << test;
        }
        return 0;
    }

    if (args.runningUnitTests) {
        const int testCount = args.unitTests.isEmpty()
            ? registeredTestCount()
            : args.unitTests.count();
        qCInfo(UnitTestListLog).noquote() << QString("Running %1 unit test(s)...").arg(testCount);

        QElapsedTimer timer;
        timer.start();

        const int exitCode = runTests(
            args.stressUnitTests,
            args.unitTests,
            args.unitTestOutput.value_or(QString())
        );

        const qint64 elapsed = timer.elapsed();
        if (exitCode == 0) {
            qCInfo(UnitTestListLog).noquote() << QString("All %1 test(s) passed in %2 ms")
                .arg(testCount).arg(elapsed);
        } else {
            qCWarning(UnitTestListLog).noquote() << QString("%1 test(s) failed (ran in %2 ms)")
                .arg(-exitCode).arg(elapsed);
        }
        return exitCode;
    }

    return 0;
}

} // namespace QGCUnitTest
