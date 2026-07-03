#pragma once

#include "UnitTest.h"

class PositionManagerTest : public UnitTest
{
    Q_OBJECT

private slots:
    void cleanup();
    void testPollDisabledLeavesDeviceClosed();
    void testUdpNmeaOpenBindsPort();
    void testUdpNmeaPositionUpdate();
    void testDisableAfterOpenClosesPort();
};
