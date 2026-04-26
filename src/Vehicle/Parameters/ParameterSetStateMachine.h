#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include "FactMetaData.h"
#include "MAVLinkLib.h"
#include "QGCStateMachine.h"

class ParameterManager;
class SendMavlinkMessageState;
class WaitForParamResponseState;
class FunctionState;
class QGCFinalState;

Q_DECLARE_LOGGING_CATEGORY(ParameterSetStateMachineLog)

/// One-shot state machine driving a single PARAM_SET round-trip:
/// send → increment pending → wait for ack/error → decrement pending → log + signal.
/// On timeout retries the send. On PARAM_ERROR or retry exhaustion logs failure,
/// notifies the user and refreshes the parameter from the vehicle.
/// Auto-deletes after finished().
class ParameterSetStateMachine : public QGCStateMachine
{
    Q_OBJECT

public:
    ParameterSetStateMachine(ParameterManager* manager, int componentId, const QString& paramName,
                             FactMetaData::ValueType_t valueType, const QVariant& rawValue, QObject* parent = nullptr);

signals:
    void setSucceeded(int componentId, const QString& paramName);
    void setFailed(int componentId, const QString& paramName);

private:
    void _createStates();
    void _wireTransitions();

    void _packParamSet(uint8_t systemId, uint8_t channel, mavlink_message_t* message);
    bool _isMatchingParamValue(const mavlink_message_t& message) const;
    bool _isMatchingParamError(const mavlink_message_t& message) const;

    ParameterManager* _manager = nullptr;
    int _componentId = 0;
    QString _paramName;
    FactMetaData::ValueType_t _valueType = FactMetaData::valueTypeInt32;
    QVariant _rawValue;

    SendMavlinkMessageState* _sendState = nullptr;
    FunctionState* _incPendingState = nullptr;
    WaitForParamResponseState* _waitAckState = nullptr;
    FunctionState* _decPendingState = nullptr;
    FunctionState* _retryDecState = nullptr;
    FunctionState* _errorDecState = nullptr;
    FunctionState* _paramRefreshState = nullptr;
    FunctionState* _userNotifyState = nullptr;
    FunctionState* _logSuccessState = nullptr;
    FunctionState* _logFailureState = nullptr;
    QGCFinalState* _finalState = nullptr;
};
