#pragma once

#include "UnitTest.h"

class QGCFormatterTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _testFormatDistance();
    void _testFormatSpeed();
    void _testFormatCoordinates();
    void _testFormatDuration();
    void _testFormatBytes();
};
