#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

class QNetworkAccessManager;
class QAbstractNetworkCache;
class QGCCompressionJob;

Q_DECLARE_LOGGING_CATEGORY(QGCFileDownloadLog)

class QGCFileDownload : public QObject
{
    Q_OBJECT

public:
    explicit QGCFileDownload(QObject *parent = nullptr);
    ~QGCFileDownload() override;

    /// Download the specified remote file.
    ///     @param remoteFile File to download. Can be http address or file system path.
    ///     @param requestAttributes Optional request attributes to set
    ///     @param autoDecompress If true, automatically decompress .gz/.xz/.zst/.bz2/.lz4 files
    ///                           after download completes (async with progress reporting)
    ///     @return true: Asynchronous download has started, false: Download initialization failed
    bool download(const QString &remoteFile,
                  const QList<QPair<QNetworkRequest::Attribute,QVariant>> &requestAttributes = {},
                  bool autoDecompress = false);

    void setCache(QAbstractNetworkCache *cache);

signals:
    void downloadProgress(qint64 curr, qint64 total);
    void decompressionProgress(qreal progress);
    void downloadComplete(const QString &remoteFile, const QString &localFile, const QString &errorMsg);

private slots:
    void _downloadFinished();
    void _downloadError(QNetworkReply::NetworkError code);
    void _decompressionFinished(bool success);

private:
    bool _downloadInternal(const QString &remoteFile, bool redirect);

    QNetworkAccessManager *_networkManager = nullptr;
    QGCCompressionJob *_decompressionJob = nullptr;
    QString _originalRemoteFile;
    QString _pendingDecompressedFilename;
    QList<QPair<QNetworkRequest::Attribute, QVariant>> _requestAttributes;
    bool _autoDecompress = false;
};
