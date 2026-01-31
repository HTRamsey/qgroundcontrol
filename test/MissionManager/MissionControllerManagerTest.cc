#include "MissionControllerManagerTest.h"
#include "MultiVehicleManager.h"
#include "Vehicle.h"
#include "MultiSignalSpy.h"

#include <QtTest/QTest>

MissionControllerManagerTest::MissionControllerManagerTest(void)
{

}

void MissionControllerManagerTest::cleanup(void)
{
    delete _multiSpyMissionManager;
    _multiSpyMissionManager = nullptr;

    UnitTest::cleanup();
}

void MissionControllerManagerTest::_initForFirmwareType(MAV_AUTOPILOT firmwareType)
{
    _connectMockLink(firmwareType);

    // Wait for the Mission Manager to finish it's initial load

    _missionManager = MultiVehicleManager::instance()->activeVehicle()->missionManager();
    QVERIFY(_missionManager);

    _multiSpyMissionManager = new MultiSignalSpy();
    Q_CHECK_PTR(_multiSpyMissionManager);
    QCOMPARE(_multiSpyMissionManager->init(_missionManager), true);

    if (_missionManager->inProgress()) {
        _multiSpyMissionManager->waitForSignal(SIGNAL(newMissionItemsAvailable(bool)), _missionManagerSignalWaitTime);
        _multiSpyMissionManager->waitForSignal(SIGNAL(inProgressChanged(bool)), _missionManagerSignalWaitTime);
        QCOMPARE(_multiSpyMissionManager->checkSignalsByMask(_multiSpyMissionManager->mask(SIGNAL(newMissionItemsAvailable(bool)), SIGNAL(inProgressChanged(bool)))), true);
    }

    QVERIFY(!_missionManager->inProgress());
    QCOMPARE(_missionManager->missionItems().count(), 0);
    _multiSpyMissionManager->clearAllSignals();
}

/// Checks the state of the inProgress value and signal to match the specified value
void MissionControllerManagerTest::_checkInProgressValues(bool inProgress)
{
    QCOMPARE(_missionManager->inProgress(), inProgress);
    QCOMPARE(_multiSpyMissionManager->pullBoolFromSignal(SIGNAL(inProgressChanged(bool))), inProgress);
}
