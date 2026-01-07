#include "QGCArchiveFile.h"
#include "QGCFileHelper.h"
#include "QGClibarchive.h"
#include "QGCLoggingCategory.h"

#include <archive.h>
#include <archive_entry.h>

#include <cstring>

QGC_LOGGING_CATEGORY(QGCArchiveFileLog, "Utilities.QGCArchiveFile")

// ============================================================================
// Constructors / Destructor
// ============================================================================

QGCArchiveFile::QGCArchiveFile(const QString &archivePath, const QString &entryName, QObject *parent)
    : QIODevice(parent)
    , _archivePath(archivePath)
    , _entryName(entryName)
    , _ownsSource(true)
{
}

QGCArchiveFile::QGCArchiveFile(QIODevice *source, const QString &entryName, QObject *parent)
    : QIODevice(parent)
    , _entryName(entryName)
    , _sourceDevice(source)
{
}

QGCArchiveFile::~QGCArchiveFile()
{
    close();
}

// ============================================================================
// QIODevice Interface
// ============================================================================

bool QGCArchiveFile::open(OpenMode mode)
{
    if (mode != ReadOnly) {
        _errorString = QStringLiteral("QGCArchiveFile only supports ReadOnly mode");
        return false;
    }

    if (_entryName.isEmpty()) {
        _errorString = QStringLiteral("Entry name cannot be empty");
        return false;
    }

    // Handle file path constructor
    if (!_archivePath.isEmpty()) {
        const bool isResource = _archivePath.startsWith(QStringLiteral(":/"));
        if (isResource) {
            // Load Qt resource into memory
            QFile resourceFile(_archivePath);
            if (!resourceFile.open(QIODevice::ReadOnly)) {
                _errorString = QStringLiteral("Failed to open resource: ") + _archivePath;
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
            auto file = std::make_unique<QFile>(_archivePath);
            if (!file->open(QIODevice::ReadOnly)) {
                _errorString = QStringLiteral("Failed to open file: ") + _archivePath;
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

    if (!seekToEntry()) {
        return false;
    }

    return QIODevice::open(mode);
}

void QGCArchiveFile::close()
{
    if (_archive) {
        archive_read_free(_archive);
        _archive = nullptr;
    }

    _buffer.clear();
    _entryFound = false;
    _eof = false;
    _entrySize = -1;
    _entryModified = QDateTime();
    _formatName.clear();
    _filterName.clear();

    if (_ownsSource) {
        _ownedSource.reset();
        _sourceDevice = nullptr;
    }

    QIODevice::close();
}

qint64 QGCArchiveFile::bytesAvailable() const
{
    return _buffer.size() + QIODevice::bytesAvailable();
}

qint64 QGCArchiveFile::size() const
{
    return _entrySize >= 0 ? _entrySize : QIODevice::size();
}

QString QGCArchiveFile::errorString() const
{
    if (!_errorString.isEmpty()) {
        return _errorString;
    }
    return QIODevice::errorString();
}

// ============================================================================
// Protected QIODevice Methods
// ============================================================================

qint64 QGCArchiveFile::readData(char *data, qint64 maxSize)
{
    if (!_entryFound) {
        return -1;
    }

    if (_eof && _buffer.isEmpty()) {
        return 0;  // EOF
    }

    // Fill buffer if needed
    while (_buffer.size() < maxSize && !_eof) {
        if (!fillBuffer()) {
            if (_buffer.isEmpty()) {
                return _eof ? 0 : -1;
            }
            break;
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

qint64 QGCArchiveFile::writeData(const char *, qint64)
{
    return -1;  // Read-only device
}

// ============================================================================
// Private Methods
// ============================================================================

bool QGCArchiveFile::initArchive()
{
    _archive = archive_read_new();
    if (!_archive) {
        _errorString = QStringLiteral("Failed to create archive reader");
        return false;
    }

    // Support all formats and filters for archives
    archive_read_support_filter_all(_archive);
    archive_read_support_format_all(_archive);

    // Enable seek callback for random-access devices (improves ZIP performance)
    if (!_sourceDevice->isSequential()) {
        archive_read_set_seek_callback(_archive, QGClibarchive::deviceSeekCallback);
    }

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

    return true;
}

bool QGCArchiveFile::seekToEntry()
{
    struct archive_entry *entry = nullptr;
    _entryFound = false;

    while (archive_read_next_header(_archive, &entry) == ARCHIVE_OK) {
        // Capture format info on first header
        if (_formatName.isEmpty()) {
            const char *fmt = archive_format_name(_archive);
            _formatName = fmt ? QString::fromUtf8(fmt) : QString();
            const char *flt = archive_filter_name(_archive, 0);
            _filterName = flt ? QString::fromUtf8(flt) : QStringLiteral("none");
        }

        const QString currentEntry = QString::fromUtf8(archive_entry_pathname(entry));

        if (currentEntry == _entryName) {
            _entryFound = true;
            _entrySize = archive_entry_size(entry);

            // Get modification time if available
            if (archive_entry_mtime_is_set(entry)) {
                _entryModified = QDateTime::fromSecsSinceEpoch(archive_entry_mtime(entry));
            }

            qCDebug(QGCArchiveFileLog) << "Found entry:" << _entryName
                                        << "size:" << _entrySize
                                        << "modified:" << _entryModified;
            return true;
        }

        // Skip this entry's data
        archive_read_data_skip(_archive);
    }

    _errorString = QStringLiteral("Entry not found in archive: ") + _entryName;
    return false;
}

bool QGCArchiveFile::fillBuffer()
{
    if (_eof || !_archive || !_entryFound) {
        return false;
    }

    char readBuffer[QGCFileHelper::kBufferSizeMax];
    const la_ssize_t bytesRead = archive_read_data(_archive, readBuffer, sizeof(readBuffer));

    if (bytesRead < 0) {
        _errorString = QString::fromUtf8(archive_error_string(_archive));
        qCWarning(QGCArchiveFileLog) << "Read error:" << _errorString;
        return false;
    }

    if (bytesRead == 0) {
        _eof = true;
        return false;
    }

    _buffer.append(readBuffer, static_cast<int>(bytesRead));
    return true;
}
