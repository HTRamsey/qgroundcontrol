#pragma once

#include "AsyncFunctionState.h"
#include "ConditionalState.h"
#include "DelayState.h"
#include "FunctionState.h"
#include "QGCAbstractTransition.h"
#include "QGCSignalTransition.h"
#include "GuardedTransition.h"
#include "InternalTransition.h"
#include "MachineEventTransition.h"
#include "NamedEventTransition.h"
#include "QGCAbstractState.h"
#include "QGCEventTransition.h"
#include "SignalDataTransition.h"
#include "QGCFinalState.h"
#include "QGCHistoryState.h"
#include "ParallelState.h"
#include "QGCStateMachineEvent.h"
#include "RequestMessageState.h"
#include "SendMavlinkCommandState.h"
#include "SendMavlinkMessageState.h"
#include "ShowAppMessageState.h"
#include "SkippableAsyncState.h"
#include "SubMachineState.h"
#include "WaitForMavlinkMessageState.h"
#include "WaitForSignalState.h"
#include "WaitStateBase.h"

#include <QtStateMachine/QStateMachine>
#include <QtStateMachine/QFinalState>
#include <QtStateMachine/QHistoryState>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include <functional>

class Vehicle;

/// QGroundControl specific state machine with enhanced error handling
class QGCStateMachine : public QStateMachine
{
    Q_OBJECT
    Q_DISABLE_COPY(QGCStateMachine)

public:
    using EntryCallback = std::function<void()>;
    using ExitCallback = std::function<void()>;
    using EventHandler = std::function<bool(QEvent*)>;

    QGCStateMachine(const QString& machineName, Vehicle* vehicle, QObject* parent = nullptr);

    Vehicle *vehicle() const { return _vehicle; }
    QString machineName() const { return objectName(); }

    /// @return true if the state machine is currently running
    /// Compatibility method matching old StateMachine::active()
    bool active() const { return isRunning(); }

    // -------------------------------------------------------------------------
    // Entry/Exit Callbacks
    // -------------------------------------------------------------------------

    /// Set a callback to be invoked when the machine starts
    void setOnEntry(EntryCallback callback) { _entryCallback = std::move(callback); }

    /// Set a callback to be invoked when the machine stops
    void setOnExit(ExitCallback callback) { _exitCallback = std::move(callback); }

    /// Set both entry and exit callbacks
    void setCallbacks(EntryCallback onEntry, ExitCallback onExit = nullptr);

    // -------------------------------------------------------------------------
    // Event Handling
    // -------------------------------------------------------------------------

    /// Set a global event handler for the machine
    /// Handler is called before events are processed by states
    void setEventHandler(EventHandler handler) { _eventHandler = std::move(handler); }

    /// Enable automatic property restoration when states exit
    /// When enabled, properties set via QGCState::setProperty/setEnabled/setVisible
    /// are automatically restored to their previous values when the state exits.
    void enablePropertyRestore();

    /// Check if property restoration is enabled
    bool isPropertyRestoreEnabled() const;

    /// Set a global error state that all QGCState errors will transition to
    /// @param errorState The state to transition to on error
    void setGlobalErrorState(QAbstractState* errorState);

    /// Get the global error state
    QAbstractState* globalErrorState() const { return _globalErrorState; }

    /// Register a QGCState with the machine and automatically wire up error transitions
    /// If a global error state is set, the state's error() signal will transition to it
    /// @param state The state to register
    void registerState(QGCState* state);

    /// Register a QGCAbstractState with the machine and automatically wire up error transitions
    /// @param state The state to register
    void registerState(QGCAbstractState* state);

    /// Create and register a FunctionState
    /// @param stateName Name for the state
    /// @param function Function to execute
    /// @return The created state
    FunctionState* addFunctionState(const QString& stateName, std::function<void()> function);

    /// Create and register an AsyncFunctionState
    /// @param stateName Name for the state
    /// @param setupFunction Function to execute on entry
    /// @param timeoutMsecs Optional timeout
    /// @return The created state
    AsyncFunctionState* addAsyncFunctionState(const QString& stateName,
                                               AsyncFunctionState::SetupFunction setupFunction,
                                               int timeoutMsecs = 0);

    /// Create and register a DelayState
    /// @param delayMsecs Delay duration
    /// @return The created state
    DelayState* addDelayState(int delayMsecs);

    /// Create and register a ParallelState
    /// @param stateName Name for the state
    /// @return The created state
    ParallelState* addParallelState(const QString& stateName);

    /// Post a named event to be processed immediately
    /// @param eventName Name of the event
    /// @param data Optional payload data
    /// @param priority Event priority (HighPriority events are processed before NormalPriority)
    void postEvent(const QString& eventName, const QVariant& data = QVariant(),
                   EventPriority priority = NormalPriority);

    /// Post a named event with a delay
    /// @param eventName Name of the event
    /// @param delayMsecs Delay in milliseconds
    /// @param data Optional payload data
    /// @return Event ID that can be used to cancel the event
    int postDelayedEvent(const QString& eventName, int delayMsecs, const QVariant& data = QVariant());

