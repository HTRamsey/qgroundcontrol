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

class FTPManagerTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _testLostPackets();
    void _testListDirectory();
    void _testListDirectoryNoResponse();
    void _testListDirectoryNakResponse();
    void _testListDirectoryNoSecondResponse();
    void _testListDirectoryNoSecondResponseAllowRetry();
    void _testListDirectoryNakSecondResponse();
    void _testListDirectoryBadSequence();

    // Overrides from UnitTest
    void cleanup(void) override;

private:
    void _performSizeBasedTestCases();
    void _performTestCases();

    struct TestCase_t {
        const char* file;
    };

    void _testCaseWorker(const TestCase_t& testCase);
    void _sizeTestCaseWorker(int fileSize);
    void _verifyFileSizeAndDelete(const QString& filename, int expectedSize);

    static const TestCase_t _rgTestCases[];
};
