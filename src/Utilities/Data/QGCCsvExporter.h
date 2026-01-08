#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QLoggingCategory>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

Q_DECLARE_LOGGING_CATEGORY(QGCCsvExporterLog)

/// CSV file exporter with streaming support.
///
/// Features:
/// - Streaming writes (doesn't buffer entire file)
/// - Proper escaping of special characters
/// - Custom delimiters and line endings
/// - Header row support
/// - Auto-quoting for strings with special characters
///
/// Example usage:
/// @code
/// QGCCsvExporter exporter("/path/to/file.csv");
/// exporter.setHeaders({"Time", "Lat", "Lon", "Alt"});
/// exporter.open();
/// exporter.writeRow({timestamp, lat, lon, alt});
/// exporter.close();
/// @endcode
class QGCCsvExporter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(bool isOpen READ isOpen NOTIFY isOpenChanged)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    explicit QGCCsvExporter(QObject *parent = nullptr);
    explicit QGCCsvExporter(const QString &filePath, QObject *parent = nullptr);
    ~QGCCsvExporter() override;

    // Configuration
    QString filePath() const { return _filePath; }
    void setFilePath(const QString &path);

    QChar delimiter() const { return _delimiter; }
    void setDelimiter(QChar delim) { _delimiter = delim; }

    QString lineEnding() const { return _lineEnding; }
    void setLineEnding(const QString &ending) { _lineEnding = ending; }

    bool alwaysQuote() const { return _alwaysQuote; }
    void setAlwaysQuote(bool quote) { _alwaysQuote = quote; }

    /// Set column headers (writes when file is opened)
    void setHeaders(const QStringList &headers);
    QStringList headers() const { return _headers; }

    // Operations
    /// Open the file for writing (creates or truncates)
    Q_INVOKABLE bool open();

    /// Open for appending (preserves existing data)
    Q_INVOKABLE bool openAppend();

    /// Close the file
    Q_INVOKABLE void close();

    /// Check if file is open
    bool isOpen() const { return _file.isOpen(); }

    /// Write a row of data
    Q_INVOKABLE bool writeRow(const QVariantList &values);

    /// Write multiple rows
    Q_INVOKABLE bool writeRows(const QList<QVariantList> &rows);

    /// Write a row from a string list
    bool writeRow(const QStringList &values);

    /// Get the number of rows written
    int rowCount() const { return _rowCount; }

    /// Flush buffered data to disk
    Q_INVOKABLE void flush();

    // Static utilities

    /// Escape a single value for CSV
    static QString escapeValue(const QString &value, QChar delimiter = ',', bool alwaysQuote = false);

    /// Parse a CSV line into values
    static QStringList parseLine(const QString &line, QChar delimiter = ',');

    /// Export a list of maps to CSV
    static bool exportToFile(const QString &filePath,
                             const QList<QVariantMap> &data,
                             const QStringList &columns = {});

signals:
    void filePathChanged(const QString &path);
    void isOpenChanged(bool open);
    void rowCountChanged(int count);
    void errorOccurred(const QString &error);

private:
    QString formatRow(const QStringList &values) const;
    void emitError(const QString &error);

    QString _filePath;
    QStringList _headers;
    QChar _delimiter = ',';
    QString _lineEnding = QStringLiteral("\n");
    bool _alwaysQuote = false;

    QFile _file;
    QTextStream _stream;
    int _rowCount = 0;
};
