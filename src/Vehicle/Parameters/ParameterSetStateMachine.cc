#include "ParameterSetStateMachine.h"

#include <QtStateMachine/QStateMachine>
#include <cstring>

#include "AppMessages.h"
#include "MAVLinkProtocol.h"
#include "ParameterManager.h"
#include "ParameterMavlinkCodec.h"
#include "QGCLoggingCategory.h"
#include "QGCMath.h"
#include "Vehicle.h"

QGC_LOGGING_CATEGORY(ParameterSetStateMachineLog, "Vehicle.Parameters.ParameterSetStateMachine")

ParameterSetStateMachine::ParameterSetStateMachine(ParameterManager* manager, int componentId, const QString& paramName,
                                                   FactMetaData::ValueType_t valueType, const QVariant& rawValue,
                                                   QObject* parent)
    // Embed param + vehicle:comp into the machine name so QGCStateMachine's
    // started/stopped/finished log lines carry full context for free.
    : QGCStateMachine(
          QStringLiteral("PARAM_SET %1 %2").arg(paramName, manager->vehicleAndComponentString(componentId)),
          manager->vehicle(), parent),
      _manager(manager),
      _componentId(componentId),
      _paramName(paramName),
      _valueType(valueType),
      _rawValue(rawValue)
{
    _createStates();
    _wireTransitions();
    setInitialState(_sendState);

    // One-shot: clean up after the run finishes.
    connect(this, &QStateMachine::finished, this, &QObject::deleteLater);
}

void ParameterSetStateMachine::_createStates()
{
    auto encoder = [this](uint8_t systemId, uint8_t channel, mavlink_message_t* message) {
        _packParamSet(systemId, channel, message);
    };
    auto ackPredicate = [this](const mavlink_message_t& message) { return _isMatchingParamValue(message); };
    auto errorPredicate = [this](const mavlink_message_t& message) { return _isMatchingParamError(message); };

    _sendState = new SendMavlinkMessageState(this, encoder, ParameterManager::kParamSetRetryCount);

    _incPendingState = new FunctionState(QStringLiteral("ParameterManager increment pending write count"), this,
                                         [this]() { _manager->noteWriteStarted(); });
    _decPendingState = new FunctionState(QStringLiteral("ParameterManager decrement pending write count"), this,
                                         [this]() { _manager->noteWriteFinished(); });
    _retryDecState = new FunctionState(QStringLiteral("ParameterManager retry decrement pending write count"), this,
                                       [this]() { _manager->noteWriteFinished(); });
    _errorDecState = new FunctionState(QStringLiteral("ParameterManager error decrement pending write count"), this,
                                       [this]() { _manager->noteWriteFinished(); });

    _waitAckState =
        new WaitForParamResponseState(this, ParameterManager::kWaitForParamValueAckMs, ackPredicate, errorPredicate);

    _paramRefreshState = new FunctionState(QStringLiteral("ParameterManager param refresh"), this,
                                           [this]() { _manager->refreshParameter(_componentId, _paramName); });
    _userNotifyState = new FunctionState(QStringLiteral("ParameterManager user notify"), this, [this]() {
        const QString errorDetail = _waitAckState->lastParamErrorString();
        const QString msg = errorDetail.isEmpty()
                                ? tr("Parameter write failed: param: %1 %2")
                                      .arg(_paramName, _manager->vehicleAndComponentString(_componentId))
                                : tr("Parameter write failed: param: %1 %2 - %3")
                                      .arg(_paramName, _manager->vehicleAndComponentString(_componentId), errorDetail);
        QGC::showAppMessage(msg);
    });
    _logSuccessState = new FunctionState(QStringLiteral("ParameterManager log success"), this, [this]() {
        qCDebug(ParameterSetStateMachineLog)
            << "Parameter write succeeded: param:" << _paramName << _manager->vehicleAndComponentString(_componentId);
        emit setSucceeded(_componentId, _paramName);
    });
    _logFailureState = new FunctionState(QStringLiteral("ParameterManager log failure"), this, [this]() {
        qCDebug(ParameterSetStateMachineLog)
            << "Parameter write failed: param:" << _paramName << _manager->vehicleAndComponentString(_componentId);
        emit setFailed(_componentId, _paramName);
    });
    _finalState = new QGCFinalState(this);
}

