#include "QGCMessageBus.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QMetaObject>

Q_LOGGING_CATEGORY(QGCMessageBusLog, "Utilities.QGCMessageBus")

QGCMessageBus *QGCMessageBus::_instance = nullptr;

QGCMessageBus *QGCMessageBus::instance()
{
    if (!_instance) {
        _instance = new QGCMessageBus();
    }
    return _instance;
}

QGCMessageBus::QGCMessageBus(QObject *parent)
    : QObject(parent)
{
}

QGCMessageBus::~QGCMessageBus()
{
    clear();
    if (_instance == this) {
        _instance = nullptr;
    }
}

QGCMessageBus::SubscriptionId QGCMessageBus::subscribe(const QString &topic,
                                                        QObject *context,
                                                        MessageHandler handler)
{
    QMutexLocker locker(&_mutex);

    Subscription sub;
    sub.id = _nextId++;
    sub.topic = topic;
    sub.context = context;
    sub.handler = std::move(handler);
    sub.isWildcard = topic.contains('*') || topic.contains('+');

    if (sub.isWildcard) {
        // Convert MQTT-style wildcards to regex
        QString pattern = topic;
        pattern.replace(QStringLiteral("."), QStringLiteral("\\."));
        pattern.replace(QStringLiteral("+"), QStringLiteral("[^/]+"));
        pattern.replace(QStringLiteral("*"), QStringLiteral(".*"));
        sub.pattern = QStringLiteral("^%1$").arg(pattern);
        _wildcardSubscriptions.append(sub.id);
    } else {
        _topicIndex.insert(topic, sub.id);
    }

    _subscriptions.insert(sub.id, sub);

    // Auto-unsubscribe when context is destroyed
    if (context) {
        connect(context, &QObject::destroyed, this, [this, id = sub.id]() {
            unsubscribe(id);
        });
    }

    emit subscriberCountChanged(subscriberCount());

    qCDebug(QGCMessageBusLog) << "Subscribed to" << topic << "id:" << sub.id;

    return sub.id;
}

QGCMessageBus::SubscriptionId QGCMessageBus::subscribe(const QString &topic,
                                                        MessageHandler handler)
{
    return subscribe(topic, nullptr, std::move(handler));
}

void QGCMessageBus::unsubscribe(SubscriptionId id)
{
    QMutexLocker locker(&_mutex);

    auto it = _subscriptions.find(id);
    if (it == _subscriptions.end()) {
        return;
    }

    const Subscription &sub = it.value();

    if (sub.isWildcard) {
        _wildcardSubscriptions.removeOne(id);
    } else {
        _topicIndex.remove(sub.topic, id);
    }

    _subscriptions.erase(it);

    emit subscriberCountChanged(subscriberCount());

    qCDebug(QGCMessageBusLog) << "Unsubscribed id:" << id;
}

void QGCMessageBus::unsubscribeAll(QObject *context)
{
    QMutexLocker locker(&_mutex);

    QList<SubscriptionId> toRemove;
    for (auto it = _subscriptions.begin(); it != _subscriptions.end(); ++it) {
        if (it.value().context == context) {
            toRemove.append(it.key());
        }
    }

    locker.unlock();

    for (SubscriptionId id : toRemove) {
        unsubscribe(id);
    }
}

void QGCMessageBus::publish(const QString &topic, const QVariant &data, bool async)
{
    QMutexLocker locker(&_mutex);

    _messageCount++;
    emit messageCountChanged(_messageCount);
    emit messagePublished(topic, data);

    // Collect matching subscriptions
    QList<Subscription> matches;

    // Exact matches
    const auto exactIds = _topicIndex.values(topic);
    for (SubscriptionId id : exactIds) {
        auto it = _subscriptions.find(id);
        if (it != _subscriptions.end()) {
            matches.append(it.value());
        }
    }

    // Wildcard matches
    for (SubscriptionId id : _wildcardSubscriptions) {
        auto it = _subscriptions.find(id);
        if (it != _subscriptions.end() && matchesTopic(it.value().pattern, topic, true)) {
            matches.append(it.value());
        }
    }

    locker.unlock();

    // Deliver messages outside lock
    for (const Subscription &sub : matches) {
        deliverMessage(sub, data, async);
    }

    qCDebug(QGCMessageBusLog) << "Published to" << topic << "subscribers:" << matches.size();
}

bool QGCMessageBus::matchesTopic(const QString &pattern, const QString &topic, bool isWildcard) const
{
    if (!isWildcard) {
        return pattern == topic;
    }

    QRegularExpression regex(pattern);
    return regex.match(topic).hasMatch();
}

void QGCMessageBus::deliverMessage(const Subscription &sub, const QVariant &data, bool async)
{
    if (!sub.handler) {
        return;
    }

    if (async && sub.context) {
        QMetaObject::invokeMethod(sub.context, [handler = sub.handler, data]() {
            handler(data);
        }, Qt::QueuedConnection);
    } else {
        sub.handler(data);
    }
}

bool QGCMessageBus::hasSubscribers(const QString &topic) const
{
    QMutexLocker locker(&_mutex);

    // Check exact matches
    if (_topicIndex.contains(topic)) {
        return true;
    }

    // Check wildcard matches
    for (SubscriptionId id : _wildcardSubscriptions) {
        auto it = _subscriptions.find(id);
        if (it != _subscriptions.end() && matchesTopic(it.value().pattern, topic, true)) {
            return true;
        }
    }

    return false;
}

QStringList QGCMessageBus::topics() const
{
    QMutexLocker locker(&_mutex);

    QSet<QString> topicSet;
    for (const Subscription &sub : _subscriptions) {
        topicSet.insert(sub.topic);
    }

    return topicSet.values();
}

int QGCMessageBus::subscriberCount() const
{
    QMutexLocker locker(&_mutex);
    return _subscriptions.size();
}

void QGCMessageBus::clear()
{
    QMutexLocker locker(&_mutex);

    _subscriptions.clear();
    _topicIndex.clear();
    _wildcardSubscriptions.clear();
    _messageCount = 0;

    emit subscriberCountChanged(0);
    emit messageCountChanged(0);

    qCDebug(QGCMessageBusLog) << "Cleared all subscriptions";
}
