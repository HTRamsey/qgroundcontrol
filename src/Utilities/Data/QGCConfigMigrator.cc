#include "QGCConfigMigrator.h"

#include <QtCore/QFile>
#include <QtCore/QQueue>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtCore/QVersionNumber>

Q_LOGGING_CATEGORY(QGCConfigMigratorLog, "Utilities.QGCConfigMigrator")

QGCConfigMigrator::QGCConfigMigrator(QObject *parent)
    : QObject(parent)
{
}

void QGCConfigMigrator::addMigration(const QString &fromVersion, const QString &toVersion,
                                      MigrationFunc func, const QString &description)
{
    Migration m;
    m.fromVersion = fromVersion;
    m.toVersion = toVersion;
    m.function = std::move(func);
    m.description = description;
    _migrations.append(m);

    qCDebug(QGCConfigMigratorLog) << "Added migration:" << fromVersion << "->" << toVersion;
}

bool QGCConfigMigrator::migrate(QJsonObject &config, const QString &fromVersion, const QString &toVersion)
{
    if (compareVersions(fromVersion, toVersion) >= 0) {
        qCDebug(QGCConfigMigratorLog) << "No migration needed:" << fromVersion << "->" << toVersion;
        return true;
    }

    const QStringList path = migrationPath(fromVersion, toVersion);
    if (path.isEmpty()) {
        qCWarning(QGCConfigMigratorLog) << "No migration path from" << fromVersion << "to" << toVersion;
        return false;
    }

    emit migrationStarted(fromVersion, toVersion);
    qCInfo(QGCConfigMigratorLog) << "Starting migration:" << fromVersion << "->" << toVersion;

    QString currentVersion = fromVersion;
    for (int i = 0; i < path.size() - 1; ++i) {
        const QString stepFrom = path[i];
        const QString stepTo = path[i + 1];

        // Find migration
        const Migration *migration = nullptr;
        for (const Migration &m : _migrations) {
            if (m.fromVersion == stepFrom && m.toVersion == stepTo) {
                migration = &m;
                break;
            }
        }

        if (!migration) {
            const QString error = QStringLiteral("Missing migration: %1 -> %2").arg(stepFrom, stepTo);
            qCWarning(QGCConfigMigratorLog) << error;
            emit migrationFailed(fromVersion, toVersion, error);
            return false;
        }

        emit migrationStep(stepFrom, stepTo, migration->description);
        qCDebug(QGCConfigMigratorLog) << "Running migration:" << stepFrom << "->" << stepTo;

        if (!migration->function(config)) {
            const QString error = QStringLiteral("Migration failed: %1 -> %2").arg(stepFrom, stepTo);
            qCWarning(QGCConfigMigratorLog) << error;
            emit migrationFailed(fromVersion, toVersion, error);
            return false;
        }

        currentVersion = stepTo;
    }

    emit migrationCompleted(fromVersion, toVersion);
    qCInfo(QGCConfigMigratorLog) << "Migration completed:" << fromVersion << "->" << toVersion;

    return true;
}

QString QGCConfigMigrator::configVersion(const QJsonObject &config, const QString &versionKey)
{
    return config.value(versionKey).toString();
}

void QGCConfigMigrator::setConfigVersion(QJsonObject &config, const QString &version, const QString &versionKey)
{
    config[versionKey] = version;
}

int QGCConfigMigrator::compareVersions(const QString &v1, const QString &v2)
{
    const QVersionNumber ver1 = QVersionNumber::fromString(v1);
    const QVersionNumber ver2 = QVersionNumber::fromString(v2);
    return QVersionNumber::compare(ver1, ver2);
}

bool QGCConfigMigrator::needsMigration(const QString &currentVersion, const QString &targetVersion) const
{
    return compareVersions(currentVersion, targetVersion) < 0;
}

QStringList QGCConfigMigrator::migrationPath(const QString &fromVersion, const QString &toVersion) const
{
    // Build graph and find path using BFS
    QHash<QString, QStringList> graph;
    QSet<QString> versions;

    for (const Migration &m : _migrations) {
        graph[m.fromVersion].append(m.toVersion);
        versions.insert(m.fromVersion);
        versions.insert(m.toVersion);
    }

    if (!versions.contains(fromVersion) || !versions.contains(toVersion)) {
        return {};
    }

    // BFS to find shortest path
    QQueue<QStringList> queue;
    QSet<QString> visited;

    queue.enqueue({fromVersion});
    visited.insert(fromVersion);

    while (!queue.isEmpty()) {
        const QStringList path = queue.dequeue();
        const QString current = path.last();

        if (current == toVersion) {
            return path;
        }

        for (const QString &next : graph.value(current)) {
            if (!visited.contains(next)) {
                visited.insert(next);
                QStringList newPath = path;
                newPath.append(next);
                queue.enqueue(newPath);
            }
        }
    }

    return {};
}

bool QGCConfigMigrator::backupConfig(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return true;  // Nothing to backup
    }

    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString backupPath = QStringLiteral("%1/%2.%3.bak")
        .arg(fileInfo.absolutePath())
        .arg(fileInfo.fileName())
        .arg(timestamp);

    if (!QFile::copy(filePath, backupPath)) {
        qCWarning(QGCConfigMigratorLog) << "Failed to backup config:" << filePath;
        return false;
    }

    qCInfo(QGCConfigMigratorLog) << "Backed up config to:" << backupPath;
    return true;
}
