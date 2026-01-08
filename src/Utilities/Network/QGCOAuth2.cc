#include "QGCOAuth2.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QRandomGenerator>
#include <QtCore/QTimer>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

Q_LOGGING_CATEGORY(QGCOAuth2Log, "Utilities.QGCOAuth2")

QGCOAuth2::QGCOAuth2(QObject *parent)
    : QObject(parent)
    , _networkManager(new QNetworkAccessManager(this))
    , _autoRefreshTimer(new QTimer(this))
{
    _autoRefreshTimer->setSingleShot(true);
    connect(_autoRefreshTimer, &QTimer::timeout, this, &QGCOAuth2::_onAutoRefreshTimer);
}

QGCOAuth2::~QGCOAuth2()
{
    cancel();
}

int QGCOAuth2::expiresIn() const
{
    if (!_tokenExpiry.isValid()) {
        return -1;
    }
    return static_cast<int>(QDateTime::currentDateTime().secsTo(_tokenExpiry));
}

bool QGCOAuth2::isTokenExpired() const
{
    if (!_tokenExpiry.isValid()) {
        return false;  // No expiry set, assume valid
    }
    return QDateTime::currentDateTime() >= _tokenExpiry;
}

bool QGCOAuth2::tokenExpiresWithin(int seconds) const
{
    if (!_tokenExpiry.isValid()) {
        return false;
    }
    return QDateTime::currentDateTime().addSecs(seconds) >= _tokenExpiry;
}

void QGCOAuth2::setTokens(const QString &accessToken, const QString &refreshToken,
                           const QDateTime &expiry)
{
    _setAccessToken(accessToken);
    _setRefreshToken(refreshToken);
    _setTokenExpiry(expiry);

    if (_autoRefresh && !accessToken.isEmpty()) {
        _startAutoRefreshTimer();
    }
}

void QGCOAuth2::clearTokens()
{
    _autoRefreshTimer->stop();
    _setAccessToken({});
    _setRefreshToken({});
    _setTokenExpiry({});

    qCDebug(QGCOAuth2Log) << "Tokens cleared";
}

void QGCOAuth2::setAutoRefresh(bool enabled)
{
    if (_autoRefresh == enabled) {
        return;
    }

    _autoRefresh = enabled;

    if (enabled && isAuthenticated()) {
        _startAutoRefreshTimer();
    } else {
        _autoRefreshTimer->stop();
    }
}

QUrl QGCOAuth2::startAuthorizationCodeFlow()
{
    if (_authorizationUrl.isEmpty() || _clientId.isEmpty()) {
        _setErrorString(tr("Authorization URL and client ID are required"));
        emit authenticationFailed(_errorString);
        return {};
    }

    // Generate state for CSRF protection
    _state = _generateState();

    QUrl url = _authorizationUrl;
    QUrlQuery query;
    query.addQueryItem("response_type", "code");
    query.addQueryItem("client_id", _clientId);
    query.addQueryItem("state", _state);

    if (!_redirectUri.isEmpty()) {
        query.addQueryItem("redirect_uri", _redirectUri);
    }
    if (!_scope.isEmpty()) {
        query.addQueryItem("scope", _scope);
    }

    // PKCE support
    if (_usePKCE) {
        _codeVerifier = _generateCodeVerifier();
        const QString challenge = _generateCodeChallenge(_codeVerifier);
        query.addQueryItem("code_challenge", challenge);
        query.addQueryItem("code_challenge_method", "S256");
    }

    url.setQuery(query);

    qCDebug(QGCOAuth2Log) << "Authorization URL generated:" << url.toString(QUrl::RemoveQuery);
    emit authorizationUrlReady(url);

    return url;
}

