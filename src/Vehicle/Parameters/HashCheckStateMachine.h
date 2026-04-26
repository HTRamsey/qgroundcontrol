#pragma once

#include <QtCore/QLoggingCategory>

#include <cstdint>

#include "MAVLinkLib.h"
#include "QGCStateMachine.h"

class HashCheckController;
class SendMavlinkMessageState;
class WaitForSignalState;
class QGCFinalState;
class Vehicle;

Q_DECLARE_LOGGING_CATEGORY(HashCheckStateMachineLog)

/// One-shot state machine driving a single PX4 `_HASH_CHECK` round-trip:
/// send PARAM_REQUEST_READ → wait for HashCheckController::responseReceived or timeout → done.
/// On timeout emits @ref timedOut(); HashCheckController re-emits as cacheOnlyTimedOut /
/// fullDownloadTimedOut depending on its arm-mode flag. Auto-deletes after finished().
class HashCheckStateMachine : public QGCStateMachine
{
    Q_OBJECT

public:
    HashCheckStateMachine(HashCheckController* controller, Vehicle* vehicle, uint8_t componentId,
                          QObject* parent = nullptr);

signals:
    void timedOut();

private:
    void _createStates();
    void _wireTransitions();
    void _packParamRequestRead(uint8_t systemId, uint8_t channel, mavlink_message_t* message);

    HashCheckController* _controller = nullptr;
    uint8_t _componentId = 0;

    SendMavlinkMessageState* _sendState = nullptr;
    WaitForSignalState* _waitState = nullptr;
    QGCFinalState* _finalState = nullptr;
};
