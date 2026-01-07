#pragma once

/// @file QGCDecompressDevice.h
/// @brief QIODevice wrapper for streaming decompression

#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QLoggingCategory>

#include <memory>

struct archive;

Q_DECLARE_LOGGING_CATEGORY(QGCDecompressDeviceLog)

/// QIODevice wrapper for streaming decompression of single-file formats
/// Supports .gz, .xz, .zst, .bz2, .lz4 compressed data
/// Read-only, sequential access only
///
/// Example usage:
/// @code
/// QGCDecompressDevice device("data.json.gz");
/// device.open(QIODevice::ReadOnly);
/// QTextStream stream(&device);
/// QString content = stream.readAll();
/// @endcode
class QGCDecompressDevice : public QIODevice
{
    Q_OBJECT

public:
    /// Construct from QIODevice source (streaming)
    /// @param source Compressed data source (must be open and readable)
    /// @param parent QObject parent
    /// @note The source device must remain valid until this device is closed
    explicit QGCDecompressDevice(QIODevice *source, QObject *parent = nullptr);

    /// Construct from file path
    /// @param filePath Path to compressed file (or Qt resource path :/)
    /// @param parent QObject parent
    explicit QGCDecompressDevice(const QString &filePath, QObject *parent = nullptr);

    ~QGCDecompressDevice() override;

    // QIODevice interface
    bool open(OpenMode mode) override;
    void close() override;
    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override;

    /// Get error description
    /// @return Error string if error occurred, empty otherwise
    QString errorString() const;

    /// Get detected format name (available after open)
    /// @return Format name like "RAW" or empty if not detected
    QString formatName() const { return _formatName; }

    /// Get detected compression filter name (available after open)
    /// @return Filter name like "gzip", "xz", "zstd", or empty if not detected
    QString filterName() const { return _filterName; }

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

private:
    bool initArchive();
    bool fillBuffer();

    // Source management
    QString _filePath;
    QIODevice *_sourceDevice = nullptr;
    bool _ownsSource = false;
    std::unique_ptr<QIODevice> _ownedSource;

    // libarchive state
    struct archive *_archive = nullptr;
    QByteArray _resourceData;  // For Qt resources
    bool _headerRead = false;
    bool _eof = false;

    // Decompressed data buffer
    QByteArray _buffer;

    // Error state
    QString _errorString;

    // Format info
    QString _formatName;
    QString _filterName;
};
