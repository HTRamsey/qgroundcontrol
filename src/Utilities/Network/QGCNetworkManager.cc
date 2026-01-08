#include "QGCNetworkManager.h"

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QNetworkInformation>

Q_LOGGING_CATEGORY(QGCNetworkManagerLog, "Utilities.QGCNetworkManager")

QGCNetworkManager *QGCNetworkManager::_instance = nullptr;

QGCNetworkManager::QGCNetworkManager(QObject *parent)
    : QObject(parent)
    , _manager(new QNetworkAccessManager(this))
{
    // Enable HTTP/2 for better multiplexing
    _manager->setAutoDeleteReplies(false);

    // Monitor network accessibility if available
    if (QNetworkInformation::loadDefaultBackend()) {
        QNetworkInformation *netInfo = QNetworkInformation::instance();
        if (netInfo != nullptr) {
            connect(netInfo, &QNetworkInformation::reachabilityChanged, this, [this]() {
                emit networkAccessibleChanged(isNetworkAccessible());
            });
        }
    }

    qCDebug(QGCNetworkManagerLog) << "Network manager initialized";
}

QGCNetworkManager::~QGCNetworkManager()
{
    qCDebug(QGCNetworkManagerLog) << "Network manager destroyed - total requests:" << _totalRequestCount
                                   << "bytes received:" << _totalBytesReceived
                                   << "bytes sent:" << _totalBytesSent;
}

QGCNetworkManager *QGCNetworkManager::instance()
{
    if (_instance == nullptr) {
        _instance = new QGCNetworkManager();
    }
    return _instance;
}

bool QGCNetworkManager::isNetworkAccessible() const
{
    QNetworkInformation *netInfo = QNetworkInformation::instance();
    if (netInfo != nullptr) {
        return netInfo->reachability() == QNetworkInformation::Reachability::Online ||
               netInfo->reachability() == QNetworkInformation::Reachability::Local;
    }
    return true;  // Assume accessible if we can't determine
}

void QGCNetworkManager::clearStatistics()
{
    _totalRequestCount = 0;
    _totalBytesReceived = 0;
    _totalBytesSent = 0;
    qCDebug(QGCNetworkManagerLog) << "Statistics cleared";
}

void QGCNetworkManager::_configureRequest(QNetworkRequest &request)
{
    // Set timeout if not already set
    if (request.transferTimeout() == 0) {
        request.setTransferTimeout(_defaultTimeoutMs);
    }

    // Configure redirect policy
    if (_autoRedirect) {
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                           QNetworkRequest::NoLessSafeRedirectPolicy);
        request.setMaximumRedirectsAllowed(_maxRedirects);
    } else {
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                           QNetworkRequest::ManualRedirectPolicy);
    }

    // Enable HTTP/2
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
}

void QGCNetworkManager::_trackRequest(QNetworkReply *reply)
{
    if (reply == nullptr) {
        return;
    }

    _activeRequestCount++;
    _totalRequestCount++;
    emit activeRequestCountChanged(_activeRequestCount);
    emit requestStarted(reply->url());

    qCDebug(QGCNetworkManagerLog) << "Request started:" << reply->url().toString()
                                   << "active:" << _activeRequestCount;

    // Track completion
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        _activeRequestCount--;
        emit activeRequestCountChanged(_activeRequestCount);

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const qint64 bytesReceived = reply->bytesAvailable();
        _totalBytesReceived += bytesReceived;

        emit requestFinished(reply->url(), status, bytesReceived);

        qCDebug(QGCNetworkManagerLog) << "Request finished:" << reply->url().toString()
                                       << "status:" << status
                                       << "bytes:" << bytesReceived
                                       << "active:" << _activeRequestCount;
    });

    // Track upload progress for sent bytes
    connect(reply, &QNetworkReply::uploadProgress, this, [this](qint64 bytesSent, qint64 /*bytesTotal*/) {
        static qint64 lastSent = 0;
        if (bytesSent > lastSent) {
            _totalBytesSent += (bytesSent - lastSent);
            lastSent = bytesSent;
        }
    });
}

QNetworkReply *QGCNetworkManager::get(const QNetworkRequest &request)
{
    QNetworkRequest req = request;
    _configureRequest(req);

    QNetworkReply *reply = _manager->get(req);
    _trackRequest(reply);
    return reply;
}

QNetworkReply *QGCNetworkManager::post(const QNetworkRequest &request, const QByteArray &data)
{
    QNetworkRequest req = request;
    _configureRequest(req);

    _totalBytesSent += data.size();

    QNetworkReply *reply = _manager->post(req, data);
    _trackRequest(reply);
    return reply;
}

QNetworkReply *QGCNetworkManager::post(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    QNetworkRequest req = request;
    _configureRequest(req);

    QNetworkReply *reply = _manager->post(req, multiPart);
    _trackRequest(reply);
    return reply;
}

QNetworkReply *QGCNetworkManager::put(const QNetworkRequest &request, const QByteArray &data)
{
    QNetworkRequest req = request;
    _configureRequest(req);

    _totalBytesSent += data.size();

    QNetworkReply *reply = _manager->put(req, data);
    _trackRequest(reply);
    return reply;
}

QNetworkReply *QGCNetworkManager::deleteResource(const QNetworkRequest &request)
{
    QNetworkRequest req = request;
    _configureRequest(req);

    QNetworkReply *reply = _manager->deleteResource(req);
    _trackRequest(reply);
    return reply;
}

QNetworkReply *QGCNetworkManager::head(const QNetworkRequest &request)
{
    QNetworkRequest req = request;
    _configureRequest(req);

    QNetworkReply *reply = _manager->head(req);
    _trackRequest(reply);
    return reply;
}
