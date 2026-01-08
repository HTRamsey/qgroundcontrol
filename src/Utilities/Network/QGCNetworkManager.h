#pragma once

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>
#include <QtNetwork/QNetworkAccessManager>

Q_DECLARE_LOGGING_CATEGORY(QGCNetworkManagerLog)

/// Singleton providing a shared QNetworkAccessManager instance.
/// Benefits:
/// - Connection pooling and HTTP/2 multiplexing
/// - Centralized SSL/proxy configuration
/// - Consistent timeout and redirect policies
/// - Optional request metrics
class QGCNetworkManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int activeRequestCount READ activeRequestCount NOTIFY activeRequestCountChanged)
    Q_PROPERTY(bool networkAccessible READ isNetworkAccessible NOTIFY networkAccessibleChanged)

public:
    /// Get the singleton instance
    static QGCNetworkManager *instance();

    /// Get the shared QNetworkAccessManager
    /// @note All network requests should use this manager for optimal connection reuse
    QNetworkAccessManager *manager() const { return _manager; }

    /// Convenience: Create a GET request using the shared manager
    /// @param url The URL to request
    /// @return The QNetworkReply (caller takes ownership)
    QNetworkReply *get(const QNetworkRequest &request);

    /// Convenience: Create a POST request using the shared manager
    QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data);
    QNetworkReply *post(const QNetworkRequest &request, QHttpMultiPart *multiPart);

    /// Convenience: Create a PUT request using the shared manager
    QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data);

    /// Convenience: Create a DELETE request using the shared manager
    QNetworkReply *deleteResource(const QNetworkRequest &request);

    /// Convenience: Create a HEAD request using the shared manager
    QNetworkReply *head(const QNetworkRequest &request);

    /// Number of currently active requests
    int activeRequestCount() const { return _activeRequestCount; }

    /// Whether network is currently accessible
    bool isNetworkAccessible() const;

    /// Total requests made since startup
    qint64 totalRequestCount() const { return _totalRequestCount; }

    /// Total bytes received since startup
    qint64 totalBytesReceived() const { return _totalBytesReceived; }

    /// Total bytes sent since startup
    qint64 totalBytesSent() const { return _totalBytesSent; }

    /// Clear accumulated statistics
    Q_INVOKABLE void clearStatistics();

    /// Set default request timeout (milliseconds)
    void setDefaultTimeout(int timeoutMs) { _defaultTimeoutMs = timeoutMs; }
    int defaultTimeout() const { return _defaultTimeoutMs; }

    /// Configure whether to automatically follow redirects (default: true)
    void setAutoRedirect(bool enabled) { _autoRedirect = enabled; }
    bool autoRedirect() const { return _autoRedirect; }

    /// Set maximum redirects to follow (default: 5)
    void setMaxRedirects(int max) { _maxRedirects = max; }
    int maxRedirects() const { return _maxRedirects; }

signals:
    void activeRequestCountChanged(int count);
    void networkAccessibleChanged(bool accessible);
    void requestStarted(const QUrl &url);
    void requestFinished(const QUrl &url, int httpStatus, qint64 bytesReceived);

private:
    explicit QGCNetworkManager(QObject *parent = nullptr);
    ~QGCNetworkManager() override;

    void _configureRequest(QNetworkRequest &request);
    void _trackRequest(QNetworkReply *reply);

    static QGCNetworkManager *_instance;

    QNetworkAccessManager *_manager = nullptr;
    int _activeRequestCount = 0;
    qint64 _totalRequestCount = 0;
    qint64 _totalBytesReceived = 0;
    qint64 _totalBytesSent = 0;
    int _defaultTimeoutMs = 30000;
    bool _autoRedirect = true;
    int _maxRedirects = 5;
};
