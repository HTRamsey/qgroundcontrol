#pragma once

#include "UnitTest.h"

class PlanMasterController;
class MultiSignalSpy;
class VisualMissionItem;

/// Unit test for SimpleMissionItem
class VisualMissionItemTest : public UnitTest
{
    Q_OBJECT

public:
    VisualMissionItemTest(void);

    void init(void) override;
    void cleanup(void) override;

protected:
    void _createSpy(VisualMissionItem* visualItem, MultiSignalSpy** visualSpy);

    PlanMasterController*   _masterController =     nullptr;
    Vehicle*                _controllerVehicle =    nullptr;
};
