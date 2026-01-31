#pragma once

#include <QtStateMachine/QAbstractState>
#include <QtCore/QString>

#include <functional>

class QGCStateMachine;
class Vehicle;

/// Lightweight base class for simple states that don't need child states or outgoing transitions
/// Use QGCState instead if you need to add transitions from this state or have child states.
/// This class is suitable for marker states, simple action states, or states that are only
/// targets of transitions from other states.
class QGCAbstractState : public QAbstractState
{
    Q_OBJECT
    Q_DISABLE_COPY(QGCAbstractState)

public:
    using EntryCallback = std::function<void()>;
    using ExitCallback = std::function<void()>;
    using EventHandler = std::function<bool(QEvent*)>;

    QGCAbstractState(const QString& stateName, QState* parent);

    QGCStateMachine* machine() const;
    Vehicle* vehicle() const;
    QString stateName() const;

    // -------------------------------------------------------------------------
    // Entry/Exit Callbacks
    // -------------------------------------------------------------------------

    /// Set a callback to be invoked when the state is entered
    void setOnEntry(EntryCallback callback) { _entryCallback = std::move(callback); }

    /// Set a callback to be invoked when the state is exited
    void setOnExit(ExitCallback callback) { _exitCallback = std::move(callback); }

    /// Set both entry and exit callbacks
    void setCallbacks(EntryCallback onEntry, ExitCallback onExit = nullptr);

    // -------------------------------------------------------------------------
    // Event Handling
    // -------------------------------------------------------------------------

    /// Set a custom event handler for this state
    void setEventHandler(EventHandler handler) { _eventHandler = std::move(handler); }

signals:
    void advance();
    void error();

protected:
    /// Override to perform actions on state entry
    virtual void onEnter() {}

    /// Override to perform actions on state exit
    virtual void onLeave() {}

    // QAbstractState overrides
    void onEntry(QEvent* event) override;
    void onExit(QEvent* event) override;
    bool event(QEvent* event) override;

private:
    EntryCallback _entryCallback;
    ExitCallback _exitCallback;
    EventHandler _eventHandler;
};
