#pragma once

#include "UnitTest.h"

class ParameterCacheTest : public UnitTest
{
    Q_OBJECT

private slots:
    void init() override;
    void cleanup() override;

    void _cacheFilePath();
    void _writeLoadRoundTripCrcMatch();
    void _loadCrcMismatch();
    void _loadFileNotFound();
    void _loadOpenFailed();
    void _loadVolatileCheckExcludes();
};
