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
#include "MockConfiguration.h"

class ParameterManagerTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _noFailure();
    /// Test no response to param_request_list
    void _requestListNoResponse();
    void _requestListMissingParamSuccess();
    /// MockLink will fail to send a param on initial request, it will also fail to send it on subsequent param_read requests.
    void _requestListMissingParamFail();
    void _FTPnoFailure();
    // void _FTPChangeParam();

private:
    /// Test failure modes which should still lead to param load success
    void _noFailureWorker(MockConfiguration::FailureMode_t failureMode);
};