void QGCOAuth2::exchangeCodeForToken(const QString &authorizationCode)
{
    if (_running) {
        qCWarning(QGCOAuth2Log) << "Token request already in progress";
        return;
    }

    if (_tokenUrl.isEmpty() || _clientId.isEmpty()) {
        _setErrorString(tr("Token URL and client ID are required"));
        emit authenticationFailed(_errorString);
        return;
    }

    _setRunning(true);
    _setErrorString({});

    QUrlQuery params;
    params.addQueryItem("grant_type", "authorization_code");
    params.addQueryItem("code", authorizationCode);
    params.addQueryItem("client_id", _clientId);

    if (!_clientSecret.isEmpty()) {
        params.addQueryItem("client_secret", _clientSecret);
    }
    if (!_redirectUri.isEmpty()) {
        params.addQueryItem("redirect_uri", _redirectUri);
    }
    if (_usePKCE && !_codeVerifier.isEmpty()) {
        params.addQueryItem("code_verifier", _codeVerifier);
    }

    QNetworkRequest request(_tokenUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    qCDebug(QGCOAuth2Log) << "Exchanging authorization code for token";

    _currentReply = _networkManager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());
    connect(_currentReply, &QNetworkReply::finished, this, &QGCOAuth2::_onTokenResponse);
}

void QGCOAuth2::requestClientCredentials()
{
    if (_running) {
        qCWarning(QGCOAuth2Log) << "Token request already in progress";
        return;
    }

    if (_tokenUrl.isEmpty() || _clientId.isEmpty() || _clientSecret.isEmpty()) {
        _setErrorString(tr("Token URL, client ID, and client secret are required"));
        emit authenticationFailed(_errorString);
        return;
    }

    _setRunning(true);
    _setErrorString({});

    QUrlQuery params;
    params.addQueryItem("grant_type", "client_credentials");
    params.addQueryItem("client_id", _clientId);
    params.addQueryItem("client_secret", _clientSecret);

    if (!_scope.isEmpty()) {
        params.addQueryItem("scope", _scope);
    }

    QNetworkRequest request(_tokenUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    qCDebug(QGCOAuth2Log) << "Requesting client credentials token";

    _currentReply = _networkManager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());
    connect(_currentReply, &QNetworkReply::finished, this, &QGCOAuth2::_onTokenResponse);
}

void QGCOAuth2::refreshAccessToken()
{
    if (_running) {
        qCWarning(QGCOAuth2Log) << "Token request already in progress";
        return;
    }

    if (_tokenUrl.isEmpty() || _clientId.isEmpty() || _refreshToken.isEmpty()) {
        _setErrorString(tr("Token URL, client ID, and refresh token are required"));
        emit authenticationFailed(_errorString);
        return;
    }

    _setRunning(true);
    _setErrorString({});

    QUrlQuery params;
    params.addQueryItem("grant_type", "refresh_token");
    params.addQueryItem("refresh_token", _refreshToken);
    params.addQueryItem("client_id", _clientId);

    if (!_clientSecret.isEmpty()) {
        params.addQueryItem("client_secret", _clientSecret);
    }

    QNetworkRequest request(_tokenUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    qCDebug(QGCOAuth2Log) << "Refreshing access token";

    _currentReply = _networkManager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());
    connect(_currentReply, &QNetworkReply::finished, this, &QGCOAuth2::_onTokenResponse);
}

void QGCOAuth2::cancel()
{
    if (_currentReply != nullptr) {
        _currentReply->abort();
        _currentReply->deleteLater();
        _currentReply = nullptr;
    }
    _setRunning(false);
}

void QGCOAuth2::_onTokenResponse()
{
    if (_currentReply == nullptr) {
        return;
    }

    const QByteArray response = _currentReply->readAll();
    const int httpStatus = _currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (_currentReply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300) {
        QString errorMsg = _currentReply->errorString();

        // Try to extract error from response body
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("error_description")) {
                errorMsg = obj["error_description"].toString();
            } else if (obj.contains("error")) {
                errorMsg = obj["error"].toString();
            }
        }

        qCWarning(QGCOAuth2Log) << "Token request failed:" << errorMsg;
        _setErrorString(errorMsg);
        _setRunning(false);
        emit authenticationFailed(errorMsg);

        _currentReply->deleteLater();
        _currentReply = nullptr;
        return;
    }

    _currentReply->deleteLater();
    _currentReply = nullptr;

    _parseTokenResponse(response);
    _setRunning(false);
}

