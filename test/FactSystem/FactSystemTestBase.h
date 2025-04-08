/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "UnitTest.h"

class AutoPilotPlugin;

/// Base class for FactSystemTest[PX4|Generic] unit tests
class FactSystemTestBase : public UnitTest
{
    Q_OBJECT

protected:
    void _init(MAV_AUTOPILOT autopilot);
    void _cleanup();

    /// Basic test of parameter values in Fact System
    void _parameter_default_component_id_test();
    void _parameter_specific_component_id_test();
    /// Test that QML can reference a Fact
    void _qml_test();
    /// Test QML getting an updated Fact value
    void _qmlUpdate_test();
};
