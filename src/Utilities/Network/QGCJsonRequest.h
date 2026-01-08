#pragma once

/// @file QGCJsonRequest.h
/// @brief Simple JSON API request client with convenience methods

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

class QNetworkAccessManager;

Q_DECLARE_LOGGING_CATEGORY(QGCJsonRequestLog)

/// Simple JSON API request client
///
/// Features:
/// - GET, POST, PUT, PATCH, DELETE methods
/// - Automatic JSON serialization/deserialization
/// - Bearer token authentication
/// - Progress tracking
/// - QML-compatible properties
/// - Integrates with QGCNetworkRetry for retry support
///
/// Example C++ usage (GET):
/// @code
/// auto *api = new QGCJsonRequest(this);
/// api->setBearerToken("my-auth-token");
/// connect(api, &QGCJsonRequest::finished, this,
///         [](bool success, const QJsonDocument &response, const QString &error) {
///     if (success) {
///         qDebug() << "User:" << response.object()["name"].toString();
///     }
/// });
/// api->get("https://api.example.com/users/123");
/// @endcode
///
/// Example C++ usage (POST):
/// @code
/// auto *api = new QGCJsonRequest(this);
/// QJsonObject payload{{"name", "John"}, {"email", "john@example.com"}};
/// api->post("https://api.example.com/users", payload);
/// @endcode
///
/// Example QML usage:
/// @code
/// QGCJsonRequest {
///     id: api
///     bearerToken: authManager.token
///     onFinished: (success, response, error) => {
///         if (success) {
///             userModel.fromJson(response)
///         } else {
///             errorDialog.show(error)
///         }
///     }
/// }
/// Button {
///     text: api.running ? "Loading..." : "Refresh"
///     enabled: !api.running
///     onClicked: api.get("https://api.example.com/data")
/// }
/// @endcode
class QGCJsonRequest : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QGCJsonRequest)

    /// Whether a request is in progress
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged FINAL)

    /// Error message from last failed request
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)

    /// HTTP status code from last response
    Q_PROPERTY(int httpStatus READ httpStatus NOTIFY httpStatusChanged FINAL)

    /// Bearer token for Authorization header
    Q_PROPERTY(QString bearerToken READ bearerToken WRITE setBearerToken NOTIFY bearerTokenChanged FINAL)

    /// Base URL for relative paths
    Q_PROPERTY(QString baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged FINAL)

    /// Default timeout in milliseconds (0 = no timeout)
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout NOTIFY timeoutChanged FINAL)

public:
    explicit QGCJsonRequest(QObject *parent = nullptr);
    ~QGCJsonRequest() override;

    // Property getters
    bool isRunning() const { return _running; }
    QString errorString() const { return _errorString; }
    int httpStatus() const { return _httpStatus; }
    QString bearerToken() const { return _bearerToken; }
    QString baseUrl() const { return _baseUrl; }
    int timeout() const { return _timeout; }

    // Property setters
    void setBearerToken(const QString &token);
    void setBaseUrl(const QString &url);
    void setTimeout(int ms);

    /// Get the last response as QJsonDocument
    QJsonDocument responseDocument() const { return _responseDoc; }

    /// Get the last response as QJsonObject (convenience)
    QJsonObject responseObject() const { return _responseDoc.object(); }

    /// Get the last response as QJsonArray (convenience)
    QJsonArray responseArray() const { return _responseDoc.array(); }

    /// Get the raw response body
    QByteArray responseBody() const { return _responseBody; }

    /// Set a custom header for all requests
    void setHeader(const QByteArray &name, const QByteArray &value);

    /// Remove a custom header
    void removeHeader(const QByteArray &name);

    /// Clear all custom headers
    void clearHeaders();

public slots:
    /// Send GET request
    /// @param url URL or path (prepended with baseUrl if relative)
    /// @return true if request started
    bool get(const QString &url);

    /// Send GET request with query parameters
    /// @param url URL or path
    /// @param params Query parameters as map
    /// @return true if request started
    bool get(const QString &url, const QVariantMap &params);

    /// Send POST request with JSON body
    /// @param url URL or path
    /// @param body JSON object to send
    /// @return true if request started
    bool post(const QString &url, const QJsonObject &body = QJsonObject());

    /// Send POST request with JSON array body
    /// @param url URL or path
    /// @param body JSON array to send
    /// @return true if request started
    bool post(const QString &url, const QJsonArray &body);

    /// Send PUT request with JSON body
    /// @param url URL or path
    /// @param body JSON object to send
    /// @return true if request started
    bool put(const QString &url, const QJsonObject &body = QJsonObject());

    /// Send PATCH request with JSON body
    /// @param url URL or path
    /// @param body JSON object to send
    /// @return true if request started
    bool patch(const QString &url, const QJsonObject &body = QJsonObject());

    /// Send DELETE request
    /// @param url URL or path
    /// @return true if request started
    bool deleteResource(const QString &url);

    /// Cancel current request
    void cancel();

signals:
    void runningChanged(bool running);
    void errorStringChanged(const QString &errorString);
    void httpStatusChanged(int httpStatus);
    void bearerTokenChanged(const QString &token);
    void baseUrlChanged(const QString &url);
    void timeoutChanged(int timeout);

    /// Emitted when request completes
    /// @param success true if request succeeded (HTTP 2xx and valid JSON)
    /// @param response Parsed JSON response (null document on error)
    /// @param errorMessage Error message (empty on success)
    void finished(bool success, const QJsonDocument &response, const QString &errorMessage);

    /// Emitted when request completes (QML-friendly overload with QVariant)
    /// @param success true if request succeeded
    /// @param response Response as QVariant (QVariantMap or QVariantList)
    /// @param errorMessage Error message (empty on success)
    void finishedVariant(bool success, const QVariant &response, const QString &errorMessage);

private slots:
    void _onFinished();
    void _onError(QNetworkReply::NetworkError error);

private:
    enum class Method { Get, Post, Put, Patch, Delete };

    void _setRunning(bool running);
    void _setErrorString(const QString &error);
    void _setHttpStatus(int status);
    bool _sendRequest(Method method, const QString &url, const QByteArray &body = {});
    QUrl _resolveUrl(const QString &url) const;
    void _applyHeaders(QNetworkRequest &request) const;
    void _cleanup();

    QNetworkAccessManager *_networkManager = nullptr;
    QNetworkReply *_reply = nullptr;

    QString _bearerToken;
    QString _baseUrl;
    QString _errorString;
    QByteArray _responseBody;
    QJsonDocument _responseDoc;
    QList<QPair<QByteArray, QByteArray>> _customHeaders;

    int _httpStatus = 0;
    int _timeout = 30000;  // 30 seconds default
    bool _running = false;
};
