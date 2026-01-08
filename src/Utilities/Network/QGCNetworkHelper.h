#pragma once

#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

class QNetworkAccessManager;

/// Network utility functions for HTTP requests, URL handling, and connectivity
/// All functions are stateless and thread-safe
namespace QGCNetworkHelper {

// ============================================================================
// Constants
// ============================================================================

/// Default request timeout in milliseconds
constexpr int kDefaultTimeoutMs = 30000;

/// Default connection timeout for initial connect
constexpr int kDefaultConnectTimeoutMs = 10000;

/// Maximum number of redirects to follow
constexpr int kMaxRedirects = 10;

// ============================================================================
// HTTP Status Code Helpers
// ============================================================================

/// HTTP status code ranges
enum class HttpStatusClass {
    Informational,  ///< 1xx - Informational
    Success,        ///< 2xx - Success
    Redirection,    ///< 3xx - Redirection
    ClientError,    ///< 4xx - Client errors
    ServerError,    ///< 5xx - Server errors
    Unknown         ///< Not a valid HTTP status
};

/// Classify an HTTP status code
HttpStatusClass classifyHttpStatus(int statusCode);

/// Check if HTTP status indicates success (2xx)
inline bool isHttpSuccess(int statusCode) {
    return statusCode >= 200 && statusCode < 300;
}

/// Check if HTTP status indicates redirect (3xx)
inline bool isHttpRedirect(int statusCode) {
    return statusCode >= 300 && statusCode < 400;
}

/// Check if HTTP status indicates client error (4xx)
inline bool isHttpClientError(int statusCode) {
    return statusCode >= 400 && statusCode < 500;
}

/// Check if HTTP status indicates server error (5xx)
inline bool isHttpServerError(int statusCode) {
    return statusCode >= 500 && statusCode < 600;
}

/// Get human-readable description for HTTP status code
/// Uses http-parser's comprehensive status code database
QString httpStatusText(int statusCode);

// ============================================================================
// HTTP Methods
// ============================================================================

/// HTTP request methods (matches http-parser's http_method enum)
enum class HttpMethod {
    Delete = 0,
    Get = 1,
    Head = 2,
    Post = 3,
    Put = 4,
    Connect = 5,
    Options = 6,
    Trace = 7,
    Patch = 28,
};

/// Get string name for an HTTP method (e.g., "GET", "POST")
QString httpMethodName(HttpMethod method);

/// Parse HTTP method from string (case-insensitive)
/// Returns HttpMethod::Get if not recognized
HttpMethod parseHttpMethod(const QString &methodStr);

// ============================================================================
// URL Utilities
// ============================================================================

/// Check if URL is valid and has supported scheme
/// Supported: http, https, file, qrc
bool isValidUrl(const QUrl &url);

/// Check if URL uses HTTP or HTTPS scheme
bool isHttpUrl(const QUrl &url);

/// Check if URL uses secure HTTPS scheme
bool isHttpsUrl(const QUrl &url);

/// Normalize URL (lowercase scheme/host, remove default ports, trailing slashes)
QUrl normalizeUrl(const QUrl &url);

/// Ensure URL has scheme, defaulting to https:// if missing
QUrl ensureScheme(const QUrl &url, const QString &defaultScheme = QStringLiteral("https"));

/// Build URL with query parameters from a map
QUrl buildUrl(const QString &baseUrl, const QMap<QString, QString> &params);

/// Build URL with query parameters from a list of pairs
QUrl buildUrl(const QString &baseUrl, const QList<QPair<QString, QString>> &params);

/// Extract filename from URL path (last path segment)
QString urlFileName(const QUrl &url);

/// Get URL without query string and fragment
QUrl urlWithoutQuery(const QUrl &url);

// ============================================================================
// Request Configuration
// ============================================================================

/// Common request configuration options
struct RequestConfig {
    int timeoutMs = kDefaultTimeoutMs;
    bool allowRedirects = true;
    bool http2Allowed = true;
    bool cacheEnabled = true;
    bool backgroundRequest = false;
    QString userAgent;
    QString accept = QStringLiteral("*/*");
    QString acceptEncoding;
    QString contentType;
};

/// Configure a QNetworkRequest with standard settings
/// @param request Request to configure
/// @param config Configuration options (uses defaults if not specified)
void configureRequest(QNetworkRequest &request, const RequestConfig &config = {});

/// Create a pre-configured QNetworkRequest
/// @param url The URL for the request
/// @param config Configuration options (uses defaults if not specified)
/// @return Configured QNetworkRequest ready for use
QNetworkRequest createRequest(const QUrl &url, const RequestConfig &config = {});

/// Set standard browser-like headers on a request
void setStandardHeaders(QNetworkRequest &request, const QString &userAgent = {});

/// Set JSON content headers (Accept and Content-Type)
void setJsonHeaders(QNetworkRequest &request);

/// Set form data content headers
void setFormHeaders(QNetworkRequest &request);

/// Get the default User-Agent string for QGC
QString defaultUserAgent();

// ============================================================================
// Network Reply Helpers
// ============================================================================

/// Get HTTP status code from a network reply
/// Returns -1 if not an HTTP response
int httpStatusCode(const QNetworkReply *reply);

/// Get redirect URL from a reply, if any
/// Returns empty URL if no redirect
QUrl redirectUrl(const QNetworkReply *reply);

/// Get error message from a network reply
/// Combines network error and HTTP status into readable message
QString errorMessage(const QNetworkReply *reply);

/// Check if reply indicates success (no error and HTTP 2xx)
bool isSuccess(const QNetworkReply *reply);

/// Check if reply indicates a redirect
bool isRedirect(const QNetworkReply *reply);

/// Get Content-Type header from reply
QString contentType(const QNetworkReply *reply);

/// Get Content-Length header from reply (-1 if not present)
qint64 contentLength(const QNetworkReply *reply);

/// Check if response is JSON based on Content-Type
bool isJsonResponse(const QNetworkReply *reply);

// ============================================================================
// Network Availability
// ============================================================================

/// Check if network is available (not disconnected)
bool isNetworkAvailable();

/// Check if internet is reachable (online state, stricter than isNetworkAvailable)
bool isInternetAvailable();

/// Check if current network connection is Ethernet
bool isNetworkEthernet();

/// Check if Bluetooth is available on this device
bool isBluetoothAvailable();

/// Network connection types
enum class ConnectionType {
    None,       ///< No network connection
    Unknown,    ///< Connection type unknown
    Ethernet,   ///< Wired ethernet
    WiFi,       ///< Wireless LAN
    Cellular,   ///< Mobile/cellular data
    Bluetooth,  ///< Bluetooth connection
};

/// Get current network connection type
ConnectionType connectionType();

/// Get human-readable name for connection type
QString connectionTypeName(ConnectionType type);

// ============================================================================
// SSL/TLS Helpers
// ============================================================================

/// Configure SSL to ignore certificate errors (use with caution!)
/// This should only be used for development/testing or known self-signed certs
void ignoreSslErrors(QNetworkReply *reply);

/// Ignore SSL errors if there's an OpenSSL version mismatch
/// Qt may be built with OpenSSL 1.x but running with OpenSSL 3.x, causing certificate issues
void ignoreSslErrorsIfNeeded(QNetworkReply *reply);

/// Check if SSL is available
bool isSslAvailable();

/// Get SSL library version string
QString sslVersion();

// ============================================================================
// Network Access Manager Helpers
// ============================================================================

/// Initialize network proxy support (call once at application startup)
/// Enables system proxy configuration for all network requests
void initializeProxySupport();

/// Create a network access manager with recommended settings
/// Caller takes ownership of the returned pointer
QNetworkAccessManager *createNetworkManager(QObject *parent = nullptr);

/// Set up default proxy configuration on a network manager
void configureProxy(QNetworkAccessManager *manager);

} // namespace QGCNetworkHelper
