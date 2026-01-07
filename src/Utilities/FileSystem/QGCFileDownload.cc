#include "QGCFileDownload.h"
#include "QGCCompression.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtNetwork/QNetworkProxy>

QGC_LOGGING_CATEGORY(QGCFileDownloadLog, "Utilities.QGCFileDownload");

QGCFileDownload::QGCFileDownload(QObject *parent)
    : QObject(parent)
    , _networkManager(new QNetworkAccessManager(this))
{
    qCDebug(QGCFileDownloadLog) << this;
}

QGCFileDownload::~QGCFileDownload()
{
    qCDebug(QGCFileDownloadLog) << this;
}

void QGCFileDownload::setCache(QAbstractNetworkCache *cache)
{
    _networkManager->setCache(cache);
}

void QGCFileDownload::setIgnoreSSLErrorsIfNeeded(QNetworkReply &networkReply)
{
    const bool sslLibraryBuildIs1x = ((QSslSocket::sslLibraryBuildVersionNumber() & 0xf0000000) == 0x10000000);
    const bool sslLibraryIs3x = ((QSslSocket::sslLibraryVersionNumber() & 0xf0000000) == 0x30000000);
    if (sslLibraryBuildIs1x && sslLibraryIs3x) {
        qCWarning(QGCFileDownloadLog) << "Ignoring ssl certificates due to OpenSSL version mismatch";
        QList<QSslError> errorsThatCanBeIgnored;
        errorsThatCanBeIgnored << QSslError(QSslError::NoPeerCertificate);
        networkReply.ignoreSslErrors(errorsThatCanBeIgnored);
    }
}

bool QGCFileDownload::download(const QString &remoteFile,
                               const QList<QPair<QNetworkRequest::Attribute,QVariant>> &requestAttributes,
                               bool autoDecompress)
{
    _requestAttributes = requestAttributes;
    _originalRemoteFile = remoteFile;
    _autoDecompress = autoDecompress;

    return _downloadInternal(remoteFile, false);
}

bool QGCFileDownload::_downloadInternal(const QString &remoteFile, bool redirect)
{
    Q_UNUSED(redirect);

    if (remoteFile.isEmpty()) {
        qCWarning(QGCFileDownloadLog) << "downloadFile empty";
        return false;
    }

    QUrl remoteUrl;
    if (remoteFile.startsWith("http:") || remoteFile.startsWith("https:")) {
        remoteUrl.setUrl(remoteFile);
    } else {
        // QNetworkRequest is used for local files too to use the same code path
        remoteUrl = QUrl::fromLocalFile(remoteFile);
    }

    if (!remoteUrl.isValid()) {
        qCWarning(QGCFileDownloadLog) << "Remote URL is invalid" << remoteFile;
        return false;
    }

    QNetworkRequest networkRequest(remoteUrl);
    for (const QPair<QNetworkRequest::Attribute,QVariant> &attribute : _requestAttributes) {
        networkRequest.setAttribute(attribute.first, attribute.second);
    }

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
    QNetworkProxy tProxy = _networkManager->proxy();
    tProxy.setType(QNetworkProxy::DefaultProxy);
    _networkManager->setProxy(tProxy);
#endif

    QNetworkReply *const networkReply = _networkManager->get(networkRequest);
    if (!networkReply) {
        qCWarning(QGCFileDownloadLog) << "QNetworkAccessManager::get failed";
        return false;
    }

    setIgnoreSSLErrorsIfNeeded(*networkReply);

    (void) connect(networkReply, &QNetworkReply::downloadProgress, this, &QGCFileDownload::downloadProgress);
    (void) connect(networkReply, &QNetworkReply::finished, this, &QGCFileDownload::_downloadFinished);
    (void) connect(networkReply, &QNetworkReply::errorOccurred, this, &QGCFileDownload::_downloadError);

    return true;
}

void QGCFileDownload::_downloadFinished()
{
    QNetworkReply *const reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (!reply) {
        return;
    }
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    if (!reply->isOpen()) {
        return;
    }

    if (!reply->url().isLocalFile()) {
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if ((statusCode < HTTP_Response::SUCCESS_OK) || (statusCode >= HTTP_Response::REDIRECTION_MULTIPLE_CHOICES)) {
            return;
        }
    }

    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirectionTarget.isNull()) {
        const QUrl redirectUrl = reply->url().resolved(redirectionTarget.toUrl());
        (void) _downloadInternal(redirectUrl.toString(), true);
        return;
    }

    // Split out filename from path
    QString remoteFileName = QFileInfo(reply->url().toString()).fileName();
    if (remoteFileName.isEmpty()) {
        qCWarning(QGCFileDownloadLog) << "Unabled to parse filename from remote url" << reply->url().toString();
        remoteFileName = "DownloadedFile";
    }

    // Strip out http parameters from remote filename
    const int parameterIndex = remoteFileName.indexOf("?");
    if (parameterIndex != -1) {
        remoteFileName = remoteFileName.left(parameterIndex);
    }

    QString downloadFilename = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (downloadFilename.isEmpty()) {
        downloadFilename = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        if (downloadFilename.isEmpty()) {
            emit downloadComplete(_originalRemoteFile, QString(), tr("Unabled to find writable download location. Tried downloads and temp directory."));
            return;
        }
    }
    downloadFilename += "/" + remoteFileName;

    if (downloadFilename.isEmpty()) {
        const QString errorMsg = "Internal error";
        qCWarning(QGCFileDownloadLog) << errorMsg;
        emit downloadComplete(_originalRemoteFile, downloadFilename, errorMsg);
        return;
    }

    // Check if we should auto-decompress this file
    if (_autoDecompress && QGCCompression::isCompressedFile(remoteFileName)) {
        // Stream decompress directly from QNetworkReply to decompressed output
        const QString decompressedFilename = QGCCompression::strippedPath(downloadFilename);

        if (QGCCompression::decompressFromDevice(reply, decompressedFilename)) {
            qCDebug(QGCFileDownloadLog) << "Auto-decompressed" << remoteFileName << "to" << decompressedFilename;
            emit downloadComplete(_originalRemoteFile, decompressedFilename, QString());
            return;
        }

        // Decompression failed - fall back to saving compressed file
        qCWarning(QGCFileDownloadLog) << "Auto-decompression failed for" << remoteFileName << "- saving compressed file";
    }

    // Save file normally (either not compressed, or decompression failed/disabled)
    QFile file(downloadFilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        emit downloadComplete(_originalRemoteFile, downloadFilename, tr("Could not save downloaded file to %1. Error: %2").arg(downloadFilename, file.errorString()));
        return;
    }

    file.write(reply->readAll());
    file.close();

    emit downloadComplete(_originalRemoteFile, downloadFilename, QString());
}

void QGCFileDownload::_downloadError(QNetworkReply::NetworkError code)
{
    QString errorMsg;

    switch (code) {
    case QNetworkReply::OperationCanceledError:
        errorMsg = tr("Download cancelled");
        break;
    case QNetworkReply::ContentNotFoundError:
        errorMsg = tr("Error: File Not Found");
        break;
    default:
        errorMsg = tr("Error during download. Error: %1").arg(code);
        break;
    }

    emit downloadComplete(_originalRemoteFile, QString(), errorMsg);
}
