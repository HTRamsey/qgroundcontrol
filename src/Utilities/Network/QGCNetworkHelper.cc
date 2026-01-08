#include "QGCNetworkHelper.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkInformation>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkProxyFactory>
#include <QtNetwork/QSslSocket>

#ifdef QGC_ENABLE_BLUETOOTH
#include <QtBluetooth/QBluetoothLocalDevice>
#endif

// http-parser provides HTTP status codes, methods, and status text
#include <http_parser.h>

QGC_LOGGING_CATEGORY(QGCNetworkHelperLog, "Utilities.QGCNetworkHelper")

namespace QGCNetworkHelper {

// ============================================================================
// HTTP Status Code Helpers
// ============================================================================

HttpStatusClass classifyHttpStatus(int statusCode)
{
    if (statusCode >= 100 && statusCode < 200) {
        return HttpStatusClass::Informational;
    }
    if (statusCode >= 200 && statusCode < 300) {
        return HttpStatusClass::Success;
    }
    if (statusCode >= 300 && statusCode < 400) {
        return HttpStatusClass::Redirection;
    }
    if (statusCode >= 400 && statusCode < 500) {
        return HttpStatusClass::ClientError;
    }
    if (statusCode >= 500 && statusCode < 600) {
        return HttpStatusClass::ServerError;
    }
    return HttpStatusClass::Unknown;
}

QString httpStatusText(int statusCode)
{
    // Use http-parser's status text lookup (covers all standard HTTP status codes)
    const char *text = http_status_str(static_cast<http_status>(statusCode));
    if (text != nullptr && text[0] != '\0') {
        return QString::fromLatin1(text);
    }
    return QStringLiteral("Unknown Status %1").arg(statusCode);
}

// ============================================================================
// HTTP Methods
// ============================================================================

QString httpMethodName(HttpMethod method)
{
    // Use http-parser's method name lookup
    const char *name = http_method_str(static_cast<http_method>(static_cast<int>(method)));
    if (name != nullptr) {
        return QString::fromLatin1(name);
    }
    return QStringLiteral("GET");
}

HttpMethod parseHttpMethod(const QString &methodStr)
{
    const QByteArray upper = methodStr.toUpper().toLatin1();
    const char *str = upper.constData();

    // Check common methods first for performance
    if (qstrcmp(str, "GET") == 0)     return HttpMethod::Get;
    if (qstrcmp(str, "POST") == 0)    return HttpMethod::Post;
    if (qstrcmp(str, "PUT") == 0)     return HttpMethod::Put;
    if (qstrcmp(str, "DELETE") == 0)  return HttpMethod::Delete;
    if (qstrcmp(str, "HEAD") == 0)    return HttpMethod::Head;
    if (qstrcmp(str, "OPTIONS") == 0) return HttpMethod::Options;
    if (qstrcmp(str, "PATCH") == 0)   return HttpMethod::Patch;
    if (qstrcmp(str, "CONNECT") == 0) return HttpMethod::Connect;
    if (qstrcmp(str, "TRACE") == 0)   return HttpMethod::Trace;

    return HttpMethod::Get;  // Default fallback
}

// ============================================================================
// URL Utilities
// ============================================================================

bool isValidUrl(const QUrl &url)
{
    if (!url.isValid()) {
        return false;
    }

    const QString scheme = url.scheme().toLower();
    return scheme == QLatin1String("http") ||
           scheme == QLatin1String("https") ||
           scheme == QLatin1String("file") ||
           scheme == QLatin1String("qrc") ||
           scheme.isEmpty();  // Relative URL
}

bool isHttpUrl(const QUrl &url)
{
    const QString scheme = url.scheme().toLower();
    return scheme == QLatin1String("http") || scheme == QLatin1String("https");
}

bool isHttpsUrl(const QUrl &url)
{
    return url.scheme().toLower() == QLatin1String("https");
}

QUrl normalizeUrl(const QUrl &url)
{
    if (!url.isValid()) {
        return url;
    }

    QUrl normalized = url;

    // Lowercase scheme and host
    normalized.setScheme(normalized.scheme().toLower());
    normalized.setHost(normalized.host().toLower());

    // Remove default ports
    const int port = normalized.port();
    const QString scheme = normalized.scheme();
    if ((scheme == QLatin1String("http") && port == 80) ||
        (scheme == QLatin1String("https") && port == 443) ||
        (scheme == QLatin1String("ftp") && port == 21)) {
        normalized.setPort(-1);
    }

    // Remove trailing slash from path (except for root)
    QString path = normalized.path();
    if (path.length() > 1 && path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
        normalized.setPath(path);
    }

    return normalized;
}

QUrl ensureScheme(const QUrl &url, const QString &defaultScheme)
{
    if (!url.isValid()) {
        return url;
    }

    if (url.scheme().isEmpty()) {
        QUrl withScheme = url;
        withScheme.setScheme(defaultScheme);
        return withScheme;
    }

    return url;
}

QUrl buildUrl(const QString &baseUrl, const QMap<QString, QString> &params)
{
    QUrl url(baseUrl);
    if (!url.isValid()) {
        return url;
    }

    QUrlQuery query;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.addQueryItem(it.key(), it.value());
    }
    url.setQuery(query);

