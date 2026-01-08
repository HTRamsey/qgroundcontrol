#pragma once

#include "UnitTest.h"

class QGCOAuth2Test : public UnitTest
{
    Q_OBJECT

private slots:
    // Construction tests
    void _testConstruction();
    void _testInitialState();

    // Configuration tests
    void _testSetAuthorizationUrl();
    void _testSetTokenUrl();
    void _testSetClientCredentials();
    void _testSetScope();

    // Token management tests
    void _testSetTokens();
    void _testClearTokens();
    void _testTokenExpiry();
    void _testTokenExpiresWithin();

    // PKCE tests
    void _testPKCEEnabled();
    void _testAuthorizationCodeFlowUrl();

    // Auto-refresh tests
    void _testAutoRefreshConfig();

    // Request validation tests
    void _testClientCredentialsMissingConfig();
    void _testRefreshTokenMissingConfig();

    // Signal tests
    void _testTokenSignals();
};
