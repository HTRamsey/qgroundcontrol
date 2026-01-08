#include "QGCCsvExporter.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

Q_LOGGING_CATEGORY(QGCCsvExporterLog, "Utilities.QGCCsvExporter")

QGCCsvExporter::QGCCsvExporter(QObject *parent)
    : QObject(parent)
{
}

QGCCsvExporter::QGCCsvExporter(const QString &filePath, QObject *parent)
    : QObject(parent)
    , _filePath(filePath)
{
}

QGCCsvExporter::~QGCCsvExporter()
{
    close();
}

void QGCCsvExporter::setFilePath(const QString &path)
{
    if (_filePath == path) {
        return;
    }

    if (isOpen()) {
        emitError(tr("Cannot change file path while file is open"));
        return;
    }

    _filePath = path;
    emit filePathChanged(path);
}

void QGCCsvExporter::setHeaders(const QStringList &headers)
{
    if (isOpen()) {
        emitError(tr("Cannot change headers while file is open"));
        return;
    }
    _headers = headers;
}

bool QGCCsvExporter::open()
{
    if (isOpen()) {
        close();
    }

    if (_filePath.isEmpty()) {
        emitError(tr("File path is empty"));
        return false;
    }

    // Ensure directory exists
    const QFileInfo fileInfo(_filePath);
    const QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(QStringLiteral("."))) {
            emitError(tr("Failed to create directory: %1").arg(dir.absolutePath()));
            return false;
        }
    }

    _file.setFileName(_filePath);
    if (!_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        emitError(tr("Failed to open file: %1").arg(_file.errorString()));
        return false;
    }

    _stream.setDevice(&_file);
    _rowCount = 0;

    // Write headers if set
    if (!_headers.isEmpty()) {
        if (!writeRow(_headers)) {
            close();
            return false;
        }
        _rowCount = 0;  // Don't count header row
    }

    emit isOpenChanged(true);
    qCDebug(QGCCsvExporterLog) << "Opened CSV file:" << _filePath;

    return true;
}

bool QGCCsvExporter::openAppend()
{
    if (isOpen()) {
        close();
    }

    if (_filePath.isEmpty()) {
        emitError(tr("File path is empty"));
        return false;
    }

    _file.setFileName(_filePath);
    if (!_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        emitError(tr("Failed to open file for append: %1").arg(_file.errorString()));
        return false;
    }

    _stream.setDevice(&_file);
    _rowCount = 0;

    emit isOpenChanged(true);
    qCDebug(QGCCsvExporterLog) << "Opened CSV file for append:" << _filePath;

    return true;
}

void QGCCsvExporter::close()
{
    if (!isOpen()) {
        return;
    }

    _stream.flush();
    _file.close();

    emit isOpenChanged(false);
    qCDebug(QGCCsvExporterLog) << "Closed CSV file:" << _filePath << "rows:" << _rowCount;
}

bool QGCCsvExporter::writeRow(const QVariantList &values)
{
    QStringList stringValues;
    stringValues.reserve(values.size());

    for (const QVariant &value : values) {
        stringValues.append(value.toString());
    }

    return writeRow(stringValues);
}

bool QGCCsvExporter::writeRows(const QList<QVariantList> &rows)
{
    for (const QVariantList &row : rows) {
        if (!writeRow(row)) {
            return false;
        }
    }
    return true;
}

bool QGCCsvExporter::writeRow(const QStringList &values)
{
    if (!isOpen()) {
        emitError(tr("File is not open"));
        return false;
    }

    const QString line = formatRow(values);
    _stream << line << _lineEnding;

    if (_stream.status() != QTextStream::Ok) {
        emitError(tr("Failed to write row"));
        return false;
    }

    _rowCount++;
    emit rowCountChanged(_rowCount);

    return true;
}

void QGCCsvExporter::flush()
{
    if (isOpen()) {
        _stream.flush();
        _file.flush();
    }
}

QString QGCCsvExporter::formatRow(const QStringList &values) const
{
    QStringList escaped;
    escaped.reserve(values.size());

    for (const QString &value : values) {
        escaped.append(escapeValue(value, _delimiter, _alwaysQuote));
    }

    return escaped.join(_delimiter);
}

QString QGCCsvExporter::escapeValue(const QString &value, QChar delimiter, bool alwaysQuote)
{
    if (value.isEmpty()) {
        return alwaysQuote ? QStringLiteral("\"\"") : QString();
    }

    // Check if quoting is needed
    const bool needsQuote = alwaysQuote ||
                            value.contains(delimiter) ||
                            value.contains('"') ||
                            value.contains('\n') ||
                            value.contains('\r');

    if (!needsQuote) {
        return value;
    }

    // Escape quotes by doubling them
    QString escaped = value;
    escaped.replace(QStringLiteral("\""), QStringLiteral("\"\""));

    return QStringLiteral("\"%1\"").arg(escaped);
}

QStringList QGCCsvExporter::parseLine(const QString &line, QChar delimiter)
{
    QStringList result;
    QString current;
    bool inQuotes = false;
    int i = 0;

    while (i < line.length()) {
        const QChar ch = line.at(i);

        if (inQuotes) {
            if (ch == '"') {
                // Check for escaped quote
                if (i + 1 < line.length() && line.at(i + 1) == '"') {
                    current += '"';
                    i += 2;
                    continue;
                }
                inQuotes = false;
            } else {
                current += ch;
            }
        } else {
            if (ch == '"') {
                inQuotes = true;
            } else if (ch == delimiter) {
                result.append(current);
                current.clear();
            } else {
                current += ch;
            }
        }

        i++;
    }

    // Add the last field
    result.append(current);

    return result;
}

bool QGCCsvExporter::exportToFile(const QString &filePath,
                                  const QList<QVariantMap> &data,
                                  const QStringList &columns)
{
    if (data.isEmpty()) {
        return false;
    }

    // Determine columns from data if not specified
    QStringList headers = columns;
    if (headers.isEmpty() && !data.isEmpty()) {
        headers = data.first().keys();
        std::sort(headers.begin(), headers.end());
    }

    QGCCsvExporter exporter(filePath);
    exporter.setHeaders(headers);

    if (!exporter.open()) {
        return false;
    }

    for (const QVariantMap &row : data) {
        QVariantList values;
        values.reserve(headers.size());
        for (const QString &col : headers) {
            values.append(row.value(col));
        }
        if (!exporter.writeRow(values)) {
            return false;
        }
    }

    exporter.close();
    return true;
}

void QGCCsvExporter::emitError(const QString &error)
{
    qCWarning(QGCCsvExporterLog) << error;
    emit errorOccurred(error);
}