void QGCOAuth2::_parseTokenResponse(const QByteArray &response)
{
    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (!doc.isObject()) {
        _setErrorString(tr("Invalid token response"));
        emit authenticationFailed(_errorString);
        return;
    }

    QJsonObject obj = doc.object();

    const QString accessToken = obj["access_token"].toString();
    if (accessToken.isEmpty()) {
        _setErrorString(tr("No access token in response"));
        emit authenticationFailed(_errorString);
        return;
    }

    const bool wasAuthenticated = isAuthenticated();
    const bool hadRefreshToken = !_refreshToken.isEmpty();

    _setAccessToken(accessToken);

    if (obj.contains("refresh_token")) {
        _setRefreshToken(obj["refresh_token"].toString());
    }

    if (obj.contains("expires_in")) {
        const int expiresIn = obj["expires_in"].toInt();
        _setTokenExpiry(QDateTime::currentDateTime().addSecs(expiresIn));
    }

    qCDebug(QGCOAuth2Log) << "Token received - expires in:" << expiresIn() << "seconds";

    // Request token persistence
    emit saveTokensRequested(_accessToken, _refreshToken, _tokenExpiry);

    if (_autoRefresh) {
        _startAutoRefreshTimer();
    }

    if (hadRefreshToken) {
        emit tokensRefreshed();
    } else if (!wasAuthenticated) {
        emit authenticationSucceeded();
    }
}

void QGCOAuth2::_onAutoRefreshTimer()
{
    if (!isAuthenticated() || _refreshToken.isEmpty()) {
        return;
    }

    qCDebug(QGCOAuth2Log) << "Auto-refreshing token";
    refreshAccessToken();
}

void QGCOAuth2::_setAccessToken(const QString &token)
{
    if (_accessToken == token) {
        return;
    }

    const bool wasAuthenticated = isAuthenticated();
    _accessToken = token;
    emit accessTokenChanged(token);

    if (wasAuthenticated != isAuthenticated()) {
        emit authenticatedChanged(isAuthenticated());
    }
}

void QGCOAuth2::_setRefreshToken(const QString &token)
{
    if (_refreshToken == token) {
        return;
    }

    _refreshToken = token;
    emit refreshTokenChanged(token);
}

void QGCOAuth2::_setTokenExpiry(const QDateTime &expiry)
{
    if (_tokenExpiry == expiry) {
        return;
    }

    _tokenExpiry = expiry;
    emit tokenExpiryChanged(expiry);
    emit expiresInChanged(expiresIn());
}

void QGCOAuth2::_setRunning(bool running)
{
    if (_running == running) {
        return;
    }

    _running = running;
    emit runningChanged(running);
}

void QGCOAuth2::_setErrorString(const QString &error)
{
    if (_errorString == error) {
        return;
    }

    _errorString = error;
    emit errorStringChanged(error);
}

void QGCOAuth2::_startAutoRefreshTimer()
{
    if (!_tokenExpiry.isValid() || _refreshToken.isEmpty()) {
        return;
    }

    const int remainingSeconds = expiresIn();
    const int refreshAt = remainingSeconds - _autoRefreshBuffer;

    if (refreshAt <= 0) {
        // Token expires very soon, refresh immediately
        _onAutoRefreshTimer();
    } else {
        _autoRefreshTimer->start(refreshAt * 1000);
        qCDebug(QGCOAuth2Log) << "Auto-refresh scheduled in" << refreshAt << "seconds";
    }
}

QString QGCOAuth2::_generateCodeVerifier() const
{
    // Generate 32 random bytes (256 bits) for PKCE code verifier
    QByteArray randomBytes;
    randomBytes.resize(32);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32 *>(randomBytes.data()),
                                          randomBytes.size() / sizeof(quint32));

    // Base64url encode (no padding)
    return QString::fromLatin1(randomBytes.toBase64(QByteArray::Base64UrlEncoding |
                                                     QByteArray::OmitTrailingEquals));
}

QString QGCOAuth2::_generateCodeChallenge(const QString &verifier) const
{
    // SHA256 hash the verifier and base64url encode
    const QByteArray hash = QCryptographicHash::hash(verifier.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromLatin1(hash.toBase64(QByteArray::Base64UrlEncoding |
                                              QByteArray::OmitTrailingEquals));
}

QString QGCOAuth2::_generateState() const
{
    QByteArray randomBytes;
    randomBytes.resize(16);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32 *>(randomBytes.data()),
                                          randomBytes.size() / sizeof(quint32));
    return QString::fromLatin1(randomBytes.toHex());
}
