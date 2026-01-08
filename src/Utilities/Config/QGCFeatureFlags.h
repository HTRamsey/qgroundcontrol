#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QVariant>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(QGCFeatureFlagsLog)

/// Feature flag management for controlled rollouts.
///
/// Supports boolean flags, percentage rollouts, user targeting,
/// and runtime flag overrides for testing.
///
/// Example usage:
/// @code
/// QGCFeatureFlags flags;
/// flags.loadFromJson(jsonConfig);
///
/// if (flags.isEnabled("new_ui")) {
///     // Show new UI
/// }
///
/// // Percentage rollout
/// if (flags.isEnabledForUser("beta_feature", userId)) {
///     // Show beta feature
/// }
/// @endcode
class QGCFeatureFlags : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int flagCount READ flagCount NOTIFY flagsChanged)

public:
    enum class FlagType {
        Boolean,       // Simple on/off
        Percentage,    // Percentage rollout (0-100)
        UserList,      // Enabled for specific users
        DateTime,      // Enabled within date range
        Custom         // Custom evaluation logic
    };
    Q_ENUM(FlagType)

    struct Flag {
        QString name;
        QString description;
        FlagType type = FlagType::Boolean;
        bool enabled = false;
        int percentage = 0;           // For Percentage type
        QStringList allowedUsers;     // For UserList type
        QDateTime startDate;          // For DateTime type
        QDateTime endDate;            // For DateTime type
        QVariantMap metadata;
        bool overridden = false;      // True if runtime override applied
    };

    explicit QGCFeatureFlags(QObject *parent = nullptr);
    ~QGCFeatureFlags() override = default;

    // Flag management
    void defineFlag(const QString &name, bool defaultEnabled = false, const QString &description = {});
    void definePercentageFlag(const QString &name, int percentage, const QString &description = {});
    void defineUserListFlag(const QString &name, const QStringList &users, const QString &description = {});
    void defineDateRangeFlag(const QString &name, const QDateTime &start, const QDateTime &end,
                              const QString &description = {});

    bool hasFlag(const QString &name) const;
    void removeFlag(const QString &name);
    void clearFlags();

    // Flag evaluation
    Q_INVOKABLE bool isEnabled(const QString &name) const;
    Q_INVOKABLE bool isEnabledForUser(const QString &name, const QString &userId) const;
    Q_INVOKABLE QVariant flagValue(const QString &name, const QVariant &defaultValue = {}) const;

    // Runtime overrides (for testing)
    void setOverride(const QString &name, bool enabled);
    void setOverride(const QString &name, const QVariant &value);
    void clearOverride(const QString &name);
    void clearAllOverrides();
    bool hasOverride(const QString &name) const;

    // Flag info
    Flag flag(const QString &name) const;
    QStringList flagNames() const;
    int flagCount() const { return static_cast<int>(_flags.size()); }
    QList<Flag> allFlags() const { return _flags.values(); }
    QList<Flag> enabledFlags() const;
    QList<Flag> disabledFlags() const;

    // Serialization
    bool loadFromJson(const QByteArray &json);
    bool loadFromFile(const QString &filePath);
    QByteArray toJson() const;
    bool saveToFile(const QString &filePath) const;

    // Refresh from remote
    void setRefreshUrl(const QString &url);
    QString refreshUrl() const { return _refreshUrl; }
    Q_INVOKABLE void refresh();
    void setAutoRefreshInterval(int seconds);

    // Analytics
    void trackFlagUsage(const QString &name, bool evaluated);
    QVariantMap usageStats() const;

signals:
    void flagsChanged();
    void flagChanged(const QString &name, bool enabled);
    void flagOverridden(const QString &name, const QVariant &value);
    void refreshCompleted(bool success);
    void error(const QString &message);

private:
    bool _evaluateFlag(const Flag &flag, const QString &userId = {}) const;
    int _hashUserId(const QString &userId) const;

    QHash<QString, Flag> _flags;
    QHash<QString, QVariant> _overrides;
    QHash<QString, int> _usageCount;
    QString _refreshUrl;
    int _autoRefreshSecs = 0;
};
