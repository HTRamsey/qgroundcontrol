#include "QGCFeatureFlags.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QDateTime>
#include <QtCore/QCryptographicHash>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

Q_LOGGING_CATEGORY(QGCFeatureFlagsLog, "Utilities.QGCFeatureFlags")

QGCFeatureFlags::QGCFeatureFlags(QObject *parent)
    : QObject(parent)
{
}

void QGCFeatureFlags::defineFlag(const QString &name, bool defaultEnabled, const QString &description)
{
    Flag flag;
    flag.name = name;
    flag.description = description;
    flag.type = FlagType::Boolean;
    flag.enabled = defaultEnabled;

    _flags.insert(name, flag);
    emit flagsChanged();

    qCDebug(QGCFeatureFlagsLog) << "Defined flag:" << name << "enabled:" << defaultEnabled;
}

void QGCFeatureFlags::definePercentageFlag(const QString &name, int percentage, const QString &description)
{
    Flag flag;
    flag.name = name;
    flag.description = description;
    flag.type = FlagType::Percentage;
    flag.percentage = qBound(0, percentage, 100);
    flag.enabled = percentage > 0;

    _flags.insert(name, flag);
    emit flagsChanged();

    qCDebug(QGCFeatureFlagsLog) << "Defined percentage flag:" << name << "percentage:" << percentage;
}

void QGCFeatureFlags::defineUserListFlag(const QString &name, const QStringList &users, const QString &description)
{
    Flag flag;
    flag.name = name;
    flag.description = description;
    flag.type = FlagType::UserList;
    flag.allowedUsers = users;
    flag.enabled = !users.isEmpty();

    _flags.insert(name, flag);
    emit flagsChanged();

    qCDebug(QGCFeatureFlagsLog) << "Defined user list flag:" << name << "users:" << users.size();
}

void QGCFeatureFlags::defineDateRangeFlag(const QString &name, const QDateTime &start, const QDateTime &end,
                                           const QString &description)
{
    Flag flag;
    flag.name = name;
    flag.description = description;
    flag.type = FlagType::DateTime;
    flag.startDate = start;
    flag.endDate = end;
    flag.enabled = true;

    _flags.insert(name, flag);
    emit flagsChanged();

    qCDebug(QGCFeatureFlagsLog) << "Defined date range flag:" << name << "from:" << start << "to:" << end;
}

bool QGCFeatureFlags::hasFlag(const QString &name) const
{
    return _flags.contains(name);
}

void QGCFeatureFlags::removeFlag(const QString &name)
{
    if (_flags.remove(name) > 0) {
        _overrides.remove(name);
        emit flagsChanged();
    }
}

void QGCFeatureFlags::clearFlags()
{
    _flags.clear();
    _overrides.clear();
    emit flagsChanged();
}

bool QGCFeatureFlags::isEnabled(const QString &name) const
{
    return isEnabledForUser(name, QString());
}

bool QGCFeatureFlags::isEnabledForUser(const QString &name, const QString &userId) const
{
    // Check override first
    if (_overrides.contains(name)) {
        return _overrides.value(name).toBool();
    }

    auto it = _flags.constFind(name);
    if (it == _flags.constEnd()) {
        qCDebug(QGCFeatureFlagsLog) << "Unknown flag queried:" << name;
        return false;
    }

    const_cast<QGCFeatureFlags *>(this)->trackFlagUsage(name, true);
    return _evaluateFlag(*it, userId);
}

QVariant QGCFeatureFlags::flagValue(const QString &name, const QVariant &defaultValue) const
{
    if (_overrides.contains(name)) {
        return _overrides.value(name);
    }

    auto it = _flags.constFind(name);
    if (it == _flags.constEnd()) {
        return defaultValue;
    }

    return it->metadata.value(QStringLiteral("value"), defaultValue);
}

void QGCFeatureFlags::setOverride(const QString &name, bool enabled)
{
    setOverride(name, QVariant(enabled));
}

void QGCFeatureFlags::setOverride(const QString &name, const QVariant &value)
{
    _overrides.insert(name, value);

    auto it = _flags.find(name);
    if (it != _flags.end()) {
        it->overridden = true;
    }

    emit flagOverridden(name, value);
    qCInfo(QGCFeatureFlagsLog) << "Override set:" << name << "=" << value;
}

