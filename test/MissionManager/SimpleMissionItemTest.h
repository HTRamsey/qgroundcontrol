#pragma once

#include "VisualMissionItemTest.h"
#include "QGroundControlQmlGlobal.h"
#include "QGCMAVLink.h"

class SimpleMissionItem;

/// Unit test for SimpleMissionItem

class SimpleMissionItemTest : public VisualMissionItemTest
{
    Q_OBJECT

public:
    SimpleMissionItemTest(void);

    void init   (void) override;
    void cleanup(void) override;

private slots:
    void _testSignals               (void);
    void _testEditorFacts           (void);
    void _testDefaultValues         (void);
    void _testCameraSectionDirty    (void);
    void _testSpeedSectionDirty     (void);
    void _testCameraSection         (void);
    void _testSpeedSection          (void);
    void _testAltitudePropogation   (void);

private:
    void _testEditorFactsWorker (QGCMAVLink::VehicleClass_t vehicleClass, QGCMAVLink::VehicleClass_t vtolMode);
    bool _classMatch            (QGCMAVLink::VehicleClass_t vehicleClass, QGCMAVLink::VehicleClass_t testClass);

    SimpleMissionItem*  _simpleItem = nullptr;
    MultiSignalSpy*     _spySimpleItem = nullptr;
    MultiSignalSpy*     _spyVisualItem = nullptr;
};
