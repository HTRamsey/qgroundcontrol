#include "QGCOAuth2Test.h"
#include "QGCOAuth2.h"

#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

// ============================================================================
// Construction Tests
// ============================================================================

void QGCOAuth2Test::_testConstruction()
{
    QGCOAuth2 oauth(this);
    QVERIFY(!oauth.isRunning());
}

void QGCOAuth2Test::_testInitialState()
{
    QGCOAuth2 oauth(this);

    QVERIFY(!oauth.isAuthenticated());
    QVERIFY(!oauth.isRunning());
    QVERIFY(oauth.accessToken().isEmpty());
    QVERIFY(oauth.refreshToken().isEmpty());
    QVERIFY(oauth.errorString().isEmpty());
    QVERIFY(!oauth.tokenExpiry().isValid());
    QCOMPARE(oauth.expiresIn(), -1);
    QVERIFY(oauth.usePKCE());
    QVERIFY(oauth.autoRefresh());
    QCOMPARE(oauth.autoRefreshBuffer(), 60);
}

// ============================================================================
// Configuration Tests
// ============================================================================

void QGCOAuth2Test::_testSetAuthorizationUrl()
{
    QGCOAuth2 oauth(this);

    QUrl url("https://auth.example.com/authorize");
    oauth.setAuthorizationUrl(url);
    QCOMPARE(oauth.authorizationUrl(), url);
}

void QGCOAuth2Test::_testSetTokenUrl()
{
    QGCOAuth2 oauth(this);

    QUrl url("https://auth.example.com/token");
    oauth.setTokenUrl(url);
    QCOMPARE(oauth.tokenUrl(), url);
}

void QGCOAuth2Test::_testSetClientCredentials()
{
    QGCOAuth2 oauth(this);

    oauth.setClientId("my-client-id");
    oauth.setClientSecret("my-client-secret");
    oauth.setRedirectUri("http://localhost:8080/callback");

    QCOMPARE(oauth.clientId(), QStringLiteral("my-client-id"));
    QCOMPARE(oauth.redirectUri(), QStringLiteral("http://localhost:8080/callback"));
}

void QGCOAuth2Test::_testSetScope()
{
    QGCOAuth2 oauth(this);

    oauth.setScope("openid profile email");
    QCOMPARE(oauth.scope(), QStringLiteral("openid profile email"));
}

// ============================================================================
// Token Management Tests
// ============================================================================

void QGCOAuth2Test::_testSetTokens()
{
    QGCOAuth2 oauth(this);

    QSignalSpy accessSpy(&oauth, &QGCOAuth2::accessTokenChanged);
    QSignalSpy refreshSpy(&oauth, &QGCOAuth2::refreshTokenChanged);
    QSignalSpy authSpy(&oauth, &QGCOAuth2::authenticatedChanged);
    QSignalSpy expirySpy(&oauth, &QGCOAuth2::tokenExpiryChanged);

    QDateTime expiry = QDateTime::currentDateTime().addSecs(3600);
    oauth.setTokens("access-token-123", "refresh-token-456", expiry);

    QCOMPARE(oauth.accessToken(), QStringLiteral("access-token-123"));
    QCOMPARE(oauth.refreshToken(), QStringLiteral("refresh-token-456"));
    QVERIFY(oauth.isAuthenticated());
    QCOMPARE(oauth.tokenExpiry(), expiry);

    QCOMPARE(accessSpy.count(), 1);
    QCOMPARE(refreshSpy.count(), 1);
    QCOMPARE(authSpy.count(), 1);
    QCOMPARE(expirySpy.count(), 1);
}

void QGCOAuth2Test::_testClearTokens()
{
    QGCOAuth2 oauth(this);

    oauth.setTokens("access-token", "refresh-token", QDateTime::currentDateTime().addSecs(3600));
    QVERIFY(oauth.isAuthenticated());

    QSignalSpy authSpy(&oauth, &QGCOAuth2::authenticatedChanged);

    oauth.clearTokens();

    QVERIFY(!oauth.isAuthenticated());
    QVERIFY(oauth.accessToken().isEmpty());
    QVERIFY(oauth.refreshToken().isEmpty());
    QVERIFY(!oauth.tokenExpiry().isValid());
    QCOMPARE(authSpy.count(), 1);
}

