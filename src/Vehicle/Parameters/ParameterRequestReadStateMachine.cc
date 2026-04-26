#include "ParameterRequestReadStateMachine.h"

#include <QtStateMachine/QStateMachine>
#include <cstring>

#include "AppMessages.h"
#include "MAVLinkProtocol.h"
#include "ParameterManager.h"
#include "ParameterMavlinkCodec.h"
#include "QGCLoggingCategory.h"
#include "Vehicle.h"

QGC_LOGGING_CATEGORY(ParameterRequestReadStateMachineLog, "Vehicle.Parameters.ParameterRequestReadStateMachine")

ParameterRequestReadStateMachine::ParameterRequestReadStateMachine(ParameterManager* manager, int componentId,
                                                                   const QString& paramName, int paramIndex,
                                                                   bool notifyFailure, QObject* parent)
    // Embed param + vehicle:comp into the machine name so QGCStateMachine's
    // started/stopped/finished log lines carry full context for free.
    : QGCStateMachine(QStringLiteral("PARAM_REQUEST_READ %1 %2")
                          .arg(paramName, manager->vehicleAndComponentString(componentId)),
                      manager->vehicle(), parent),
      _manager(manager),
      _componentId(componentId),
      _paramName(paramName),
      _paramIndex(paramIndex),
      _notifyFailure(notifyFailure)
{
    _createStates();
    _wireTransitions();
    setInitialState(_sendState);

    // One-shot: clean up after the run finishes.
    connect(this, &QStateMachine::finished, this, &QObject::deleteLater);
}

void ParameterRequestReadStateMachine::_createStates()
{
    auto encoder = [this](uint8_t systemId, uint8_t channel, mavlink_message_t* message) {
        _packParamRequestRead(systemId, channel, message);
    };
    auto ackPredicate = [this](const mavlink_message_t& message) { return _isMatchingParamValue(message); };
    auto errorPredicate = [this](const mavlink_message_t& message) { return _isMatchingParamError(message); };

    _sendState = new SendMavlinkMessageState(this, encoder, ParameterManager::kParamRequestReadRetryCount);
    _waitAckState =
        new WaitForParamResponseState(this, ParameterManager::kWaitForParamValueAckMs, ackPredicate, errorPredicate);

    _userNotifyState = new FunctionState(QStringLiteral("User notify"), this, [this]() {
        const QString errorDetail = _waitAckState->lastParamErrorString();
        const QString msg = errorDetail.isEmpty()
                                ? tr("Parameter read failed: param: %1 %2")
                                      .arg(_paramName, _manager->vehicleAndComponentString(_componentId))
                                : tr("Parameter read failed: param: %1 %2 - %3")
                                      .arg(_paramName, _manager->vehicleAndComponentString(_componentId), errorDetail);
        QGC::showAppMessage(msg);
    });
    _logSuccessState = new FunctionState(QStringLiteral("Log success"), this, [this]() {
        qCDebug(ParameterRequestReadStateMachineLog)
            << "PARAM_REQUEST_READ succeeded: name:" << _paramName << "index" << _paramIndex
            << _manager->vehicleAndComponentString(_componentId);
        emit readSucceeded(_componentId, _paramName, _paramIndex);
    });
    _logFailureState = new FunctionState(QStringLiteral("Log failure"), this, [this]() {
        qCDebug(ParameterRequestReadStateMachineLog)
            << "PARAM_REQUEST_READ failed: param:" << _paramName << "index" << _paramIndex
            << _manager->vehicleAndComponentString(_componentId);
        emit readFailed(_componentId, _paramName, _paramIndex);
    });
    _finalState = new QGCFinalState(this);
}

void ParameterRequestReadStateMachine::_wireTransitions()
{
    // Successful path
    _sendState->addThisTransition(&QGCState::advance, _waitAckState);
    _waitAckState->addThisTransition(&QGCState::advance, _logSuccessState);
    _logSuccessState->addThisTransition(&QGCState::advance, _finalState);

    // Retry path (timeout)
    _waitAckState->addTransition(_waitAckState, &WaitStateBase::timeout, _sendState);

    // PARAM_ERROR path (definitive rejection — no retries)
    _waitAckState->addThisTransition(&QGCState::error, _logFailureState);

    // Send retries exhausted
    _sendState->addThisTransition(&QGCState::error, _logFailureState);

    // Failure branch: optionally notify the user before completing
    if (_notifyFailure) {
        _logFailureState->addThisTransition(&QGCState::advance, _userNotifyState);
    } else {
        _logFailureState->addThisTransition(&QGCState::advance, _finalState);
    }
    _userNotifyState->addThisTransition(&QGCState::advance, _finalState);
}

void ParameterRequestReadStateMachine::_packParamRequestRead(uint8_t /*systemId*/, uint8_t channel,
                                                             mavlink_message_t* message)
{
    char paramId[MAVLINK_MSG_PARAM_REQUEST_READ_FIELD_PARAM_ID_LEN + 1] = {};
    std::strncpy(paramId, _paramName.toLatin1().constData(), MAVLINK_MSG_PARAM_REQUEST_READ_FIELD_PARAM_ID_LEN);

    (void)mavlink_msg_param_request_read_pack_chan(
        MAVLinkProtocol::instance()->getSystemId(), MAVLinkProtocol::getComponentId(), channel, message,
        static_cast<uint8_t>(_manager->vehicle()->id()), static_cast<uint8_t>(_componentId), paramId,
        static_cast<int16_t>(_paramIndex));
}

bool ParameterRequestReadStateMachine::_isMatchingParamValue(const mavlink_message_t& message) const
{
    if (message.compid != _componentId) {
        return false;
    }

    mavlink_param_value_t param_value{};
    mavlink_msg_param_value_decode(&message, &param_value);

    return ParameterMavlinkCodec::paramIdMatches(param_value.param_id, MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN,
                                                 param_value.param_index, _paramName, _paramIndex);
}

bool ParameterRequestReadStateMachine::_isMatchingParamError(const mavlink_message_t& message) const
{
    if (message.compid != _componentId) {
        return false;
    }

    mavlink_param_error_t paramError{};
    mavlink_msg_param_error_decode(&message, &paramError);

    return ParameterMavlinkCodec::paramIdMatches(paramError.param_id, MAVLINK_MSG_PARAM_ERROR_FIELD_PARAM_ID_LEN,
                                                 paramError.param_index, _paramName, _paramIndex);
}
