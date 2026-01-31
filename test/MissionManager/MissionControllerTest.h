#pragma once

#include "MissionControllerManagerTest.h"

class MissionController;
class MultiSignalSpy;
class PlanMasterController;
class VisualMissionItem;

class MissionControllerTest : public MissionControllerManagerTest
{
    Q_OBJECT

public:
    MissionControllerTest(void);

private slots:
    void cleanup(void);

    void _testLoadJsonSectionAvailable  (void);
    void _testEmptyVehicleAPM           (void);
    void _testEmptyVehiclePX4           (void);
    void _testGlobalAltMode             (void);
    void _testGimbalRecalc              (void);
    void _testVehicleYawRecalc          (void);

private:
#if 0
    void _testOfflineToOnlineAPM(void);
    void _testOfflineToOnlinePX4(void);
#endif

private:
    void _initForFirmwareType(MAV_AUTOPILOT firmwareType);
    void _testEmptyVehicleWorker(MAV_AUTOPILOT firmwareType);
    void _testAddWaypointWorker(MAV_AUTOPILOT firmwareType);
#if 0
    void _testOfflineToOnlineWorker(MAV_AUTOPILOT firmwareType);
#endif
    void _setupVisualItemSignals(VisualMissionItem* visualItem);

    MultiSignalSpy*         _multiSpyMissionController  = nullptr;
    MultiSignalSpy*         _multiSpyMissionItem        = nullptr;
    PlanMasterController*   _masterController           = nullptr;
    MissionController*      _missionController          = nullptr;
};
