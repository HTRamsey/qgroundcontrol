#pragma once

#include "UnitTest.h"

class VehicleGPSFactGroupTest : public UnitTest
{
    Q_OBJECT

private slots:
    void testQualityNone();
    void testQualityPoor();
    void testQualityFair();
    void testQualityGood();
    void testQualityExcellent();
    void testQualityChangedEmittedOnGpsRawInt();
    void testQualityRecomputedOnDirectFactChange();
};
