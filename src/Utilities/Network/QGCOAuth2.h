#pragma once

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(QGCOAuth2Log)

class QNetworkAccessManager;
class QNetworkReply;

/// OAuth2 authentication helper supporting various grant types.
/// Features:
/// - Authorization Code flow (with PKCE support)
/// - Client Credentials flow
/// - Refresh Token flow
/// - Automatic token refresh before expiry
/// - Token persistence (via signals)
/// - QML-compatible
class QGCOAuth2 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString accessToken READ accessToken NOTIFY accessTokenChanged)
    Q_PROPERTY(QString refreshToken READ refreshToken NOTIFY refreshTokenChanged)
    Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QDateTime tokenExpiry READ tokenExpiry NOTIFY tokenExpiryChanged)
    Q_PROPERTY(int expiresIn READ expiresIn NOTIFY expiresInChanged)

public:
    /// OAuth2 grant types
    enum class GrantType {
        AuthorizationCode,  ///< Standard 3-legged OAuth
        ClientCredentials,  ///< Machine-to-machine
        RefreshToken        ///< Token refresh
    };
    Q_ENUM(GrantType)

    explicit QGCOAuth2(QObject *parent = nullptr);
    ~QGCOAuth2() override;

    // Configuration
    void setAuthorizationUrl(const QUrl &url) { _authorizationUrl = url; }
    void setTokenUrl(const QUrl &url) { _tokenUrl = url; }
    void setClientId(const QString &clientId) { _clientId = clientId; }
    void setClientSecret(const QString &clientSecret) { _clientSecret = clientSecret; }
    void setRedirectUri(const QString &uri) { _redirectUri = uri; }
    void setScope(const QString &scope) { _scope = scope; }

    QUrl authorizationUrl() const { return _authorizationUrl; }
    QUrl tokenUrl() const { return _tokenUrl; }
    QString clientId() const { return _clientId; }
    QString redirectUri() const { return _redirectUri; }
    QString scope() const { return _scope; }

    // Token state
    QString accessToken() const { return _accessToken; }
    QString refreshToken() const { return _refreshToken; }
    bool isAuthenticated() const { return !_accessToken.isEmpty() && !isTokenExpired(); }
    bool isRunning() const { return _running; }
    QString errorString() const { return _errorString; }
    QDateTime tokenExpiry() const { return _tokenExpiry; }
    int expiresIn() const;
    bool isTokenExpired() const;

    /// Check if token will expire within the given seconds
    bool tokenExpiresWithin(int seconds) const;

    /// Set tokens directly (for restoring from storage)
    void setTokens(const QString &accessToken, const QString &refreshToken = {},
                   const QDateTime &expiry = {});

    /// Clear all tokens
    Q_INVOKABLE void clearTokens();

    /// Enable PKCE (Proof Key for Code Exchange) for Authorization Code flow
    void setUsePKCE(bool enabled) { _usePKCE = enabled; }
    bool usePKCE() const { return _usePKCE; }

    /// Enable auto-refresh of tokens before expiry
    void setAutoRefresh(bool enabled);
    bool autoRefresh() const { return _autoRefresh; }

    /// Set seconds before expiry to trigger auto-refresh (default: 60)
    void setAutoRefreshBuffer(int seconds) { _autoRefreshBuffer = seconds; }
    int autoRefreshBuffer() const { return _autoRefreshBuffer; }

public slots:
    /// Start Authorization Code flow
    /// @return Authorization URL to open in browser
    QUrl startAuthorizationCodeFlow();

    /// Complete Authorization Code flow with the code from redirect
    /// @param authorizationCode The code received from authorization server
    void exchangeCodeForToken(const QString &authorizationCode);

    /// Start Client Credentials flow (machine-to-machine)
    void requestClientCredentials();

    /// Refresh the access token using refresh token
    void refreshAccessToken();

    /// Cancel any pending request
    void cancel();

signals:
    void accessTokenChanged(const QString &token);
    void refreshTokenChanged(const QString &token);
    void authenticatedChanged(bool authenticated);
    void runningChanged(bool running);
    void errorStringChanged(const QString &error);
    void tokenExpiryChanged(const QDateTime &expiry);
    void expiresInChanged(int seconds);

    /// Emitted when authentication succeeds
    void authenticationSucceeded();

    /// Emitted when authentication fails
    void authenticationFailed(const QString &errorMessage);

    /// Emitted when tokens are refreshed
    void tokensRefreshed();

    /// Emitted when tokens should be saved to persistent storage
    /// @param accessToken The access token
    /// @param refreshToken The refresh token
    /// @param expiry Token expiry time
    void saveTokensRequested(const QString &accessToken, const QString &refreshToken,
                              const QDateTime &expiry);

    /// Emitted when authorization URL is ready (for opening in browser)
    void authorizationUrlReady(const QUrl &url);

private slots:
    void _onTokenResponse();
    void _onAutoRefreshTimer();

private:
    void _setAccessToken(const QString &token);
    void _setRefreshToken(const QString &token);
    void _setTokenExpiry(const QDateTime &expiry);
    void _setRunning(bool running);
    void _setErrorString(const QString &error);
    void _parseTokenResponse(const QByteArray &response);
    void _startAutoRefreshTimer();
    QString _generateCodeVerifier() const;
    QString _generateCodeChallenge(const QString &verifier) const;
    QString _generateState() const;

    QNetworkAccessManager *_networkManager = nullptr;
    QNetworkReply *_currentReply = nullptr;
    class QTimer *_autoRefreshTimer = nullptr;

    // Configuration
    QUrl _authorizationUrl;
    QUrl _tokenUrl;
    QString _clientId;
    QString _clientSecret;
    QString _redirectUri;
    QString _scope;

    // Token state
    QString _accessToken;
    QString _refreshToken;
    QDateTime _tokenExpiry;
    QString _errorString;
    bool _running = false;

    // PKCE
    bool _usePKCE = true;
    QString _codeVerifier;
    QString _state;

    // Auto-refresh
    bool _autoRefresh = true;
    int _autoRefreshBuffer = 60;
};
