#include "QGCStateMachine.h"
#include "QGCApplication.h"
#include "AudioOutput.h"
#include "MultiVehicleManager.h"
#include "Vehicle.h"

#include <QtStateMachine/QFinalState>
#include <QtStateMachine/QSignalTransition>

QGCStateMachine::QGCStateMachine(const QString& machineName, Vehicle *vehicle, QObject* parent)
    : QStateMachine (parent)
    , _vehicle      (vehicle)
{
    setObjectName(machineName);

    connect(this, &QGCStateMachine::started, this, [this] () {
        qCDebug(QGCStateMachineLog) << "State machine started:" << objectName();
    });
    connect(this, &QGCStateMachine::stopped, this, [this] () {
        qCDebug(QGCStateMachineLog) << "State machine stopped:" << objectName();
        if (_deleteOnStop) {
            deleteLater();
        }
    });
    connect(this, &QGCStateMachine::finished, this, [this] () {
        qCDebug(QGCStateMachineLog) << "State machine finished:" << objectName();
    });
}

void QGCStateMachine::start()
{
    if (isRunning()) {
        qCWarning(QGCStateMachineLog) << objectName() << "start() called but already running - check signal connections";
    }
    QStateMachine::start();
}

void QGCStateMachine::setGlobalErrorState(QAbstractState* errorState)
{
    _globalErrorState = errorState;
}

void QGCStateMachine::enablePropertyRestore()
{
    setGlobalRestorePolicy(QState::RestoreProperties);
    qCDebug(QGCStateMachineLog) << objectName() << "property restore enabled";
}

bool QGCStateMachine::isPropertyRestoreEnabled() const
{
    return globalRestorePolicy() == QState::RestoreProperties;
}

void QGCStateMachine::registerState(QGCState* state)
{
    // Only add global error transition if no local error state is set
    if (_globalErrorState && !state->localErrorState()) {
        state->addTransition(state, &QGCState::error, _globalErrorState);
    }
}

void QGCStateMachine::registerState(QGCAbstractState* state)
{
    // QGCAbstractState cannot have local error states, so always wire to global
    if (_globalErrorState) {
        // Need to create a signal transition manually since QGCAbstractState
        // doesn't inherit QState::addTransition
        auto* transition = new QSignalTransition(state, &QGCAbstractState::error);
        transition->setTargetState(_globalErrorState);
        // Parent the transition to the machine so it gets cleaned up
        transition->setParent(this);
    }
}

FunctionState* QGCStateMachine::addFunctionState(const QString& stateName, std::function<void()> function)
{
    auto* state = new FunctionState(stateName, this, std::move(function));
    registerState(state);
    return state;
}

AsyncFunctionState* QGCStateMachine::addAsyncFunctionState(const QString& stateName,
                                                            AsyncFunctionState::SetupFunction setupFunction,
                                                            int timeoutMsecs)
{
    auto* state = new AsyncFunctionState(stateName, this, std::move(setupFunction), timeoutMsecs);
    registerState(state);
    return state;
}

DelayState* QGCStateMachine::addDelayState(int delayMsecs)
{
    auto* state = new DelayState(this, delayMsecs);
    registerState(state);
    return state;
}

ParallelState* QGCStateMachine::addParallelState(const QString& stateName)
{
    auto* state = new ParallelState(stateName, this);
    registerState(state);
    return state;
}

void QGCStateMachine::postEvent(const QString& eventName, const QVariant& data, EventPriority priority)
{
    qCDebug(QGCStateMachineLog) << objectName() << "posting event:" << eventName
                                 << (priority == HighPriority ? "(high priority)" : "");
    QStateMachine::postEvent(new QGCStateMachineEvent(eventName, data), priority);
}

int QGCStateMachine::postDelayedEvent(const QString& eventName, int delayMsecs, const QVariant& data)
{
    qCDebug(QGCStateMachineLog) << objectName() << "posting delayed event:" << eventName << "delay:" << delayMsecs << "ms";
    return QStateMachine::postDelayedEvent(new QGCStateMachineEvent(eventName, data), delayMsecs);
}