    return url;
}

QUrl buildUrl(const QString &baseUrl, const QList<QPair<QString, QString>> &params)
{
    QUrl url(baseUrl);
    if (!url.isValid()) {
        return url;
    }

    QUrlQuery query;
    for (const auto &[key, value] : params) {
        query.addQueryItem(key, value);
    }
    url.setQuery(query);

    return url;
}

QString urlFileName(const QUrl &url)
{
    const QString path = url.path();
    const int lastSlash = path.lastIndexOf(QLatin1Char('/'));
    if (lastSlash >= 0 && lastSlash < path.length() - 1) {
        return path.mid(lastSlash + 1);
    }
    return path;
}

QUrl urlWithoutQuery(const QUrl &url)
{
    QUrl result = url;
    result.setQuery(QString());
    result.setFragment(QString());
    return result;
}

// ============================================================================
// Request Configuration
// ============================================================================

void configureRequest(QNetworkRequest &request, const RequestConfig &config)
{
    // Timeout
    request.setTransferTimeout(config.timeoutMs);

    // Redirect policy
    if (config.allowRedirects) {
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                             QNetworkRequest::NoLessSafeRedirectPolicy);
    } else {
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                             QNetworkRequest::ManualRedirectPolicy);
    }

    // HTTP/2
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, config.http2Allowed);

    // Caching
    if (config.cacheEnabled) {
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                             QNetworkRequest::PreferCache);
        request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
    } else {
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                             QNetworkRequest::AlwaysNetwork);
        request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
    }

    // Background request
    request.setAttribute(QNetworkRequest::BackgroundRequestAttribute, config.backgroundRequest);

    // Headers
    const QString userAgent = config.userAgent.isEmpty() ? defaultUserAgent() : config.userAgent;
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent);

    if (!config.accept.isEmpty()) {
        request.setRawHeader("Accept", config.accept.toUtf8());
    }

    if (!config.acceptEncoding.isEmpty()) {
        request.setRawHeader("Accept-Encoding", config.acceptEncoding.toUtf8());
    }

    if (!config.contentType.isEmpty()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, config.contentType);
    }

    request.setRawHeader("Connection", "keep-alive");
}

QNetworkRequest createRequest(const QUrl &url, const RequestConfig &config)
{
    QNetworkRequest request(url);
    configureRequest(request, config);
    return request;
}

void setStandardHeaders(QNetworkRequest &request, const QString &userAgent)
{
    const QString ua = userAgent.isEmpty() ? defaultUserAgent() : userAgent;
    request.setHeader(QNetworkRequest::UserAgentHeader, ua);
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    request.setRawHeader("Connection", "keep-alive");
}

void setJsonHeaders(QNetworkRequest &request)
{
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
}

void setFormHeaders(QNetworkRequest &request)
{
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/x-www-form-urlencoded"));
}

