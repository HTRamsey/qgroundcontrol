#include "QGCJsonRequest.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QJsonParseError>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>

QGC_LOGGING_CATEGORY(QGCJsonRequestLog, "Utilities.QGCJsonRequest")

namespace {
constexpr const char *kContentTypeJson = "application/json";
constexpr const char *kAcceptJson = "application/json";
} // namespace

// ============================================================================
// Construction / Destruction
// ============================================================================

QGCJsonRequest::QGCJsonRequest(QObject *parent)
    : QObject(parent)
    , _networkManager(new QNetworkAccessManager(this))
{
    qCDebug(QGCJsonRequestLog) << "Created";
}

QGCJsonRequest::~QGCJsonRequest()
{
    cancel();
    qCDebug(QGCJsonRequestLog) << "Destroyed";
}

// ============================================================================
// Property Setters
// ============================================================================

void QGCJsonRequest::setBearerToken(const QString &token)
{
    if (_bearerToken != token) {
        _bearerToken = token;
        emit bearerTokenChanged(token);
        qCDebug(QGCJsonRequestLog) << "Bearer token" << (token.isEmpty() ? "cleared" : "set");
    }
}

void QGCJsonRequest::setBaseUrl(const QString &url)
{
    if (_baseUrl != url) {
        _baseUrl = url;
        emit baseUrlChanged(url);
        qCDebug(QGCJsonRequestLog) << "Base URL set to:" << url;
    }
}

void QGCJsonRequest::setTimeout(int ms)
{
    if (_timeout != ms) {
        _timeout = ms;
        emit timeoutChanged(ms);
    }
}

// ============================================================================
// Header Management
// ============================================================================

void QGCJsonRequest::setHeader(const QByteArray &name, const QByteArray &value)
{
    // Remove existing header with same name
    removeHeader(name);
    _customHeaders.append({name, value});
}

void QGCJsonRequest::removeHeader(const QByteArray &name)
{
    _customHeaders.erase(
        std::remove_if(_customHeaders.begin(), _customHeaders.end(),
                       [&name](const auto &h) { return h.first.toLower() == name.toLower(); }),
        _customHeaders.end());
}

void QGCJsonRequest::clearHeaders()
{
    _customHeaders.clear();
}

// ============================================================================
// Request Methods
// ============================================================================

bool QGCJsonRequest::get(const QString &url)
{
    return _sendRequest(Method::Get, url);
}

bool QGCJsonRequest::get(const QString &url, const QVariantMap &params)
{
    QUrl resolvedUrl = _resolveUrl(url);
    QUrlQuery query(resolvedUrl);

    for (auto it = params.begin(); it != params.end(); ++it) {
        query.addQueryItem(it.key(), it.value().toString());
    }
    resolvedUrl.setQuery(query);

    return _sendRequest(Method::Get, resolvedUrl.toString());
}

bool QGCJsonRequest::post(const QString &url, const QJsonObject &body)
{
    const QByteArray data = body.isEmpty() ? QByteArray() : QJsonDocument(body).toJson(QJsonDocument::Compact);
    return _sendRequest(Method::Post, url, data);
}

bool QGCJsonRequest::post(const QString &url, const QJsonArray &body)
{
    const QByteArray data = QJsonDocument(body).toJson(QJsonDocument::Compact);
    return _sendRequest(Method::Post, url, data);
}

bool QGCJsonRequest::put(const QString &url, const QJsonObject &body)
{
    const QByteArray data = body.isEmpty() ? QByteArray() : QJsonDocument(body).toJson(QJsonDocument::Compact);
    return _sendRequest(Method::Put, url, data);
}

bool QGCJsonRequest::patch(const QString &url, const QJsonObject &body)
{
    const QByteArray data = body.isEmpty() ? QByteArray() : QJsonDocument(body).toJson(QJsonDocument::Compact);
    return _sendRequest(Method::Patch, url, data);
}

bool QGCJsonRequest::deleteResource(const QString &url)
{
    return _sendRequest(Method::Delete, url);
}

void QGCJsonRequest::cancel()
{
    if (_reply != nullptr) {
        qCDebug(QGCJsonRequestLog) << "Cancelling request";
        _reply->abort();
    }
}

// ============================================================================
// Private Slots
// ============================================================================

