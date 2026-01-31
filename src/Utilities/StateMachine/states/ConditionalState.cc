#include "ConditionalState.h"
#include "QGCLoggingCategory.h"

ConditionalState::ConditionalState(const QString& stateName, QState* parent, Predicate predicate, Action action)
    : QGCState(stateName, parent)
    , _predicate(std::move(predicate))
    , _action(std::move(action))
{
    connect(this, &QState::entered, this, &ConditionalState::_onEntered);
}

void ConditionalState::_onEntered()
{
    if (_predicate && _predicate()) {
        qCDebug(QGCStateMachineLog) << "Condition met, executing" << stateName();
        if (_action) {
            _action();
        }
        emit advance();
    } else {
        qCDebug(QGCStateMachineLog) << "Condition not met, skipping" << stateName();
        emit skipped();
    }
}
