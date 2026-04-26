#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QString>

#include "MAVLinkLib.h"
#include "QGCStateMachine.h"

class ParameterManager;
class SendMavlinkMessageState;
class WaitForParamResponseState;
class FunctionState;
class QGCFinalState;

Q_DECLARE_LOGGING_CATEGORY(ParameterRequestReadStateMachineLog)

/// One-shot state machine driving a single PARAM_REQUEST_READ round-trip:
/// send → wait for ack/error → log + signal.
/// On timeout retries the send. On PARAM_ERROR or retry exhaustion logs failure;
/// optionally notifies the user before completing.
/// Auto-deletes after finished().
class ParameterRequestReadStateMachine : public QGCStateMachine
{
    Q_OBJECT

public:
    ParameterRequestReadStateMachine(ParameterManager* manager, int componentId, const QString& paramName,
                                     int paramIndex, bool notifyFailure, QObject* parent = nullptr);

signals:
    void readSucceeded(int componentId, const QString& paramName, int paramIndex);
    void readFailed(int componentId, const QString& paramName, int paramIndex);

private:
    void _createStates();
    void _wireTransitions();

    void _packParamRequestRead(uint8_t systemId, uint8_t channel, mavlink_message_t* message);
    bool _isMatchingParamValue(const mavlink_message_t& message) const;
    bool _isMatchingParamError(const mavlink_message_t& message) const;

    ParameterManager* _manager = nullptr;
    int _componentId = 0;
    QString _paramName;
    int _paramIndex = -1;
    bool _notifyFailure = false;

    SendMavlinkMessageState* _sendState = nullptr;
    WaitForParamResponseState* _waitAckState = nullptr;
    FunctionState* _userNotifyState = nullptr;
    FunctionState* _logSuccessState = nullptr;
    FunctionState* _logFailureState = nullptr;
    QGCFinalState* _finalState = nullptr;
};
