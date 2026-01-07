#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

class QNetworkAccessManager;
class QAbstractNetworkCache;

Q_DECLARE_LOGGING_CATEGORY(QGCFileDownloadLog)

class QGCFileDownload : public QObject
{
    Q_OBJECT

    enum HTTP_Response {
        SUCCESS_OK = 200,
        REDIRECTION_MULTIPLE_CHOICES = 300
    };

public:
    explicit QGCFileDownload(QObject *parent = nullptr);
    ~QGCFileDownload() override;

    /// Download the specified remote file.
    ///     @param remoteFile File to download. Can be http address or file system path.
    ///     @param requestAttributes Optional request attributes to set
    ///     @param autoDecompress If true, automatically decompress .gz/.xz/.zst/.bz2/.lz4 files
    ///                           during download (streams directly to decompressed output)
    ///     @return true: Asynchronous download has started, false: Download initialization failed
    bool download(const QString &remoteFile,
                  const QList<QPair<QNetworkRequest::Attribute,QVariant>> &requestAttributes = {},
                  bool autoDecompress = false);

    void setCache(QAbstractNetworkCache *cache);

    static void setIgnoreSSLErrorsIfNeeded(QNetworkReply &networkReply);

signals:
    void downloadProgress(qint64 curr, qint64 total);
    void downloadComplete(const QString &remoteFile, const QString &localFile, const QString &errorMsg);

private slots:
    void _downloadFinished();

    /// Called when an error occurs during download
    void _downloadError(QNetworkReply::NetworkError code);

private:
    bool _downloadInternal(const QString &remoteFile, bool redirect);

    QNetworkAccessManager *_networkManager = nullptr;
    QString _originalRemoteFile;
    QList<QPair<QNetworkRequest::Attribute, QVariant>> _requestAttributes;
    bool _autoDecompress = false;
};
