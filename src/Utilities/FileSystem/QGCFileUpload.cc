#include "QGCFileUpload.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFileInfo>
#include <QtCore/QMimeDatabase>
#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>

QGC_LOGGING_CATEGORY(QGCFileUploadLog, "Utilities.QGCFileUpload")

// ============================================================================
// Construction / Destruction
// ============================================================================

QGCFileUpload::QGCFileUpload(QObject *parent)
    : QObject(parent)
    , _networkManager(new QNetworkAccessManager(this))
{
    qCDebug(QGCFileUploadLog) << "Created";
}

QGCFileUpload::~QGCFileUpload()
{
    cancel();
    qCDebug(QGCFileUploadLog) << "Destroyed";
}

// ============================================================================
// Upload Methods
// ============================================================================

bool QGCFileUpload::uploadFile(const QString &url, const QString &localPath,
                                const QString &contentType)
{
    if (_running) {
        qCWarning(QGCFileUploadLog) << "Upload already in progress";
        return false;
    }

    if (url.isEmpty()) {
        _setErrorString(tr("Empty URL"));
        return false;
    }

    if (localPath.isEmpty()) {
        _setErrorString(tr("Empty file path"));
        return false;
    }

    _file = new QFile(localPath, this);
    if (!_file->open(QIODevice::ReadOnly)) {
        _setErrorString(tr("Cannot open file: %1").arg(_file->errorString()));
        delete _file;
        _file = nullptr;
        return false;
    }

    _url = QUrl::fromUserInput(url);
    emit urlChanged(_url);

    _localPath = localPath;
    emit localPathChanged(_localPath);

    QNetworkRequest request(_url);

    // Set content type
    const QString mimeType = contentType.isEmpty() ? _detectContentType(localPath) : contentType;
    request.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    request.setHeader(QNetworkRequest::ContentLengthHeader, _file->size());

    // Apply custom headers
    for (const auto &header : _customHeaders) {
        request.setRawHeader(header.first, header.second);
    }

    qCDebug(QGCFileUploadLog) << "Uploading" << localPath << "to" << url
                               << "size:" << _file->size() << "type:" << mimeType;

    QNetworkReply *reply = _networkManager->put(request, _file);
    return _startUpload(reply);
}

bool QGCFileUpload::uploadFileMultipart(const QString &url, const QString &localPath,
                                         const QString &fieldName)
{
    if (_running) {
        qCWarning(QGCFileUploadLog) << "Upload already in progress";
        return false;
    }

    if (url.isEmpty()) {
        _setErrorString(tr("Empty URL"));
        return false;
    }

    if (localPath.isEmpty()) {
        _setErrorString(tr("Empty file path"));
        return false;
    }

    _file = new QFile(localPath, this);
    if (!_file->open(QIODevice::ReadOnly)) {
        _setErrorString(tr("Cannot open file: %1").arg(_file->errorString()));
        delete _file;
        _file = nullptr;
        return false;
    }

    _url = QUrl::fromUserInput(url);
    emit urlChanged(_url);

    _localPath = localPath;
    emit localPathChanged(_localPath);

    // Create multipart form
    _multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);

    // Add form fields
    for (const auto &field : _formFields) {
        QHttpPart textPart;
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QStringLiteral("form-data; name=\"%1\"").arg(field.first));
        textPart.setBody(field.second.toUtf8());
        _multiPart->append(textPart);
    }

    // Add file part
    const QFileInfo fileInfo(localPath);
    const QString mimeType = _detectContentType(localPath);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QStringLiteral("form-data; name=\"%1\"; filename=\"%2\"")
                           .arg(fieldName, fileInfo.fileName()));
    filePart.setBodyDevice(_file);
    _multiPart->append(filePart);

    QNetworkRequest request(_url);

    // Apply custom headers
    for (const auto &header : _customHeaders) {
        request.setRawHeader(header.first, header.second);
    }

    qCDebug(QGCFileUploadLog) << "Uploading multipart" << localPath << "to" << url
                               << "field:" << fieldName << "with" << _formFields.size() << "form fields";

    QNetworkReply *reply = _networkManager->post(request, _multiPart);
    return _startUpload(reply);
}

bool QGCFileUpload::uploadData(const QString &url, const QByteArray &data,
                                const QString &contentType)
{
    if (_running) {
        qCWarning(QGCFileUploadLog) << "Upload already in progress";
        return false;
    }

    if (url.isEmpty()) {
        _setErrorString(tr("Empty URL"));
        return false;
    }

    _url = QUrl::fromUserInput(url);
    emit urlChanged(_url);

    _localPath.clear();
    emit localPathChanged(_localPath);

    QNetworkRequest request(_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.size());

    // Apply custom headers
    for (const auto &header : _customHeaders) {
        request.setRawHeader(header.first, header.second);
    }

    qCDebug(QGCFileUploadLog) << "Uploading data to" << url
                               << "size:" << data.size() << "type:" << contentType;

    QNetworkReply *reply = _networkManager->put(request, data);
    return _startUpload(reply);
}

// ============================================================================
// Form Field Management
// ============================================================================

void QGCFileUpload::addFormField(const QString &name, const QString &value)
{
    _formFields.append({name, value});
    qCDebug(QGCFileUploadLog) << "Added form field:" << name;
}

