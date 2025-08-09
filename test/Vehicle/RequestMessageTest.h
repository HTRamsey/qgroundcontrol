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
#include "MAVLinkLib.h"
#include "MockLink.h"
#include "Vehicle.h"

class RequestMessageTest : public UnitTest
{
    Q_OBJECT

signals:
    void resultHandlerCalled();
    
private slots:
    void _performTestCases();
    void _compIdAllFailure();
    void _duplicateCommand();

private:
    struct TestCase_t {
        MockLink::RequestMessageFailureMode_t               failureMode;
        MAV_RESULT                                          expectedCommandResult;
        Vehicle::RequestMessageResultHandlerFailureCode_t   expectedFailureCode;
        int                                                 expectedSendCount;
        bool                                                resultHandlerCalled;
    };

    void _testCaseWorker(TestCase_t& testCase);

    static void _requestMessageResultHandler(void* resultHandlerData, MAV_RESULT commandResult, Vehicle::RequestMessageResultHandlerFailureCode_t failureCode, const mavlink_message_t& message);
    static void _compIdAllRequestMessageResultHandler(void* resultHandlerData, MAV_RESULT commandResult, Vehicle::RequestMessageResultHandlerFailureCode_t failureCode, const mavlink_message_t& message);

    bool _resultHandlerCalled = false;

    static TestCase_t _rgTestCases[];
};