void ParameterSetStateMachine::_wireTransitions()
{
    // Successful path
    _sendState->addThisTransition(&QGCState::advance, _incPendingState);
    _incPendingState->addThisTransition(&QGCState::advance, _waitAckState);
    _waitAckState->addThisTransition(&QGCState::advance, _decPendingState);
    _decPendingState->addThisTransition(&QGCState::advance, _logSuccessState);
    _logSuccessState->addThisTransition(&QGCState::advance, _finalState);

    // Retry path (timeout)
    _waitAckState->addTransition(_waitAckState, &WaitStateBase::timeout, _retryDecState);
    _retryDecState->addThisTransition(&QGCState::advance, _sendState);

    // PARAM_ERROR path (definitive rejection — no retries)
    _waitAckState->addThisTransition(&QGCState::error, _errorDecState);
    _errorDecState->addThisTransition(&QGCState::advance, _logFailureState);

    // Send retries exhausted
    _sendState->addThisTransition(&QGCState::error, _logFailureState);

    // Failure branch: log → notify → refresh → final
    _logFailureState->addThisTransition(&QGCState::advance, _userNotifyState);
    _userNotifyState->addThisTransition(&QGCState::advance, _paramRefreshState);
    _paramRefreshState->addThisTransition(&QGCState::advance, _finalState);
}

void ParameterSetStateMachine::_packParamSet(uint8_t /*systemId*/, uint8_t channel, mavlink_message_t* message)
{
    const MAV_PARAM_TYPE paramType = ParameterMavlinkCodec::factTypeToMavType(_valueType);

    const auto union_value = ParameterMavlinkCodec::fillUnion(_valueType, _rawValue);
    if (!union_value) {
        return;
    }

    char paramId[MAVLINK_MSG_PARAM_SET_FIELD_PARAM_ID_LEN + 1] = {};
    std::strncpy(paramId, _paramName.toLatin1().constData(), MAVLINK_MSG_PARAM_SET_FIELD_PARAM_ID_LEN);

    (void)mavlink_msg_param_set_pack_chan(MAVLinkProtocol::instance()->getSystemId(), MAVLinkProtocol::getComponentId(),
                                          channel, message, static_cast<uint8_t>(_manager->vehicle()->id()),
                                          static_cast<uint8_t>(_componentId), paramId, union_value->param_float,
                                          static_cast<uint8_t>(paramType));
}

bool ParameterSetStateMachine::_isMatchingParamValue(const mavlink_message_t& message) const
{
    if (message.compid != _componentId) {
        return false;
    }

    mavlink_param_value_t param_value{};
    mavlink_msg_param_value_decode(&message, &param_value);

    if (!ParameterMavlinkCodec::paramIdMatches(param_value.param_id, MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN,
                                               param_value.param_index, _paramName, -1)) {
        return false;
    }

    mavlink_param_union_t param_union{};
    param_union.param_float = param_value.param_value;
    param_union.type = param_value.param_type;
    const auto receivedValue = ParameterMavlinkCodec::unionToVariant(param_union);
    if (!receivedValue) {
        return false;
    }
    if (_rawValue.typeId() != receivedValue->typeId()) {
        qCWarning(ParameterSetStateMachineLog) << "QVariant type mismatch on PARAM_VALUE ack for" << _paramName
                                               << ": expected type" << _rawValue.typeId() << "got type"
                                               << receivedValue->typeId();
        return false;
    }
    if (param_value.param_type == MAV_PARAM_TYPE_REAL32) {
        return QGC::fuzzyCompare(_rawValue.toFloat(), receivedValue->toFloat());
    }
    return *receivedValue == _rawValue;
}

bool ParameterSetStateMachine::_isMatchingParamError(const mavlink_message_t& message) const
{
    if (message.compid != _componentId) {
        return false;
    }

    mavlink_param_error_t paramError{};
    mavlink_msg_param_error_decode(&message, &paramError);

    return ParameterMavlinkCodec::paramIdMatches(paramError.param_id, MAVLINK_MSG_PARAM_ERROR_FIELD_PARAM_ID_LEN,
                                                 paramError.param_index, _paramName, -1);
}