    /// Cancel a previously posted delayed event
    /// @param eventId The ID returned by postDelayedEvent
    /// @return true if the event was cancelled, false if already delivered or not found
    bool cancelDelayedEvent(int eventId);

    // -------------------------------------------------------------------------
    // State Configuration
    // -------------------------------------------------------------------------

    /// Set the initial state and optionally start the machine
    /// @param state The initial state
    /// @param autoStart If true, starts the machine immediately
    void setInitialState(QAbstractState* state, bool autoStart = false);

    /// Add a final state that will stop the machine when entered
    /// @param stateName Name for the state
    /// @return The created final state
    QGCFinalState* addFinalState(const QString& stateName = QString());

    /// Create and register a ConditionalState
    /// @param stateName Name for the state
    /// @param predicate Condition to evaluate
    /// @param action Action to execute if predicate is true
    /// @return The created state
    ConditionalState* addConditionalState(const QString& stateName,
                                          ConditionalState::Predicate predicate,
                                          ConditionalState::Action action = nullptr);

    /// Create and register a WaitForSignalState
    /// @param stateName Name for the state
    /// @param sender Object that emits the signal
    /// @param signal Signal to wait for
    /// @param timeoutMsecs Optional timeout (0 = no timeout)
    /// @return The created state
    template<typename Func>
    WaitForSignalState* addWaitForSignalState(const QString& stateName,
                                               const QObject* sender,
                                               Func signal,
                                               int timeoutMsecs = 0)
    {
        auto* state = new WaitForSignalState(stateName, this, sender, signal, timeoutMsecs);
        registerState(state);
        return state;
    }

    /// Create and register a SubMachineState
    /// @param stateName Name for the state
    /// @param factory Function that creates the child state machine
    /// @return The created state
    SubMachineState* addSubMachineState(const QString& stateName, SubMachineState::MachineFactory factory);

    // -------------------------------------------------------------------------
    // State Queries
    // -------------------------------------------------------------------------

    /// Check if a specific state is currently active
    /// @param state The state to check
    /// @return true if the state is in the current configuration
    bool isStateActive(QAbstractState* state) const;

    /// Get all currently active states
    /// @return Set of active states
    QSet<QAbstractState*> activeStates() const { return configuration(); }

    /// Find a state by name
    /// @param stateName The object name of the state
    /// @return The state, or nullptr if not found
    QAbstractState* findState(const QString& stateName) const;

    /// Find all states of a specific type
    template<typename T>
    QList<T*> findStates() const
    {
        return findChildren<T*>();
    }

    // -------------------------------------------------------------------------
    // Error Handling
    // -------------------------------------------------------------------------

    /// Check if the machine is in an error state
    bool isInErrorState() const;

    /// Get the last error message (from Qt's internal error handling)
    QString lastError() const { return errorString(); }

    /// Clear the error state and optionally restart
    /// @param restart If true, restarts the machine after clearing
    void clearError(bool restart = false);

    // -------------------------------------------------------------------------
    // Lifecycle Helpers
    // -------------------------------------------------------------------------

    /// Start the machine if not already running
    void ensureRunning();

    /// Stop the machine and optionally delete it
    /// @param deleteOnStop If true, the machine will be deleted when stopped
    void stopMachine(bool deleteOnStop = true);

    /// Restart the machine (stop then start)
    void restart();

    // -------------------------------------------------------------------------
    // Transition Helpers
    // -------------------------------------------------------------------------

    /// Create a simple signal transition between states
    /// @param from Source state
    /// @param signal Signal that triggers the transition
    /// @param to Target state
    template<typename Func>
    QSignalTransition* addTransition(QState* from, Func signal, QAbstractState* to)
    {
        return from->addTransition(from, signal, to);
    }

    /// Create a guarded transition between states
    /// @param from Source state
    /// @param signal Signal that triggers the transition
    /// @param to Target state
    /// @param guard Predicate that must return true for transition
    template<typename Func>
    GuardedTransition* addGuardedTransition(QState* from, Func signal, QAbstractState* to,
                                            GuardedTransition::Guard guard)
    {
        auto* transition = new GuardedTransition(from, signal, to, std::move(guard));
        from->addTransition(transition);
        return transition;
    }

    /// Create a machine event transition
    /// @param from Source state
    /// @param eventName Event name to match
    /// @param to Target state
    MachineEventTransition* addEventTransition(QState* from, const QString& eventName, QAbstractState* to);

public slots:
    /// Start the state machine with debug logging
    void start();

signals:
    void error();

protected:
    /// Override to perform actions when machine starts
    virtual void onEnter() {}

    /// Override to perform actions when machine stops
    virtual void onLeave() {}

    // QStateMachine overrides
    void onEntry(QEvent* event) override;
    void onExit(QEvent* event) override;
    bool event(QEvent* event) override;

private:
    Vehicle* _vehicle = nullptr;
    QAbstractState* _globalErrorState = nullptr;
    bool _deleteOnStop = false;
    EntryCallback _entryCallback;
    ExitCallback _exitCallback;
    EventHandler _eventHandler;
};
