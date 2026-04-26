#include "HashCheckStateMachine.h"

#include <QtStateMachine/QStateMachine>
#include <cstring>

#include "HashCheckController.h"
#include "MAVLinkProtocol.h"
#include "QGCLoggingCategory.h"
#include "Vehicle.h"

QGC_LOGGING_CATEGORY(HashCheckStateMachineLog, "Vehicle.Parameters.HashCheckStateMachine")

HashCheckStateMachine::HashCheckStateMachine(HashCheckController* controller, Vehicle* vehicle, uint8_t componentId,
                                             QObject* parent)
    : QGCStateMachine(QStringLiteral("HASH_CHECK %1").arg(vehicle->logPrefix(componentId)), vehicle, parent),
      _controller(controller),
      _componentId(componentId)
{
    _createStates();
    _wireTransitions();
    setInitialState(_sendState);

    // One-shot: clean up after the run finishes.
    connect(this, &QStateMachine::finished, this, &QObject::deleteLater);
}

void HashCheckStateMachine::_createStates()
{
    auto encoder = [this](uint8_t systemId, uint8_t channel, mavlink_message_t* message) {
        _packParamRequestRead(systemId, channel, message);
    };

    // Single send — the wait-state timeout is the retry mechanism.
    _sendState = new SendMavlinkMessageState(this, encoder, /*retryCount=*/0);

    // Wait for HashCheckController::responseReceived (PM forwards the _HASH_CHECK PARAM_VALUE arrival)
    // or for the timeout, whichever comes first.
    _waitState = new WaitForSignalState(QStringLiteral("HashCheck wait"), this, _controller,
                                        &HashCheckController::responseReceived,
                                        HashCheckController::kHashCheckTimeoutMs);

    _finalState = new QGCFinalState(this);
}

void HashCheckStateMachine::_wireTransitions()
{
    _sendState->addThisTransition(&QGCState::advance, _waitState);
    _waitState->addThisTransition(&QGCState::advance, _finalState);
    _waitState->addTransition(_waitState, &WaitStateBase::timeout, _finalState);
    connect(_waitState, &WaitStateBase::timeout, this, &HashCheckStateMachine::timedOut);
}

void HashCheckStateMachine::_packParamRequestRead(uint8_t /*systemId*/, uint8_t channel, mavlink_message_t* message)
{
    char paramId[MAVLINK_MSG_PARAM_REQUEST_READ_FIELD_PARAM_ID_LEN + 1] = {};
    std::strncpy(paramId, "_HASH_CHECK", MAVLINK_MSG_PARAM_REQUEST_READ_FIELD_PARAM_ID_LEN);

    (void)mavlink_msg_param_request_read_pack_chan(MAVLinkProtocol::instance()->getSystemId(),
                                                   MAVLinkProtocol::getComponentId(), channel, message,
                                                   static_cast<uint8_t>(vehicle()->id()), _componentId, paramId, -1);
}
