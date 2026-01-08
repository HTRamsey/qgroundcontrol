#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QVariant>
#include <QtCore/QLoggingCategory>

#include <functional>

Q_DECLARE_LOGGING_CATEGORY(QGCStateMachineLog)

/// Generic hierarchical state machine with guards and actions.
///
/// Example usage:
/// @code
/// QGCStateMachine sm;
/// sm.addState("idle", [](){ qDebug() << "Entered idle"; });
/// sm.addState("running", [](){ qDebug() << "Entered running"; });
/// sm.addTransition("idle", "start", "running");
/// sm.addTransition("running", "stop", "idle");
/// sm.setInitialState("idle");
/// sm.start();
/// sm.trigger("start");  // Transitions to "running"
/// @endcode
class QGCStateMachine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentState READ currentState NOTIFY stateChanged)
    Q_PROPERTY(QString previousState READ previousState NOTIFY stateChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)

public:
    using Action = std::function<void()>;
    using Guard = std::function<bool()>;
    using ActionWithData = std::function<void(const QVariant &data)>;
    using GuardWithData = std::function<bool(const QVariant &data)>;

    struct State {
        QString name;
        Action onEnter;
        Action onExit;
        QString parent;  // For hierarchical states
        QVariantMap metadata;
    };

    struct Transition {
        QString fromState;
        QString event;
        QString toState;
        GuardWithData guard;
        ActionWithData action;
        int priority = 0;  // Higher priority transitions are checked first
    };

    explicit QGCStateMachine(QObject *parent = nullptr);
    ~QGCStateMachine() override = default;

    // State management
    void addState(const QString &name, Action onEnter = nullptr, Action onExit = nullptr);
    void addState(const QString &name, const QString &parent, Action onEnter = nullptr, Action onExit = nullptr);
    void setStateMetadata(const QString &state, const QVariantMap &metadata);
    bool hasState(const QString &name) const;
    QStringList states() const;
    QStringList childStates(const QString &parent) const;

    // Transition management
    void addTransition(const QString &from, const QString &event, const QString &to,
                       GuardWithData guard = nullptr, ActionWithData action = nullptr, int priority = 0);
    void addTransition(const QString &from, const QString &event, const QString &to, Guard guard);
    bool hasTransition(const QString &from, const QString &event) const;
    QStringList events() const;
    QStringList eventsFrom(const QString &state) const;

    // Machine control
    void setInitialState(const QString &state);
    QString initialState() const { return _initialState; }
    void start();
    void stop();
    void reset();

    // Event triggering
    Q_INVOKABLE bool trigger(const QString &event, const QVariant &data = {});
    Q_INVOKABLE bool canTrigger(const QString &event, const QVariant &data = {}) const;

    // State queries
    QString currentState() const { return _currentState; }
    QString previousState() const { return _previousState; }
    bool isRunning() const { return _running; }
    bool isInState(const QString &state) const;
    bool isInStateOrChild(const QString &state) const;
    QStringList stateHierarchy() const;

    // History
    QStringList history() const { return _history; }
    void setHistorySize(int size) { _historySize = size; }
    int historySize() const { return _historySize; }

    // Persistence
    QVariantMap saveState() const;
    bool restoreState(const QVariantMap &state);

signals:
    void stateChanged(const QString &newState, const QString &oldState);
    void stateEntered(const QString &state);
    void stateExited(const QString &state);
    void transitionStarted(const QString &from, const QString &event, const QString &to);
    void transitionCompleted(const QString &from, const QString &event, const QString &to);
    void transitionFailed(const QString &from, const QString &event, const QString &reason);
    void runningChanged(bool running);

private:
    bool _transitionTo(const QString &toState, const QString &event, const QVariant &data);
    QList<Transition> _findTransitions(const QString &from, const QString &event) const;
    QString _getParentState(const QString &state) const;
    QStringList _getAncestors(const QString &state) const;

    QHash<QString, State> _states;
    QList<Transition> _transitions;
    QString _initialState;
    QString _currentState;
    QString _previousState;
    bool _running = false;
    QStringList _history;
    int _historySize = 100;
};