QString defaultUserAgent()
{
    static QString userAgent;
    if (userAgent.isEmpty()) {
        userAgent = QStringLiteral("%1/%2 (Qt %3)")
            .arg(QCoreApplication::applicationName())
            .arg(QCoreApplication::applicationVersion())
            .arg(QString::fromLatin1(qVersion()));
    }
    return userAgent;
}

// ============================================================================
// Network Reply Helpers
// ============================================================================

int httpStatusCode(const QNetworkReply *reply)
{
    if (!reply) {
        return -1;
    }
    const QVariant statusAttr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    return statusAttr.isValid() ? statusAttr.toInt() : -1;
}

QUrl redirectUrl(const QNetworkReply *reply)
{
    if (!reply) {
        return {};
    }

    const QVariant redirectAttr = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (redirectAttr.isNull()) {
        return {};
    }

    const QUrl redirectTarget = redirectAttr.toUrl();
    return reply->url().resolved(redirectTarget);
}

QString errorMessage(const QNetworkReply *reply)
{
    if (!reply) {
        return QStringLiteral("No reply");
    }

    // Check for network error first
    if (reply->error() != QNetworkReply::NoError) {
        return reply->errorString();
    }

    // Check HTTP status code
    const int status = httpStatusCode(reply);
    if (status >= 400) {
        return QStringLiteral("HTTP %1: %2").arg(status).arg(httpStatusText(status));
    }

    return {};
}

bool isSuccess(const QNetworkReply *reply)
{
    if (!reply) {
        return false;
    }

    if (reply->error() != QNetworkReply::NoError) {
        return false;
    }

    const int status = httpStatusCode(reply);
    return status == -1 || isHttpSuccess(status);  // -1 means non-HTTP (e.g., file://)
}

bool isRedirect(const QNetworkReply *reply)
{
    if (!reply) {
        return false;
    }

    const int status = httpStatusCode(reply);
    return isHttpRedirect(status) || !redirectUrl(reply).isEmpty();
}

QString contentType(const QNetworkReply *reply)
{
    if (!reply) {
        return {};
    }
    return reply->header(QNetworkRequest::ContentTypeHeader).toString();
}

qint64 contentLength(const QNetworkReply *reply)
{
    if (!reply) {
        return -1;
    }
    const QVariant lenAttr = reply->header(QNetworkRequest::ContentLengthHeader);
    return lenAttr.isValid() ? lenAttr.toLongLong() : -1;
}

bool isJsonResponse(const QNetworkReply *reply)
{
    const QString type = contentType(reply);
    return type.contains(QLatin1String("application/json"), Qt::CaseInsensitive) ||
           type.contains(QLatin1String("+json"), Qt::CaseInsensitive);
}

// ============================================================================
// Network Availability
// ============================================================================

bool isNetworkAvailable()
{
    if (!QNetworkInformation::loadDefaultBackend()) {
        qCDebug(QGCNetworkHelperLog) << "Failed to load network information backend";
        return true;  // Assume available if we can't check
    }

    const QNetworkInformation *netInfo = QNetworkInformation::instance();
    if (netInfo == nullptr) {
        return true;
    }

    return netInfo->reachability() != QNetworkInformation::Reachability::Disconnected;
}

bool isInternetAvailable()
{
    if (QNetworkInformation::availableBackends().isEmpty()) {
        return false;
    }

    if (!QNetworkInformation::loadDefaultBackend()) {
        return false;
    }

    if (!QNetworkInformation::loadBackendByFeatures(QNetworkInformation::Feature::Reachability)) {
        return false;
    }

    const QNetworkInformation *netInfo = QNetworkInformation::instance();
    if (netInfo == nullptr) {
        return false;
    }

    return netInfo->reachability() == QNetworkInformation::Reachability::Online;
}

bool isNetworkEthernet()
{
    if (QNetworkInformation::availableBackends().isEmpty()) {
        return false;
    }

    if (!QNetworkInformation::loadDefaultBackend()) {
        return false;
    }

    const QNetworkInformation *netInfo = QNetworkInformation::instance();
    if (netInfo == nullptr) {
        return false;
    }

    return netInfo->transportMedium() == QNetworkInformation::TransportMedium::Ethernet;
}