void QGCFeatureFlags::clearOverride(const QString &name)
{
    _overrides.remove(name);

    auto it = _flags.find(name);
    if (it != _flags.end()) {
        it->overridden = false;
    }

    qCDebug(QGCFeatureFlagsLog) << "Override cleared:" << name;
}

void QGCFeatureFlags::clearAllOverrides()
{
    _overrides.clear();

    for (auto it = _flags.begin(); it != _flags.end(); ++it) {
        it->overridden = false;
    }

    qCInfo(QGCFeatureFlagsLog) << "All overrides cleared";
}

bool QGCFeatureFlags::hasOverride(const QString &name) const
{
    return _overrides.contains(name);
}

QGCFeatureFlags::Flag QGCFeatureFlags::flag(const QString &name) const
{
    return _flags.value(name);
}

QStringList QGCFeatureFlags::flagNames() const
{
    return _flags.keys();
}

QList<QGCFeatureFlags::Flag> QGCFeatureFlags::enabledFlags() const
{
    QList<Flag> result;
    for (const Flag &flag : _flags) {
        if (_evaluateFlag(flag)) {
            result.append(flag);
        }
    }
    return result;
}

QList<QGCFeatureFlags::Flag> QGCFeatureFlags::disabledFlags() const
{
    QList<Flag> result;
    for (const Flag &flag : _flags) {
        if (!_evaluateFlag(flag)) {
            result.append(flag);
        }
    }
    return result;
}

bool QGCFeatureFlags::loadFromJson(const QByteArray &json)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit error(QStringLiteral("JSON parse error: %1").arg(parseError.errorString()));
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonArray flags = root.value(QStringLiteral("flags")).toArray();

    for (const QJsonValue &val : flags) {
        const QJsonObject obj = val.toObject();
        const QString name = obj.value(QStringLiteral("name")).toString();
        const QString type = obj.value(QStringLiteral("type")).toString(QStringLiteral("boolean"));

        if (type == QStringLiteral("boolean")) {
            defineFlag(name, obj.value(QStringLiteral("enabled")).toBool(),
                      obj.value(QStringLiteral("description")).toString());
        } else if (type == QStringLiteral("percentage")) {
            definePercentageFlag(name, obj.value(QStringLiteral("percentage")).toInt(),
                                obj.value(QStringLiteral("description")).toString());
        } else if (type == QStringLiteral("userList")) {
            QStringList users;
            const QJsonArray userArray = obj.value(QStringLiteral("users")).toArray();
            for (const QJsonValue &u : userArray) {
                users.append(u.toString());
            }
            defineUserListFlag(name, users, obj.value(QStringLiteral("description")).toString());
        } else if (type == QStringLiteral("dateRange")) {
            defineDateRangeFlag(name,
                               QDateTime::fromString(obj.value(QStringLiteral("startDate")).toString(), Qt::ISODate),
                               QDateTime::fromString(obj.value(QStringLiteral("endDate")).toString(), Qt::ISODate),
                               obj.value(QStringLiteral("description")).toString());
        }

        // Store metadata
        if (obj.contains(QStringLiteral("metadata"))) {
            auto it = _flags.find(name);
            if (it != _flags.end()) {
                it->metadata = obj.value(QStringLiteral("metadata")).toObject().toVariantMap();
            }
        }
    }

    qCInfo(QGCFeatureFlagsLog) << "Loaded" << _flags.size() << "flags from JSON";
    return true;
}

bool QGCFeatureFlags::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error(QStringLiteral("Failed to open file: %1").arg(filePath));
        return false;
    }

    return loadFromJson(file.readAll());
}