void QGCJsonRequest::_onFinished()
{
    if (_reply == nullptr) {
        return;
    }

    _setHttpStatus(_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    _responseBody = _reply->readAll();

    const bool httpSuccess = (_reply->error() == QNetworkReply::NoError) &&
                             (_httpStatus >= 200 && _httpStatus < 300);

    qCDebug(QGCJsonRequestLog) << "Request finished - HTTP:" << _httpStatus
                                << "size:" << _responseBody.size()
                                << "success:" << httpSuccess;

    // Try to parse JSON response
    QJsonParseError parseError;
    _responseDoc = QJsonDocument::fromJson(_responseBody, &parseError);

    QString errorMsg;
    bool success = httpSuccess;

    if (!httpSuccess) {
        // HTTP error
        errorMsg = _reply->errorString();
        if (errorMsg.isEmpty()) {
            errorMsg = tr("HTTP %1").arg(_httpStatus);
        }

        // Try to extract error message from JSON response
        if (!_responseDoc.isNull() && _responseDoc.isObject()) {
            const QJsonObject obj = _responseDoc.object();
            if (obj.contains(QStringLiteral("error"))) {
                const QJsonValue errorVal = obj.value(QStringLiteral("error"));
                if (errorVal.isString()) {
                    errorMsg = errorVal.toString();
                } else if (errorVal.isObject()) {
                    errorMsg = errorVal.toObject().value(QStringLiteral("message")).toString(errorMsg);
                }
            } else if (obj.contains(QStringLiteral("message"))) {
                errorMsg = obj.value(QStringLiteral("message")).toString(errorMsg);
            }
        }
    } else if (!_responseBody.isEmpty() && _responseDoc.isNull()) {
        // HTTP success but JSON parse failed
        success = false;
        errorMsg = tr("Invalid JSON response: %1").arg(parseError.errorString());
        qCWarning(QGCJsonRequestLog) << "JSON parse error:" << parseError.errorString();
    }

    _setErrorString(errorMsg);

    emit finished(success, _responseDoc, errorMsg);

    // Emit QML-friendly variant signal
    QVariant responseVariant;
    if (_responseDoc.isObject()) {
        responseVariant = _responseDoc.object().toVariantMap();
    } else if (_responseDoc.isArray()) {
        responseVariant = _responseDoc.array().toVariantList();
    }
    emit finishedVariant(success, responseVariant, errorMsg);

    _cleanup();
    _setRunning(false);
}

void QGCJsonRequest::_onError(QNetworkReply::NetworkError error)
{
    if (error == QNetworkReply::OperationCanceledError) {
        qCDebug(QGCJsonRequestLog) << "Request cancelled";
        _setErrorString(tr("Request cancelled"));
    } else {
        qCWarning(QGCJsonRequestLog) << "Network error:" << error;
    }
}

// ============================================================================
// Private Methods
// ============================================================================

void QGCJsonRequest::_setRunning(bool running)
{
    if (_running != running) {
        _running = running;
        emit runningChanged(running);
    }
}

void QGCJsonRequest::_setErrorString(const QString &error)
{
    if (_errorString != error) {
        _errorString = error;
        emit errorStringChanged(error);
    }
}

void QGCJsonRequest::_setHttpStatus(int status)
{
    if (_httpStatus != status) {
        _httpStatus = status;
        emit httpStatusChanged(status);
    }
}

bool QGCJsonRequest::_sendRequest(Method method, const QString &url, const QByteArray &body)
{
    if (_running) {
        qCWarning(QGCJsonRequestLog) << "Request already in progress";
        return false;
    }

    const QUrl resolvedUrl = _resolveUrl(url);
    if (!resolvedUrl.isValid()) {
        _setErrorString(tr("Invalid URL: %1").arg(url));
        return false;
    }

    QNetworkRequest request(resolvedUrl);
    _applyHeaders(request);

    // Set timeout if supported
    if (_timeout > 0) {
        request.setTransferTimeout(_timeout);
    }

    _responseBody.clear();
    _responseDoc = QJsonDocument();
    _setHttpStatus(0);
    _setErrorString({});
    _setRunning(true);

    const char *methodName = "";
    switch (method) {
    case Method::Get:
        methodName = "GET";
        _reply = _networkManager->get(request);
        break;
    case Method::Post:
        methodName = "POST";
        _reply = _networkManager->post(request, body);
        break;
    case Method::Put:
        methodName = "PUT";
        _reply = _networkManager->put(request, body);
        break;
    case Method::Patch:
        methodName = "PATCH";
        _reply = _networkManager->sendCustomRequest(request, "PATCH", body);
        break;
    case Method::Delete:
        methodName = "DELETE";
        _reply = _networkManager->deleteResource(request);
        break;
    }

    qCDebug(QGCJsonRequestLog) << methodName << resolvedUrl.toString()
                                << "body size:" << body.size();

    connect(_reply, &QNetworkReply::finished, this, &QGCJsonRequest::_onFinished);
    connect(_reply, &QNetworkReply::errorOccurred, this, &QGCJsonRequest::_onError);

    return true;
}

QUrl QGCJsonRequest::_resolveUrl(const QString &url) const
{
    QUrl parsedUrl = QUrl::fromUserInput(url);

    // If URL is relative and we have a base URL, resolve it
    if (parsedUrl.isRelative() && !_baseUrl.isEmpty()) {
        const QUrl base = QUrl::fromUserInput(_baseUrl);
        parsedUrl = base.resolved(parsedUrl);
    }

    return parsedUrl;
}

void QGCJsonRequest::_applyHeaders(QNetworkRequest &request) const
{
    // Set JSON headers
    request.setHeader(QNetworkRequest::ContentTypeHeader, kContentTypeJson);
    request.setRawHeader("Accept", kAcceptJson);

    // Set bearer token if configured
    if (!_bearerToken.isEmpty()) {
        request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(_bearerToken).toUtf8());
    }

    // Apply custom headers
    for (const auto &header : _customHeaders) {
        request.setRawHeader(header.first, header.second);
    }
}

void QGCJsonRequest::_cleanup()
{
    if (_reply != nullptr) {
        _reply->deleteLater();
        _reply = nullptr;
    }
}
