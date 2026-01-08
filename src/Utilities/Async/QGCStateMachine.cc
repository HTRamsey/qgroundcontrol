#include "QGCStateMachine.h"

#include <algorithm>

Q_LOGGING_CATEGORY(QGCStateMachineLog, "Utilities.QGCStateMachine")

QGCStateMachine::QGCStateMachine(QObject *parent)
    : QObject(parent)
{
}

void QGCStateMachine::addState(const QString &name, Action onEnter, Action onExit)
{
    addState(name, QString(), std::move(onEnter), std::move(onExit));
}

void QGCStateMachine::addState(const QString &name, const QString &parent, Action onEnter, Action onExit)
{
    if (name.isEmpty()) {
        qCWarning(QGCStateMachineLog) << "Cannot add state with empty name";
        return;
    }

    if (!parent.isEmpty() && !_states.contains(parent)) {
        qCWarning(QGCStateMachineLog) << "Parent state" << parent << "does not exist";
        return;
    }

    State state;
    state.name = name;
    state.onEnter = std::move(onEnter);
    state.onExit = std::move(onExit);
    state.parent = parent;

    _states.insert(name, state);
    qCDebug(QGCStateMachineLog) << "Added state:" << name << (parent.isEmpty() ? "" : ("(parent: " + parent + ")"));
}

void QGCStateMachine::setStateMetadata(const QString &state, const QVariantMap &metadata)
{
    auto it = _states.find(state);
    if (it != _states.end()) {
        it->metadata = metadata;
    }
}

bool QGCStateMachine::hasState(const QString &name) const
{
    return _states.contains(name);
}

QStringList QGCStateMachine::states() const
{
    return _states.keys();
}

QStringList QGCStateMachine::childStates(const QString &parent) const
{
    QStringList children;
    for (auto it = _states.constBegin(); it != _states.constEnd(); ++it) {
        if (it->parent == parent) {
            children.append(it.key());
        }
    }
    return children;
}

void QGCStateMachine::addTransition(const QString &from, const QString &event, const QString &to,
                                     GuardWithData guard, ActionWithData action, int priority)
{
    if (!_states.contains(from)) {
        qCWarning(QGCStateMachineLog) << "Source state" << from << "does not exist";
        return;
    }

    if (!_states.contains(to)) {
        qCWarning(QGCStateMachineLog) << "Target state" << to << "does not exist";
        return;
    }

    Transition t;
    t.fromState = from;
    t.event = event;
    t.toState = to;
    t.guard = std::move(guard);
    t.action = std::move(action);
    t.priority = priority;

    _transitions.append(t);
    qCDebug(QGCStateMachineLog) << "Added transition:" << from << "->" << event << "->" << to;
}

void QGCStateMachine::addTransition(const QString &from, const QString &event, const QString &to, Guard guard)
{
    GuardWithData wrappedGuard = nullptr;
    if (guard) {
        wrappedGuard = [g = std::move(guard)](const QVariant &) { return g(); };
    }
    addTransition(from, event, to, std::move(wrappedGuard), nullptr, 0);
}

bool QGCStateMachine::hasTransition(const QString &from, const QString &event) const
{
    for (const Transition &t : _transitions) {
        if (t.fromState == from && t.event == event) {
            return true;
        }
    }
    return false;
}

QStringList QGCStateMachine::events() const
{
    QSet<QString> eventSet;
    for (const Transition &t : _transitions) {
        eventSet.insert(t.event);
    }
    return eventSet.values();
}

QStringList QGCStateMachine::eventsFrom(const QString &state) const
{
    QSet<QString> eventSet;
    for (const Transition &t : _transitions) {
        if (t.fromState == state) {
            eventSet.insert(t.event);
        }
    }
    return eventSet.values();
}

void QGCStateMachine::setInitialState(const QString &state)
{
    if (!_states.contains(state)) {
        qCWarning(QGCStateMachineLog) << "Initial state" << state << "does not exist";
        return;
    }
    _initialState = state;
}

void QGCStateMachine::start()
{
    if (_running) {
        return;
    }

    if (_initialState.isEmpty()) {
        qCWarning(QGCStateMachineLog) << "No initial state set";
        return;
    }

    if (!_states.contains(_initialState)) {
        qCWarning(QGCStateMachineLog) << "Initial state" << _initialState << "does not exist";
        return;
    }

    _running = true;
    emit runningChanged(true);

    _currentState = _initialState;
    _history.clear();
    _history.append(_currentState);

    const State &state = _states.value(_currentState);
    if (state.onEnter) {
        state.onEnter();
    }

    emit stateEntered(_currentState);
    emit stateChanged(_currentState, QString());

    qCInfo(QGCStateMachineLog) << "Started in state:" << _currentState;
}

void QGCStateMachine::stop()
{
    if (!_running) {
        return;
    }

    if (!_currentState.isEmpty()) {
        const State &state = _states.value(_currentState);
        if (state.onExit) {
            state.onExit();
        }
        emit stateExited(_currentState);
    }

    _running = false;
    emit runningChanged(false);

    qCInfo(QGCStateMachineLog) << "Stopped";
}

void QGCStateMachine::reset()
{
    stop();
    _currentState.clear();
    _previousState.clear();
    _history.clear();
}

