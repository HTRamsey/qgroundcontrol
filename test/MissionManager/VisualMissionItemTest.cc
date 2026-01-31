#include "VisualMissionItemTest.h"
#include "MultiSignalSpy.h"
#include "PlanMasterController.h"
#include "VisualMissionItem.h"

#include <QtTest/QTest>

VisualMissionItemTest::VisualMissionItemTest(void)
{

}

void VisualMissionItemTest::init(void)
{
    UnitTest::init();

    _masterController = new PlanMasterController(MAV_AUTOPILOT_PX4, MAV_TYPE_QUADROTOR, this);
    _controllerVehicle = _masterController->controllerVehicle();
}

void VisualMissionItemTest::cleanup(void)
{
    delete _masterController;

    _masterController   = nullptr;
    _controllerVehicle  = nullptr;

    UnitTest::cleanup();
}

void VisualMissionItemTest::_createSpy(VisualMissionItem* visualItem, MultiSignalSpy** visualSpy)
{
    *visualSpy = nullptr;
    MultiSignalSpy* spy = new MultiSignalSpy();
    QVERIFY(spy->init(visualItem));
    *visualSpy = spy;
}