void QGCOAuth2Test::_testTokenExpiry()
{
    QGCOAuth2 oauth(this);

    // Token with future expiry
    oauth.setTokens("token", {}, QDateTime::currentDateTime().addSecs(3600));
    QVERIFY(!oauth.isTokenExpired());
    QVERIFY(oauth.expiresIn() > 3500);

    // Token with past expiry
    oauth.setTokens("token", {}, QDateTime::currentDateTime().addSecs(-100));
    QVERIFY(oauth.isTokenExpired());
    QVERIFY(oauth.expiresIn() < 0);
}

void QGCOAuth2Test::_testTokenExpiresWithin()
{
    QGCOAuth2 oauth(this);

    // Token expires in 30 seconds
    oauth.setTokens("token", {}, QDateTime::currentDateTime().addSecs(30));

    QVERIFY(oauth.tokenExpiresWithin(60));   // Expires within 60 seconds
    QVERIFY(!oauth.tokenExpiresWithin(10));  // Does not expire within 10 seconds
}

// ============================================================================
// PKCE Tests
// ============================================================================

void QGCOAuth2Test::_testPKCEEnabled()
{
    QGCOAuth2 oauth(this);

    QVERIFY(oauth.usePKCE());  // Default is enabled

    oauth.setUsePKCE(false);
    QVERIFY(!oauth.usePKCE());

    oauth.setUsePKCE(true);
    QVERIFY(oauth.usePKCE());
}

void QGCOAuth2Test::_testAuthorizationCodeFlowUrl()
{
    QGCOAuth2 oauth(this);

    oauth.setAuthorizationUrl(QUrl("https://auth.example.com/authorize"));
    oauth.setClientId("test-client");
    oauth.setRedirectUri("http://localhost:8080/callback");
    oauth.setScope("openid");
    oauth.setUsePKCE(true);

    QSignalSpy urlSpy(&oauth, &QGCOAuth2::authorizationUrlReady);

    QUrl authUrl = oauth.startAuthorizationCodeFlow();

    QVERIFY(authUrl.isValid());
    QCOMPARE(urlSpy.count(), 1);

    QUrlQuery query(authUrl);
    QCOMPARE(query.queryItemValue("response_type"), QStringLiteral("code"));
    QCOMPARE(query.queryItemValue("client_id"), QStringLiteral("test-client"));
    QVERIFY(!query.queryItemValue("state").isEmpty());
    QVERIFY(!query.queryItemValue("code_challenge").isEmpty());
    QCOMPARE(query.queryItemValue("code_challenge_method"), QStringLiteral("S256"));
}

// ============================================================================
// Auto-refresh Tests
// ============================================================================

void QGCOAuth2Test::_testAutoRefreshConfig()
{
    QGCOAuth2 oauth(this);

    QVERIFY(oauth.autoRefresh());
    QCOMPARE(oauth.autoRefreshBuffer(), 60);

    oauth.setAutoRefresh(false);
    QVERIFY(!oauth.autoRefresh());

    oauth.setAutoRefreshBuffer(120);
    QCOMPARE(oauth.autoRefreshBuffer(), 120);
}

// ============================================================================
// Request Validation Tests
// ============================================================================

void QGCOAuth2Test::_testClientCredentialsMissingConfig()
{
    QGCOAuth2 oauth(this);

    QSignalSpy failedSpy(&oauth, &QGCOAuth2::authenticationFailed);

    // Missing required config
    oauth.requestClientCredentials();

    QCOMPARE(failedSpy.count(), 1);
    QVERIFY(!oauth.errorString().isEmpty());
}

void QGCOAuth2Test::_testRefreshTokenMissingConfig()
{
    QGCOAuth2 oauth(this);

    QSignalSpy failedSpy(&oauth, &QGCOAuth2::authenticationFailed);

    // Missing required config
    oauth.refreshAccessToken();

    QCOMPARE(failedSpy.count(), 1);
    QVERIFY(!oauth.errorString().isEmpty());
}

// ============================================================================
// Signal Tests
// ============================================================================

void QGCOAuth2Test::_testTokenSignals()
{
    QGCOAuth2 oauth(this);

    QSignalSpy accessSpy(&oauth, &QGCOAuth2::accessTokenChanged);
    QSignalSpy authSpy(&oauth, &QGCOAuth2::authenticatedChanged);
    QSignalSpy saveSpy(&oauth, &QGCOAuth2::saveTokensRequested);

    oauth.setTokens("token", "refresh", QDateTime::currentDateTime().addSecs(3600));

    QCOMPARE(accessSpy.count(), 1);
    QCOMPARE(authSpy.count(), 1);

    // Save signal is only emitted during token exchange, not setTokens()
    QCOMPARE(saveSpy.count(), 0);
}