void QGCFileUpload::clearFormFields()
{
    _formFields.clear();
    qCDebug(QGCFileUploadLog) << "Cleared form fields";
}

void QGCFileUpload::setHeader(const QByteArray &name, const QByteArray &value)
{
    // Remove existing header with same name
    _customHeaders.erase(
        std::remove_if(_customHeaders.begin(), _customHeaders.end(),
                       [&name](const auto &h) { return h.first == name; }),
        _customHeaders.end());
    _customHeaders.append({name, value});
}

void QGCFileUpload::clearHeaders()
{
    _customHeaders.clear();
}

void QGCFileUpload::cancel()
{
    if (_reply != nullptr) {
        qCDebug(QGCFileUploadLog) << "Cancelling upload";
        _reply->abort();
    }
}

// ============================================================================
// Private Slots
// ============================================================================

void QGCFileUpload::_onUploadProgress(qint64 bytesSent, qint64 totalBytes)
{
    if (totalBytes > 0) {
        _setProgress(static_cast<qreal>(bytesSent) / static_cast<qreal>(totalBytes));
    }
    emit uploadProgress(bytesSent, totalBytes);
}

void QGCFileUpload::_onFinished()
{
    if (_reply == nullptr) {
        return;
    }

    _setHttpStatus(_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    _responseBody = _reply->readAll();

    // Collect response headers
    _responseHeaders.clear();
    for (const auto &header : _reply->rawHeaderPairs()) {
        _responseHeaders.append(header);
    }

    const bool success = (_reply->error() == QNetworkReply::NoError) &&
                         (_httpStatus >= 200 && _httpStatus < 300);

    qCDebug(QGCFileUploadLog) << "Upload finished - success:" << success
                               << "status:" << _httpStatus
                               << "response size:" << _responseBody.size();

    if (success) {
        _setErrorString({});
        emit finished(true, _responseBody, {});
    } else {
        QString errorMsg = _errorString;
        if (errorMsg.isEmpty()) {
            errorMsg = _reply->errorString();
        }
        if (errorMsg.isEmpty() && _httpStatus > 0) {
            errorMsg = tr("HTTP %1").arg(_httpStatus);
        }
        _setErrorString(errorMsg);
        emit finished(false, _responseBody, errorMsg);
    }

    _cleanup();
    _setRunning(false);
}

void QGCFileUpload::_onError(QNetworkReply::NetworkError error)
{
    if (error == QNetworkReply::OperationCanceledError) {
        qCDebug(QGCFileUploadLog) << "Upload cancelled";
        _setErrorString(tr("Upload cancelled"));
    } else {
        qCWarning(QGCFileUploadLog) << "Network error:" << error;
        if (_reply != nullptr) {
            _setErrorString(_reply->errorString());
        }
    }
}

void QGCFileUpload::_onSslErrors(const QList<QSslError> &errors)
{
    QStringList errorStrings;
    for (const QSslError &error : errors) {
        errorStrings.append(error.errorString());
        qCWarning(QGCFileUploadLog) << "SSL error:" << error.errorString();
    }
    _setErrorString(tr("SSL errors: %1").arg(errorStrings.join(QStringLiteral(", "))));
}

// ============================================================================
// Private Methods
// ============================================================================

void QGCFileUpload::_setRunning(bool running)
{
    if (_running != running) {
        _running = running;
        emit runningChanged(running);
    }
}

void QGCFileUpload::_setProgress(qreal progress)
{
    if (!qFuzzyCompare(_progress, progress)) {
        _progress = progress;
        emit progressChanged(progress);
    }
}

void QGCFileUpload::_setErrorString(const QString &error)
{
    if (_errorString != error) {
        _errorString = error;
        emit errorStringChanged(error);
    }
}

void QGCFileUpload::_setHttpStatus(int status)
{
    if (_httpStatus != status) {
        _httpStatus = status;
        emit httpStatusChanged(status);
    }
}

void QGCFileUpload::_cleanup()
{
    if (_reply != nullptr) {
        _reply->deleteLater();
        _reply = nullptr;
    }

    if (_file != nullptr) {
        _file->close();
        delete _file;
        _file = nullptr;
    }

    if (_multiPart != nullptr) {
        delete _multiPart;
        _multiPart = nullptr;
    }
}

bool QGCFileUpload::_startUpload(QNetworkReply *reply)
{
    _reply = reply;
    _responseBody.clear();
    _responseHeaders.clear();
    _setHttpStatus(0);
    _setProgress(0.0);
    _setErrorString({});
    _setRunning(true);

    connect(_reply, &QNetworkReply::uploadProgress,
            this, &QGCFileUpload::_onUploadProgress);
    connect(_reply, &QNetworkReply::finished,
            this, &QGCFileUpload::_onFinished);
    connect(_reply, &QNetworkReply::errorOccurred,
            this, &QGCFileUpload::_onError);
    connect(_reply, &QNetworkReply::sslErrors,
            this, &QGCFileUpload::_onSslErrors);

    return true;
}

QString QGCFileUpload::_detectContentType(const QString &filePath) const
{
    QMimeDatabase db;
    const QMimeType mimeType = db.mimeTypeForFile(filePath);
    return mimeType.isValid() ? mimeType.name() : QStringLiteral("application/octet-stream");
}
