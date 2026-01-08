#pragma once

/// @file QGCFileUpload.h
/// @brief File upload with progress tracking and multipart form support

#include <QtCore/QFile>
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

class QNetworkAccessManager;
class QHttpMultiPart;

Q_DECLARE_LOGGING_CATEGORY(QGCFileUploadLog)

/// File upload with progress tracking and QML support
///
/// Features:
/// - Single file or multipart form uploads
/// - Progress reporting
/// - Cancellation support
/// - QML-compatible properties
/// - Automatic content-type detection
///
/// Example C++ usage (single file):
/// @code
/// auto *uploader = new QGCFileUpload(this);
/// connect(uploader, &QGCFileUpload::finished, this,
///         [](bool success, const QByteArray &response, const QString &error) {
///     qDebug() << "Upload" << (success ? "succeeded" : "failed");
/// });
/// uploader->uploadFile("https://api.example.com/upload", "/path/to/file.bin");
/// @endcode
///
/// Example C++ usage (multipart form):
/// @code
/// auto *uploader = new QGCFileUpload(this);
/// uploader->addFormField("description", "My file description");
/// uploader->addFormField("category", "documents");
/// uploader->uploadFile("https://api.example.com/upload", "/path/to/doc.pdf", "document");
/// @endcode
///
/// Example QML usage:
/// @code
/// QGCFileUpload {
///     id: uploader
///     onProgressChanged: progressBar.value = progress
///     onFinished: (success, response, error) => {
///         if (!success) console.log("Error:", error)
///     }
/// }
/// Button {
///     text: uploader.running ? "Cancel" : "Upload"
///     onClicked: uploader.running ? uploader.cancel()
///                                  : uploader.uploadFile(serverUrl, localFilePath)
/// }
/// @endcode
class QGCFileUpload : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QGCFileUpload)

    /// Current upload progress (0.0 to 1.0)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged FINAL)

    /// Whether an upload is currently in progress
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged FINAL)

    /// Error message from last failed upload
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)

    /// URL being uploaded to
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged FINAL)

    /// Local file path being uploaded
    Q_PROPERTY(QString localPath READ localPath NOTIFY localPathChanged FINAL)

    /// HTTP status code from last response (0 if not available)
    Q_PROPERTY(int httpStatus READ httpStatus NOTIFY httpStatusChanged FINAL)

public:
    explicit QGCFileUpload(QObject *parent = nullptr);
    ~QGCFileUpload() override;

    // Property getters
    qreal progress() const { return _progress; }
    bool isRunning() const { return _running; }
    QString errorString() const { return _errorString; }
    QUrl url() const { return _url; }
    QString localPath() const { return _localPath; }
    int httpStatus() const { return _httpStatus; }

    /// Get the response body from the last upload
    QByteArray responseBody() const { return _responseBody; }

    /// Get response headers from the last upload
    QList<QPair<QByteArray, QByteArray>> responseHeaders() const { return _responseHeaders; }

public slots:
    /// Upload a file using PUT request (raw file upload)
    /// @param url Server URL to upload to
    /// @param localPath Path to local file to upload
    /// @param contentType Optional content type (auto-detected if empty)
    /// @return true if upload started successfully
    bool uploadFile(const QString &url, const QString &localPath,
                    const QString &contentType = QString());

    /// Upload a file using POST with multipart/form-data
    /// @param url Server URL to upload to
    /// @param localPath Path to local file to upload
    /// @param fieldName Form field name for the file (default: "file")
    /// @return true if upload started successfully
    bool uploadFileMultipart(const QString &url, const QString &localPath,
                             const QString &fieldName = QStringLiteral("file"));

    /// Upload data directly using PUT request
    /// @param url Server URL to upload to
    /// @param data Data to upload
    /// @param contentType Content type header value
    /// @return true if upload started successfully
    bool uploadData(const QString &url, const QByteArray &data,
                    const QString &contentType = QStringLiteral("application/octet-stream"));

    /// Add a form field for multipart uploads
    /// Call before uploadFileMultipart() to include additional fields
    /// @param name Field name
    /// @param value Field value
    void addFormField(const QString &name, const QString &value);

    /// Clear all added form fields
    void clearFormFields();

    /// Set a custom header for the upload request
    /// @param name Header name
    /// @param value Header value
    void setHeader(const QByteArray &name, const QByteArray &value);

    /// Clear all custom headers
    void clearHeaders();

    /// Cancel the current upload
    void cancel();

signals:
    /// Emitted when upload progress changes
    void progressChanged(qreal progress);

    /// Emitted when running state changes
    void runningChanged(bool running);

    /// Emitted when error string changes
    void errorStringChanged(const QString &errorString);

    /// Emitted when URL changes
    void urlChanged(const QUrl &url);

    /// Emitted when local path changes
    void localPathChanged(const QString &localPath);

    /// Emitted when HTTP status changes
    void httpStatusChanged(int httpStatus);

    /// Emitted when upload completes
    /// @param success true if upload succeeded (HTTP 2xx)
    /// @param responseBody Response body from server
    /// @param errorMessage Error message (empty on success)
    void finished(bool success, const QByteArray &responseBody, const QString &errorMessage);

    /// Emitted during upload with byte counts
    void uploadProgress(qint64 bytesSent, qint64 totalBytes);

private slots:
    void _onUploadProgress(qint64 bytesSent, qint64 totalBytes);
    void _onFinished();
    void _onError(QNetworkReply::NetworkError error);
    void _onSslErrors(const QList<QSslError> &errors);

private:
    void _setRunning(bool running);
    void _setProgress(qreal progress);
    void _setErrorString(const QString &error);
    void _setHttpStatus(int status);
    void _cleanup();
    bool _startUpload(QNetworkReply *reply);
    QString _detectContentType(const QString &filePath) const;

    QNetworkAccessManager *_networkManager = nullptr;
    QNetworkReply *_reply = nullptr;
    QFile *_file = nullptr;
    QHttpMultiPart *_multiPart = nullptr;

    QUrl _url;
    QString _localPath;
    QString _errorString;
    QByteArray _responseBody;
    QList<QPair<QByteArray, QByteArray>> _responseHeaders;
    QList<QPair<QString, QString>> _formFields;
    QList<QPair<QByteArray, QByteArray>> _customHeaders;

    qreal _progress = 0.0;
    int _httpStatus = 0;
    bool _running = false;
};