QByteArray QGCFeatureFlags::toJson() const
{
    QJsonArray flagsArray;

    for (const Flag &flag : _flags) {
        QJsonObject obj;
        obj[QStringLiteral("name")] = flag.name;
        obj[QStringLiteral("description")] = flag.description;

        switch (flag.type) {
        case FlagType::Boolean:
            obj[QStringLiteral("type")] = QStringLiteral("boolean");
            obj[QStringLiteral("enabled")] = flag.enabled;
            break;
        case FlagType::Percentage:
            obj[QStringLiteral("type")] = QStringLiteral("percentage");
            obj[QStringLiteral("percentage")] = flag.percentage;
            break;
        case FlagType::UserList:
            obj[QStringLiteral("type")] = QStringLiteral("userList");
            obj[QStringLiteral("users")] = QJsonArray::fromStringList(flag.allowedUsers);
            break;
        case FlagType::DateTime:
            obj[QStringLiteral("type")] = QStringLiteral("dateRange");
            obj[QStringLiteral("startDate")] = flag.startDate.toString(Qt::ISODate);
            obj[QStringLiteral("endDate")] = flag.endDate.toString(Qt::ISODate);
            break;
        case FlagType::Custom:
            obj[QStringLiteral("type")] = QStringLiteral("custom");
            break;
        }

        if (!flag.metadata.isEmpty()) {
            obj[QStringLiteral("metadata")] = QJsonObject::fromVariantMap(flag.metadata);
        }

        flagsArray.append(obj);
    }

    QJsonObject root;
    root[QStringLiteral("flags")] = flagsArray;

    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

bool QGCFeatureFlags::saveToFile(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(toJson());
    return true;
}

void QGCFeatureFlags::setRefreshUrl(const QString &url)
{
    _refreshUrl = url;
}

void QGCFeatureFlags::refresh()
{
    if (_refreshUrl.isEmpty()) {
        emit error(QStringLiteral("No refresh URL set"));
        return;
    }

    auto *manager = new QNetworkAccessManager(this);
    const QUrl url(_refreshUrl);
    QNetworkRequest request{url};

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        if (reply->error() == QNetworkReply::NoError) {
            if (loadFromJson(reply->readAll())) {
                emit refreshCompleted(true);
            } else {
                emit refreshCompleted(false);
            }
        } else {
            emit error(QStringLiteral("Refresh failed: %1").arg(reply->errorString()));
            emit refreshCompleted(false);
        }

        reply->deleteLater();
        manager->deleteLater();
    });
}

void QGCFeatureFlags::setAutoRefreshInterval(int seconds)
{
    _autoRefreshSecs = seconds;
    // Note: Actual timer implementation would go here
}

void QGCFeatureFlags::trackFlagUsage(const QString &name, bool evaluated)
{
    Q_UNUSED(evaluated)
    _usageCount[name] = _usageCount.value(name, 0) + 1;
}

QVariantMap QGCFeatureFlags::usageStats() const
{
    QVariantMap stats;
    for (auto it = _usageCount.constBegin(); it != _usageCount.constEnd(); ++it) {
        stats[it.key()] = it.value();
    }
    return stats;
}

bool QGCFeatureFlags::_evaluateFlag(const Flag &flag, const QString &userId) const
{
    switch (flag.type) {
    case FlagType::Boolean:
        return flag.enabled;

    case FlagType::Percentage:
        if (userId.isEmpty()) {
            // Use random check for anonymous users
            return (rand() % 100) < flag.percentage;
        }
        // Deterministic check based on user ID
        return (_hashUserId(userId) % 100) < flag.percentage;

    case FlagType::UserList:
        return flag.allowedUsers.contains(userId);

    case FlagType::DateTime: {
        const QDateTime now = QDateTime::currentDateTime();
        const bool afterStart = !flag.startDate.isValid() || now >= flag.startDate;
        const bool beforeEnd = !flag.endDate.isValid() || now <= flag.endDate;
        return afterStart && beforeEnd;
    }

    case FlagType::Custom:
        return flag.enabled;  // Custom types need external evaluation
    }

    return false;
}

int QGCFeatureFlags::_hashUserId(const QString &userId) const
{
    const QByteArray hash = QCryptographicHash::hash(userId.toUtf8(), QCryptographicHash::Md5);
    // Use first 4 bytes as integer
    int result = 0;
    for (int i = 0; i < 4 && i < hash.size(); ++i) {
        result = (result << 8) | static_cast<unsigned char>(hash[i]);
    }
    return std::abs(result);
}