bool isBluetoothAvailable()
{
#ifdef QGC_ENABLE_BLUETOOTH
    const QList<QBluetoothHostInfo> devices = QBluetoothLocalDevice::allDevices();
    return !devices.isEmpty();
#else
    return false;
#endif
}

ConnectionType connectionType()
{
    if (!QNetworkInformation::loadDefaultBackend()) {
        return ConnectionType::Unknown;
    }

    const QNetworkInformation *netInfo = QNetworkInformation::instance();
    if (!netInfo) {
        return ConnectionType::Unknown;
    }

    if (netInfo->reachability() == QNetworkInformation::Reachability::Disconnected) {
        return ConnectionType::None;
    }

    switch (netInfo->transportMedium()) {
    case QNetworkInformation::TransportMedium::Ethernet:
        return ConnectionType::Ethernet;
    case QNetworkInformation::TransportMedium::WiFi:
        return ConnectionType::WiFi;
    case QNetworkInformation::TransportMedium::Cellular:
        return ConnectionType::Cellular;
    case QNetworkInformation::TransportMedium::Bluetooth:
        return ConnectionType::Bluetooth;
    default:
        return ConnectionType::Unknown;
    }
}

QString connectionTypeName(ConnectionType type)
{
    switch (type) {
    case ConnectionType::None:      return QStringLiteral("None");
    case ConnectionType::Ethernet:  return QStringLiteral("Ethernet");
    case ConnectionType::WiFi:      return QStringLiteral("WiFi");
    case ConnectionType::Cellular:  return QStringLiteral("Cellular");
    case ConnectionType::Bluetooth: return QStringLiteral("Bluetooth");
    case ConnectionType::Unknown:
    default:                        return QStringLiteral("Unknown");
    }
}

// ============================================================================
// SSL/TLS Helpers
// ============================================================================

void ignoreSslErrors(QNetworkReply *reply)
{
    if (!reply) {
        return;
    }

    QObject::connect(reply, &QNetworkReply::sslErrors, reply, [reply](const QList<QSslError> &errors) {
        qCWarning(QGCNetworkHelperLog) << "Ignoring SSL errors for" << reply->url();
        for (const QSslError &error : errors) {
            qCDebug(QGCNetworkHelperLog) << "  -" << error.errorString();
        }
        reply->ignoreSslErrors();
    });
}

void ignoreSslErrorsIfNeeded(QNetworkReply *reply)
{
    if (!reply) {
        return;
    }

    // Check for OpenSSL version mismatch: Qt built with 1.x but running with 3.x
    const bool sslLibraryBuildIs1x = ((QSslSocket::sslLibraryBuildVersionNumber() & 0xf0000000) == 0x10000000);
    const bool sslLibraryIs3x = ((QSslSocket::sslLibraryVersionNumber() & 0xf0000000) == 0x30000000);
    if (sslLibraryBuildIs1x && sslLibraryIs3x) {
        qCWarning(QGCNetworkHelperLog) << "Ignoring SSL certificate errors due to OpenSSL version mismatch";
        QList<QSslError> errorsThatCanBeIgnored;
        errorsThatCanBeIgnored << QSslError(QSslError::NoPeerCertificate);
        reply->ignoreSslErrors(errorsThatCanBeIgnored);
    }
}

bool isSslAvailable()
{
    return QSslSocket::supportsSsl();
}

QString sslVersion()
{
    return QSslSocket::sslLibraryVersionString();
}

// ============================================================================
// Network Access Manager Helpers
// ============================================================================

void initializeProxySupport()
{
    QNetworkProxyFactory::setUseSystemConfiguration(true);
}

QNetworkAccessManager *createNetworkManager(QObject *parent)
{
    auto *manager = new QNetworkAccessManager(parent);
    configureProxy(manager);
    return manager;
}

void configureProxy(QNetworkAccessManager *manager)
{
    if (!manager) {
        return;
    }

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
    QNetworkProxy proxy = manager->proxy();
    proxy.setType(QNetworkProxy::DefaultProxy);
    manager->setProxy(proxy);
#endif
}

} // namespace QGCNetworkHelper