bool QGCStateMachine::cancelDelayedEvent(int eventId)
{
    qCDebug(QGCStateMachineLog) << objectName() << "cancelling delayed event id:" << eventId;
    return QStateMachine::cancelDelayedEvent(eventId);
}

// -----------------------------------------------------------------------------
// State Configuration
// -----------------------------------------------------------------------------

void QGCStateMachine::setInitialState(QAbstractState* state, bool autoStart)
{
    QStateMachine::setInitialState(state);
    if (autoStart) {
        start();
    }
}

QGCFinalState* QGCStateMachine::addFinalState(const QString& stateName)
{
    auto* state = new QGCFinalState(stateName.isEmpty() ? QStringLiteral("Final") : stateName, this);
    return state;
}

ConditionalState* QGCStateMachine::addConditionalState(const QString& stateName,
                                                        ConditionalState::Predicate predicate,
                                                        ConditionalState::Action action)
{
    auto* state = new ConditionalState(stateName, this, std::move(predicate), std::move(action));
    registerState(state);
    return state;
}

SubMachineState* QGCStateMachine::addSubMachineState(const QString& stateName, SubMachineState::MachineFactory factory)
{
    auto* state = new SubMachineState(stateName, this, std::move(factory));
    registerState(state);
    return state;
}

// -----------------------------------------------------------------------------
// State Queries
// -----------------------------------------------------------------------------

bool QGCStateMachine::isStateActive(QAbstractState* state) const
{
    return configuration().contains(state);
}

QAbstractState* QGCStateMachine::findState(const QString& stateName) const
{
    const auto states = findChildren<QAbstractState*>();
    for (QAbstractState* state : states) {
        if (state->objectName() == stateName) {
            return state;
        }
    }
    return nullptr;
}

// -----------------------------------------------------------------------------
// Error Handling
// -----------------------------------------------------------------------------

bool QGCStateMachine::isInErrorState() const
{
    if (_globalErrorState) {
        return configuration().contains(_globalErrorState);
    }
    return false;
}

void QGCStateMachine::clearError(bool restart)
{
    if (restart) {
        stop();
        start();
    }
}

// -----------------------------------------------------------------------------
// Lifecycle Helpers
// -----------------------------------------------------------------------------

void QGCStateMachine::ensureRunning()
{
    if (!isRunning()) {
        start();
    }
}

void QGCStateMachine::stopMachine(bool deleteOnStop)
{
    _deleteOnStop = deleteOnStop;
    stop();
}

void QGCStateMachine::restart()
{
    if (isRunning()) {
        // stop() is asynchronous, so connect to stopped signal to start again
        QMetaObject::Connection* conn = new QMetaObject::Connection();
        *conn = connect(this, &QStateMachine::stopped, this, [this, conn]() {
            disconnect(*conn);
            delete conn;
            start();
        });
        stop();
    } else {
        start();
    }
}

// -----------------------------------------------------------------------------
// Transition Helpers
// -----------------------------------------------------------------------------

MachineEventTransition* QGCStateMachine::addEventTransition(QState* from, const QString& eventName, QAbstractState* to)
{
    auto* transition = new MachineEventTransition(eventName, to);
    from->addTransition(transition);
    return transition;
}

// -----------------------------------------------------------------------------
// Entry/Exit Callbacks
// -----------------------------------------------------------------------------

void QGCStateMachine::setCallbacks(EntryCallback onEntry, ExitCallback onExit)
{
    _entryCallback = std::move(onEntry);
    _exitCallback = std::move(onExit);
}

// -----------------------------------------------------------------------------
// QStateMachine Overrides
// -----------------------------------------------------------------------------

void QGCStateMachine::onEntry(QEvent* event)
{
    QStateMachine::onEntry(event);

    if (_entryCallback) {
        _entryCallback();
    }

    onEnter();
}

void QGCStateMachine::onExit(QEvent* event)
{
    onLeave();

    if (_exitCallback) {
        _exitCallback();
    }

    QStateMachine::onExit(event);
}

bool QGCStateMachine::event(QEvent* event)
{
    // Only allow handler to intercept custom events, not internal state machine events
    if (_eventHandler && event->type() >= QEvent::User) {
        if (_eventHandler(event)) {
            return true;
        }
    }

    return QStateMachine::event(event);
}
