#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QVariant>
#include <QtCore/QMutex>
#include <QtCore/QLoggingCategory>

#include <functional>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(QGCMessageBusLog)

/// Publish-subscribe message bus for decoupled component communication.
///
/// Supports typed messages, topic filtering, and async delivery.
///
/// Example usage:
/// @code
/// // Subscribe to a topic
/// QGCMessageBus::instance()->subscribe("telemetry/altitude", this, [](const QVariant &data) {
///     double alt = data.toDouble();
///     qDebug() << "Altitude:" << alt;
/// });
///
/// // Publish a message
/// QGCMessageBus::instance()->publish("telemetry/altitude", 150.5);
/// @endcode
class QGCMessageBus : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int subscriberCount READ subscriberCount NOTIFY subscriberCountChanged)
    Q_PROPERTY(int messageCount READ messageCount NOTIFY messageCountChanged)

public:
    using MessageHandler = std::function<void(const QVariant &data)>;
    using SubscriptionId = quint64;

    static QGCMessageBus *instance();

    explicit QGCMessageBus(QObject *parent = nullptr);
    ~QGCMessageBus() override;

    /// Subscribe to a topic
    /// @param topic Topic name (supports wildcards: "telemetry/*", "vehicle/+/status")
    /// @param context QObject for automatic unsubscription when destroyed
    /// @param handler Callback function
    /// @return Subscription ID for manual unsubscription
    SubscriptionId subscribe(const QString &topic, QObject *context, MessageHandler handler);

    /// Subscribe without context (manual unsubscription required)
    SubscriptionId subscribe(const QString &topic, MessageHandler handler);

    /// Unsubscribe by ID
    void unsubscribe(SubscriptionId id);

    /// Unsubscribe all subscriptions for a context
    void unsubscribeAll(QObject *context);

    /// Publish a message to a topic
    /// @param topic Topic name
    /// @param data Message payload
    /// @param async If true, deliver via queued connection (default: false)
    Q_INVOKABLE void publish(const QString &topic, const QVariant &data = {}, bool async = false);

    /// Check if topic has any subscribers
    Q_INVOKABLE bool hasSubscribers(const QString &topic) const;

    /// Get list of all active topics
    QStringList topics() const;

    // Statistics
    int subscriberCount() const;
    int messageCount() const { return _messageCount; }

    /// Clear all subscriptions
    Q_INVOKABLE void clear();

signals:
    void subscriberCountChanged(int count);
    void messageCountChanged(int count);
    void messagePublished(const QString &topic, const QVariant &data);

private:
    struct Subscription {
        SubscriptionId id;
        QString topic;
        QString pattern;  // For wildcard matching
        QObject *context = nullptr;
        MessageHandler handler;
        bool isWildcard = false;
    };

    bool matchesTopic(const QString &pattern, const QString &topic, bool isWildcard) const;
    void deliverMessage(const Subscription &sub, const QVariant &data, bool async);

    mutable QMutex _mutex;
    QHash<SubscriptionId, Subscription> _subscriptions;
    QMultiHash<QString, SubscriptionId> _topicIndex;  // Fast lookup by exact topic
    QList<SubscriptionId> _wildcardSubscriptions;     // Subscriptions with wildcards

    SubscriptionId _nextId = 1;
    int _messageCount = 0;

    static QGCMessageBus *_instance;
};
