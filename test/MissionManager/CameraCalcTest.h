#pragma once

#include "UnitTest.h"

class MultiSignalSpy;
class CameraCalc;
class PlanMasterController;

class CameraCalcTest : public UnitTest
{
    Q_OBJECT

public:
    CameraCalcTest(void);

protected:
    void init(void) final;
    void cleanup(void) final;

private slots:
    void _testDirty             (void);
    void _testAdjustedFootprint (void);
    void _testAltDensityRecalc  (void);

private:
    PlanMasterController*   _masterController   = nullptr;
    Vehicle*                _controllerVehicle  = nullptr;
    MultiSignalSpy*         _multiSpy           = nullptr;
    CameraCalc*             _cameraCalc         = nullptr;
};
