#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QJsonObject>
#include <QtCore/QLoggingCategory>

#include <functional>

Q_DECLARE_LOGGING_CATEGORY(QGCConfigMigratorLog)

/// Settings migration helper for version upgrades.
///
/// Example usage:
/// @code
/// QGCConfigMigrator migrator;
/// migrator.addMigration("1.0", "2.0", [](QJsonObject &config) {
///     config["newKey"] = config.take("oldKey");
///     return true;
/// });
/// migrator.migrate(config, "1.0", "2.0");
/// @endcode
class QGCConfigMigrator : public QObject
{
    Q_OBJECT

public:
    using MigrationFunc = std::function<bool(QJsonObject &config)>;

    struct Migration {
        QString fromVersion;
        QString toVersion;
        MigrationFunc function;
        QString description;
    };

    explicit QGCConfigMigrator(QObject *parent = nullptr);
    ~QGCConfigMigrator() override = default;

    /// Add a migration step
    void addMigration(const QString &fromVersion, const QString &toVersion,
                      MigrationFunc func, const QString &description = {});

    /// Run all migrations from one version to another
    bool migrate(QJsonObject &config, const QString &fromVersion, const QString &toVersion);

    /// Get current config version
    static QString configVersion(const QJsonObject &config, const QString &versionKey = QStringLiteral("version"));

    /// Set config version
    static void setConfigVersion(QJsonObject &config, const QString &version,
                                  const QString &versionKey = QStringLiteral("version"));

    /// Compare versions (-1, 0, 1)
    static int compareVersions(const QString &v1, const QString &v2);

    /// Check if migration is needed
    bool needsMigration(const QString &currentVersion, const QString &targetVersion) const;

    /// Get migration path
    QStringList migrationPath(const QString &fromVersion, const QString &toVersion) const;

    /// Create backup of config before migration
    static bool backupConfig(const QString &filePath);

signals:
    void migrationStarted(const QString &fromVersion, const QString &toVersion);
    void migrationStep(const QString &fromVersion, const QString &toVersion, const QString &description);
    void migrationCompleted(const QString &fromVersion, const QString &toVersion);
    void migrationFailed(const QString &fromVersion, const QString &toVersion, const QString &error);

private:
    QList<Migration> _migrations;
};