bool QGCStateMachine::trigger(const QString &event, const QVariant &data)
{
    if (!_running) {
        qCDebug(QGCStateMachineLog) << "Cannot trigger event, state machine not running";
        return false;
    }

    const QList<Transition> transitions = _findTransitions(_currentState, event);
    if (transitions.isEmpty()) {
        qCDebug(QGCStateMachineLog) << "No transition for event" << event << "from state" << _currentState;
        emit transitionFailed(_currentState, event, QStringLiteral("No matching transition"));
        return false;
    }

    for (const Transition &t : transitions) {
        if (t.guard && !t.guard(data)) {
            continue;
        }

        return _transitionTo(t.toState, event, data);
    }

    emit transitionFailed(_currentState, event, QStringLiteral("All guards failed"));
    return false;
}

bool QGCStateMachine::canTrigger(const QString &event, const QVariant &data) const
{
    if (!_running) {
        return false;
    }

    const QList<Transition> transitions = _findTransitions(_currentState, event);
    for (const Transition &t : transitions) {
        if (!t.guard || t.guard(data)) {
            return true;
        }
    }

    return false;
}

bool QGCStateMachine::isInState(const QString &state) const
{
    return _currentState == state;
}

bool QGCStateMachine::isInStateOrChild(const QString &state) const
{
    if (_currentState == state) {
        return true;
    }

    const QStringList ancestors = _getAncestors(_currentState);
    return ancestors.contains(state);
}

QStringList QGCStateMachine::stateHierarchy() const
{
    QStringList hierarchy;
    hierarchy.append(_currentState);

    QString parent = _getParentState(_currentState);
    while (!parent.isEmpty()) {
        hierarchy.prepend(parent);
        parent = _getParentState(parent);
    }

    return hierarchy;
}

QVariantMap QGCStateMachine::saveState() const
{
    QVariantMap state;
    state[QStringLiteral("currentState")] = _currentState;
    state[QStringLiteral("previousState")] = _previousState;
    state[QStringLiteral("initialState")] = _initialState;
    state[QStringLiteral("history")] = _history;
    state[QStringLiteral("running")] = _running;
    return state;
}

bool QGCStateMachine::restoreState(const QVariantMap &state)
{
    const QString savedState = state.value(QStringLiteral("currentState")).toString();
    if (!_states.contains(savedState)) {
        qCWarning(QGCStateMachineLog) << "Cannot restore to unknown state:" << savedState;
        return false;
    }

    _currentState = savedState;
    _previousState = state.value(QStringLiteral("previousState")).toString();
    _initialState = state.value(QStringLiteral("initialState")).toString();
    _history = state.value(QStringLiteral("history")).toStringList();
    _running = state.value(QStringLiteral("running")).toBool();

    if (_running) {
        emit runningChanged(true);
        emit stateChanged(_currentState, _previousState);
    }

    qCInfo(QGCStateMachineLog) << "Restored state:" << _currentState;
    return true;
}

bool QGCStateMachine::_transitionTo(const QString &toState, const QString &event, const QVariant &data)
{
    const QString fromState = _currentState;

    emit transitionStarted(fromState, event, toState);
    qCDebug(QGCStateMachineLog) << "Transition:" << fromState << "->" << event << "->" << toState;

    // Exit current state
    const State &oldState = _states.value(fromState);
    if (oldState.onExit) {
        oldState.onExit();
    }
    emit stateExited(fromState);

    // Execute transition action
    for (const Transition &t : _transitions) {
        if (t.fromState == fromState && t.event == event && t.toState == toState) {
            if (t.action) {
                t.action(data);
            }
            break;
        }
    }

    // Update state
    _previousState = fromState;
    _currentState = toState;

    // Update history
    _history.append(_currentState);
    while (_history.size() > _historySize) {
        _history.removeFirst();
    }

    // Enter new state
    const State &newState = _states.value(toState);
    if (newState.onEnter) {
        newState.onEnter();
    }
    emit stateEntered(toState);

    emit transitionCompleted(fromState, event, toState);
    emit stateChanged(toState, fromState);

    return true;
}

QList<QGCStateMachine::Transition> QGCStateMachine::_findTransitions(const QString &from, const QString &event) const
{
    QList<Transition> result;

    // Check current state and all ancestors
    QStringList statesToCheck;
    statesToCheck.append(from);
    statesToCheck.append(_getAncestors(from));

    for (const QString &state : statesToCheck) {
        for (const Transition &t : _transitions) {
            if (t.fromState == state && t.event == event) {
                result.append(t);
            }
        }
    }

    // Sort by priority (higher first)
    std::sort(result.begin(), result.end(), [](const Transition &a, const Transition &b) {
        return a.priority > b.priority;
    });

    return result;
}

QString QGCStateMachine::_getParentState(const QString &state) const
{
    auto it = _states.constFind(state);
    if (it != _states.constEnd()) {
        return it->parent;
    }
    return {};
}

QStringList QGCStateMachine::_getAncestors(const QString &state) const
{
    QStringList ancestors;
    QString parent = _getParentState(state);

    while (!parent.isEmpty()) {
        ancestors.append(parent);
        parent = _getParentState(parent);
    }

    return ancestors;
}
