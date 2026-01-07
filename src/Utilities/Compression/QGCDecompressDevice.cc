#include "QGCDecompressDevice.h"
#include "QGCFileHelper.h"
#include "QGClibarchive.h"
#include "QGCLoggingCategory.h"

#include <archive.h>
#include <archive_entry.h>

#include <cstring>

QGC_LOGGING_CATEGORY(QGCDecompressDeviceLog, "Utilities.QGCDecompressDevice")

// ============================================================================
// Constructors / Destructor
// ============================================================================

QGCDecompressDevice::QGCDecompressDevice(QIODevice *source, QObject *parent)
    : QIODevice(parent)
    , _sourceDevice(source)
    , _ownsSource(false)
{
}

QGCDecompressDevice::QGCDecompressDevice(const QString &filePath, QObject *parent)
    : QIODevice(parent)
    , _filePath(filePath)
    , _ownsSource(true)
{
}

QGCDecompressDevice::~QGCDecompressDevice()
{
    close();
}

// ============================================================================
// QIODevice Interface
// ============================================================================

bool QGCDecompressDevice::open(OpenMode mode)
{
    if (mode != ReadOnly) {
        _errorString = QStringLiteral("QGCDecompressDevice only supports ReadOnly mode");
        return false;
    }

    // Handle file path constructor
    if (!_filePath.isEmpty()) {
        const bool isResource = _filePath.startsWith(QStringLiteral(":/"));
        if (isResource) {
            // Load Qt resource into memory
            QFile resourceFile(_filePath);
            if (!resourceFile.open(QIODevice::ReadOnly)) {
                _errorString = QStringLiteral("Failed to open resource: ") + _filePath;
                return false;
            }
            _resourceData = resourceFile.readAll();
            resourceFile.close();

            // Create QBuffer as source
            auto buffer = std::make_unique<QBuffer>();
            buffer->setData(_resourceData);
            if (!buffer->open(QIODevice::ReadOnly)) {
                _errorString = QStringLiteral("Failed to open buffer for resource");
                return false;
            }
            _ownedSource = std::move(buffer);
            _sourceDevice = _ownedSource.get();
        } else {
            auto file = std::make_unique<QFile>(_filePath);
            if (!file->open(QIODevice::ReadOnly)) {
                _errorString = QStringLiteral("Failed to open file: ") + _filePath;
                return false;
            }
            _ownedSource = std::move(file);
            _sourceDevice = _ownedSource.get();
        }
    }

    // Validate source device
    if (!_sourceDevice || !_sourceDevice->isOpen() || !_sourceDevice->isReadable()) {
        _errorString = QStringLiteral("Source device is not ready");
        return false;
    }

    if (!initArchive()) {
        return false;
    }

    return QIODevice::open(mode);
}

void QGCDecompressDevice::close()
{
    if (_archive) {
        archive_read_free(_archive);
        _archive = nullptr;
    }

    _buffer.clear();
    _headerRead = false;
    _eof = false;
    _formatName.clear();
    _filterName.clear();

    if (_ownsSource) {
        _ownedSource.reset();
        _sourceDevice = nullptr;
    }

    QIODevice::close();
}

qint64 QGCDecompressDevice::bytesAvailable() const
{
    return _buffer.size() + QIODevice::bytesAvailable();
}

QString QGCDecompressDevice::errorString() const
{
    if (!_errorString.isEmpty()) {
        return _errorString;
    }
    return QIODevice::errorString();
}

// ============================================================================
// Protected QIODevice Methods
// ============================================================================

qint64 QGCDecompressDevice::readData(char *data, qint64 maxSize)
{
    if (_eof && _buffer.isEmpty()) {
        return 0;  // EOF, no more data
    }

    // Fill buffer if needed
    while (_buffer.size() < maxSize && !_eof) {
        if (!fillBuffer()) {
            if (_buffer.isEmpty()) {
                return _eof ? 0 : -1;  // Error or EOF
            }
            break;  // Return what we have
        }
    }

    // Return data from buffer
    const qint64 bytesToCopy = qMin(static_cast<qint64>(_buffer.size()), maxSize);
    if (bytesToCopy > 0) {
        std::memcpy(data, _buffer.constData(), static_cast<size_t>(bytesToCopy));
        _buffer.remove(0, static_cast<int>(bytesToCopy));
    }

    return bytesToCopy;
}

qint64 QGCDecompressDevice::writeData(const char *, qint64)
{
    return -1;  // Read-only device
}

// ============================================================================
// Private Methods
// ============================================================================

bool QGCDecompressDevice::initArchive()
{
    _archive = archive_read_new();
    if (!_archive) {
        _errorString = QStringLiteral("Failed to create archive reader");
        return false;
    }

    // Support all compression filters, raw format for single-stream decompression
    archive_read_support_filter_all(_archive);
    archive_read_support_format_raw(_archive);

    // Open using QIODevice callbacks
    const int result = archive_read_open2(_archive, _sourceDevice,
                                          nullptr,  // open callback
                                          QGClibarchive::deviceReadCallback,
                                          QGClibarchive::deviceSkipCallback,
                                          QGClibarchive::deviceCloseCallback);

    if (result != ARCHIVE_OK) {
        _errorString = QString::fromUtf8(archive_error_string(_archive));
        archive_read_free(_archive);
        _archive = nullptr;
        return false;
    }

    // Read the first (and only) entry header for raw format
    struct archive_entry *entry = nullptr;
    if (archive_read_next_header(_archive, &entry) != ARCHIVE_OK) {
        _errorString = QString::fromUtf8(archive_error_string(_archive));
        archive_read_free(_archive);
        _archive = nullptr;
        return false;
    }

    _headerRead = true;

    // Capture format info
    const char *fmt = archive_format_name(_archive);
    _formatName = fmt ? QString::fromUtf8(fmt) : QString();
    const char *flt = archive_filter_name(_archive, 0);
    _filterName = flt ? QString::fromUtf8(flt) : QStringLiteral("none");

    qCDebug(QGCDecompressDeviceLog) << "Opened compressed stream, format:" << _formatName
                                     << "filter:" << _filterName;

    return true;
}

bool QGCDecompressDevice::fillBuffer()
{
    if (_eof || !_archive) {
        return false;
    }

    char readBuffer[QGCFileHelper::kBufferSizeMax];
    const la_ssize_t bytesRead = archive_read_data(_archive, readBuffer, sizeof(readBuffer));

    if (bytesRead < 0) {
        _errorString = QString::fromUtf8(archive_error_string(_archive));
        qCWarning(QGCDecompressDeviceLog) << "Read error:" << _errorString;
        return false;
    }

    if (bytesRead == 0) {
        _eof = true;
        return false;
    }

    _buffer.append(readBuffer, static_cast<int>(bytesRead));
    return true;
}
