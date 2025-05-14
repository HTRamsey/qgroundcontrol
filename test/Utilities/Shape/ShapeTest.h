#pragma once

#include "UnitTest.h"

class ShapeTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _testLoadPolylineFromSHP();
    void _testLoadPolygonFromSHP();
};
