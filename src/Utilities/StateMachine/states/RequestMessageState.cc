#include "RequestMessageState.h"
#include "QGCLoggingCategory.h"
#include "VehicleLinkManager.h"

RequestMessageState::RequestMessageState(QState* parent,
                                         uint32_t messageId,
                                         MessageHandler messageHandler,
                                         int compId,
                                         float param1,
                                         float param2,
                                         float param3,
                                         float param4,
                                         float param5)
    : QGCState(QStringLiteral("RequestMessageState:%1").arg(messageId), parent)
    , _messageId(messageId)
    , _messageHandler(std::move(messageHandler))
    , _compId(compId)
    , _param1(param1)
    , _param2(param2)
    , _param3(param3)
    , _param4(param4)
    , _param5(param5)
{
    connect(this, &QState::entered, this, &RequestMessageState::_onEntered);
}

void RequestMessageState::_onEntered()
{
    SharedLinkInterfacePtr sharedLink = vehicle()->vehicleLinkManager()->primaryLink().lock();

    if (!sharedLink) {
        qCDebug(QGCStateMachineLog) << "Skipping request due to no primary link" << stateName();
        _failureCode = Vehicle::RequestMessageFailureCommandNotAcked;
        emit error();
        return;
    }

    if (sharedLink->linkConfiguration()->isHighLatency() || sharedLink->isLogReplay()) {
        qCDebug(QGCStateMachineLog) << "Skipping request due to link type" << stateName();
        // Not an error - just skip on high latency/replay links
        emit advance();
        return;
    }

    qCDebug(QGCStateMachineLog) << "Requesting message id" << _messageId << stateName();
    vehicle()->requestMessage(_resultHandler, this, _compId, _messageId, _param1, _param2, _param3, _param4, _param5);
}

void RequestMessageState::_resultHandler(void* resultHandlerData,
                                         MAV_RESULT commandResult,
                                         Vehicle::RequestMessageResultHandlerFailureCode_t failureCode,
                                         const mavlink_message_t& message)
{
    auto* state = static_cast<RequestMessageState*>(resultHandlerData);
    state->_failureCode = failureCode;
    state->_commandResult = commandResult;

    if (failureCode == Vehicle::RequestMessageNoFailure) {
        qCDebug(QGCStateMachineLog) << "Message received successfully" << state->stateName();

        if (state->_messageHandler) {
            state->_messageHandler(state->vehicle(), message);
        }

        emit state->messageReceived(message);
        emit state->advance();
    } else {
        switch (failureCode) {
        case Vehicle::RequestMessageFailureCommandError:
            qCDebug(QGCStateMachineLog) << "Command error" << commandResult << state->stateName();
            break;
        case Vehicle::RequestMessageFailureCommandNotAcked:
            qCDebug(QGCStateMachineLog) << "Command not acked" << state->stateName();
            break;
        case Vehicle::RequestMessageFailureMessageNotReceived:
            qCDebug(QGCStateMachineLog) << "Message not received" << state->stateName();
            break;
        case Vehicle::RequestMessageFailureDuplicateCommand:
            qCDebug(QGCStateMachineLog) << "Duplicate command" << state->stateName();
            break;
        default:
            qCDebug(QGCStateMachineLog) << "Unknown failure" << failureCode << state->stateName();
            break;
        }
        emit state->error();
    }
}
