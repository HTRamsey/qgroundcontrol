#pragma once

/// @file QGCArchiveFile.h
/// @brief QIODevice for reading a single entry from an archive

#include <QtCore/QBuffer>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QLoggingCategory>

#include <memory>

struct archive;

Q_DECLARE_LOGGING_CATEGORY(QGCArchiveFileLog)

/// QIODevice for reading a single entry from an archive without full extraction
/// Supports ZIP, TAR, 7z, and other libarchive-supported formats
/// Read-only, sequential access only
///
/// Example usage:
/// @code
/// QGCArchiveFile file("archive.zip", "config.json");
/// file.open(QIODevice::ReadOnly);
/// QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
/// @endcode
class QGCArchiveFile : public QIODevice
{
    Q_OBJECT

public:
    /// Construct from archive file path
    /// @param archivePath Path to archive file (or Qt resource path :/)
    /// @param entryName Name of entry to read within the archive
    /// @param parent QObject parent
    QGCArchiveFile(const QString &archivePath, const QString &entryName, QObject *parent = nullptr);

    /// Construct from QIODevice source (streaming)
    /// @param source Archive data source (must be open and readable)
    /// @param entryName Name of entry to read within the archive
    /// @param parent QObject parent
    /// @note The source device must remain valid until this device is closed
    QGCArchiveFile(QIODevice *source, const QString &entryName, QObject *parent = nullptr);

    ~QGCArchiveFile() override;

    // QIODevice interface
    bool open(OpenMode mode) override;
    void close() override;
    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override;
    qint64 size() const override;

    /// Get entry name being read
    /// @return Entry name within the archive
    QString entryName() const { return _entryName; }

    /// Get entry size (available after open)
    /// @return Entry size in bytes, or -1 if unknown
    qint64 entrySize() const { return _entrySize; }

    /// Check if entry was found in archive (available after open)
    /// @return true if entry exists and was found
    bool entryFound() const { return _entryFound; }

    /// Get entry modification time (available after open)
    /// @return Modification time, or invalid QDateTime if unknown
    QDateTime entryModified() const { return _entryModified; }

    /// Get error description
    /// @return Error string if error occurred, empty otherwise
    QString errorString() const;

    /// Get detected archive format name (available after open)
    /// @return Format name like "ZIP 2.0", "POSIX ustar", or empty if not detected
    QString formatName() const { return _formatName; }

    /// Get detected compression filter name (available after open)
    /// @return Filter name like "gzip", "xz", "none", or empty if not detected
    QString filterName() const { return _filterName; }

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

private:
    bool initArchive();
    bool seekToEntry();
    bool fillBuffer();

    // Source management
    QString _archivePath;
    QString _entryName;
    QIODevice *_sourceDevice = nullptr;
    bool _ownsSource = false;
    std::unique_ptr<QIODevice> _ownedSource;

    // libarchive state
    struct archive *_archive = nullptr;
    QByteArray _resourceData;  // For Qt resources
    bool _entryFound = false;
    bool _eof = false;

    // Entry metadata
    qint64 _entrySize = -1;
    QDateTime _entryModified;

    // Decompressed data buffer
    QByteArray _buffer;

    // Error state
    QString _errorString;

    // Format info
    QString _formatName;
    QString _filterName;
};
