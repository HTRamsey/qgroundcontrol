#pragma once

#include "UnitTest.h"

class ExifParserTest : public UnitTest
{
    Q_OBJECT

private slots:
	void _readTimeTest();
	void _writeTest();
};
