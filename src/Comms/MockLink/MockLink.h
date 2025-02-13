#ifndef MOCKLINK_H
#define MOCKLINK_H

#include "LinkInterface.h"
#include "MAVLinkLib.h"
#include "MockConfiguration.h"
#include "MockLinkMissionItemHandler.h"
#include <QMutex>
#include <QElapsedTimer>
#include <QGeoCoordinate>
#include <QMap>
#include <QList>
#include <QThread>

// Forward declarations:
class MockLinkFTP;
class MockLinkWorker;

Q_DECLARE_LOGGING_CATEGORY(MockLinkLog)
Q_DECLARE_LOGGING_CATEGORY(MockLinkVerboseLog)

/**
 * @brief The MockLink class
 *
 * This class simulates a MAVLink communication link. It used to subclass QThread
 * and implement its own event loop. Now it uses a separate worker object that runs
 * on its own QThread.
 */
class MockLink : public LinkInterface {
    Q_OBJECT
public:
    explicit MockLink(SharedLinkConfigurationPtr &config, QObject *parent = nullptr);
    virtual ~MockLink();

    // LinkInterface overrides:
    bool isConnected() const final { return _connected; }
    bool _connect() final;
    void disconnect() final;

    Q_INVOKABLE void setCommLost(bool commLost) { _commLost = commLost; }
    Q_INVOKABLE void simulateConnectionRemoved();

    int vehicleId() const { return _vehicleSystemId; }
    MAV_AUTOPILOT getFirmwareType() const { return _firmwareType; }

    void emitRemoteControlChannelRawChanged(int channel, uint16_t raw);
    void respondWithMavlinkMessage(const mavlink_message_t &msg);
    MockLinkFTP *mockLinkFTP() const;

    // Methods to set failure modes for testing
    void setMissionItemFailureMode(MockLinkMissionItemHandler::FailureMode_t failureMode, MAV_MISSION_RESULT failureAckResult) const { _missionItemHandler->setFailureMode(failureMode, failureAckResult); }
    void sendUnexpectedMissionAck(MAV_MISSION_RESULT ackType) const { _missionItemHandler->sendUnexpectedMissionAck(ackType); }
    void sendUnexpectedMissionItem() const { _missionItemHandler->sendUnexpectedMissionItem(); }
    void sendUnexpectedMissionRequest() const { _missionItemHandler->sendUnexpectedMissionRequest(); }
    void sendUnexpectedCommandAck(MAV_CMD command, MAV_RESULT ackResult);

    void resetMissionItemHandler() const { _missionItemHandler->reset(); }
    QString logDownloadFile() const { return _logDownloadFilename; }
    void clearReceivedMavCommandCounts() { _receivedMavCommandCountMap.clear(); }
    int receivedMavCommandCount(MAV_CMD command) const { return _receivedMavCommandCountMap[command]; }

    enum RequestMessageFailureMode_t {
        FailRequestMessageNone,
        FailRequestMessageCommandAcceptedMsgNotSent,
        FailRequestMessageCommandUnsupported,
        FailRequestMessageCommandNoResponse,
    };
    void setRequestMessageFailureMode(RequestMessageFailureMode_t failureMode) { _requestMessageFailureMode = failureMode; }

    // Static methods to start various mock links:
    static MockLink *startPX4MockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode = MockConfiguration::FailNone);
    static MockLink *startGenericMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode = MockConfiguration::FailNone);
    static MockLink *startNoInitialConnectMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode = MockConfiguration::FailNone);
    static MockLink *startAPMArduCopterMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode = MockConfiguration::FailNone);
    static MockLink *startAPMArduPlaneMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode = MockConfiguration::FailNone);
    static MockLink *startAPMArduSubMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode = MockConfiguration::FailNone);
    static MockLink *startAPMArduRoverMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode = MockConfiguration::FailNone);

    // Special command constants for testing:
    static constexpr MAV_CMD MAV_CMD_MOCKLINK_ALWAYS_RESULT_ACCEPTED = MAV_CMD_USER_1;
    static constexpr MAV_CMD MAV_CMD_MOCKLINK_ALWAYS_RESULT_FAILED = MAV_CMD_USER_2;
    static constexpr MAV_CMD MAV_CMD_MOCKLINK_SECOND_ATTEMPT_RESULT_ACCEPTED = MAV_CMD_USER_3;
    static constexpr MAV_CMD MAV_CMD_MOCKLINK_SECOND_ATTEMPT_RESULT_FAILED = MAV_CMD_USER_4;
    static constexpr MAV_CMD MAV_CMD_MOCKLINK_NO_RESPONSE = MAV_CMD_USER_5;
    static constexpr MAV_CMD MAV_CMD_MOCKLINK_NO_RESPONSE_NO_RETRY = static_cast<MAV_CMD>(MAV_CMD_USER_5 + 1);
    static constexpr MAV_CMD MAV_CMD_MOCKLINK_RESULT_IN_PROGRESS_ACCEPTED = static_cast<MAV_CMD>(MAV_CMD_USER_5 + 2);
    static constexpr MAV_CMD MAV_CMD_MOCKLINK_RESULT_IN_PROGRESS_FAILED = static_cast<MAV_CMD>(MAV_CMD_USER_5 + 3);
    static constexpr MAV_CMD MAV_CMD_MOCKLINK_RESULT_IN_PROGRESS_NO_ACK = static_cast<MAV_CMD>(MAV_CMD_USER_5 + 4);

    // ——— Worker thread related ———
    // These methods are called by the worker object on its timers.
    void run1HzTasks();
    void run10HzTasks();
    void run500HzTasks();
    void sendStatusTextMessages();

    /// Returns true if status text messages should be sent.
    bool shouldSendStatusText() const { return _sendStatusText; }

signals:
    void connected();
    void disconnected();
    void writeBytesQueuedSignal(const QByteArray &bytes);
    void highLatencyTransmissionEnabledChanged(bool highLatencyTransmissionEnabled);

private slots:
    void _writeBytes(const QByteArray &bytes) final;
    void _writeBytesQueued(const QByteArray &bytes);
    void _run1HzTasks();
    void _run10HzTasks();
    void _run500HzTasks();
    void _sendStatusTextMessages();

    // All other original private slots…
    void _handleIncomingNSHBytes(const char *bytes, int cBytes);
    void _handleIncomingMavlinkBytes(const uint8_t *bytes, int cBytes);
    void _handleIncomingMavlinkMsg(const mavlink_message_t &msg);
    void _handleHeartBeat(const mavlink_message_t &msg);
    void _handleParamMapRC(const mavlink_message_t &msg);
    void _handleSetMode(const mavlink_message_t &msg);
    void _handleManualControl(const mavlink_message_t &msg);
    void _setParamFloatUnionIntoMap(int componentId, const QString &paramName, float paramFloat);
    float _floatUnionForParam(int componentId, const QString &paramName);
    void _handleParamRequestList(const mavlink_message_t &msg);
    void _paramRequestListWorker();
    void _handleParamSet(const mavlink_message_t &msg);
    void _handleParamRequestRead(const mavlink_message_t &msg);
    void _handleFTP(const mavlink_message_t &msg);
    void _handleInProgressCommandLong(const mavlink_command_long_t &request);
    void _handleCommandLong(const mavlink_message_t &msg);
    void _respondWithAutopilotVersion();
    void _sendHomePosition();
    void _sendGpsRawInt();
    void _sendGlobalPositionInt();
    void _sendExtendedSysState();
    void _sendChunkedStatusText(uint16_t chunkId, bool missingChunks);
    void _handlePreFlightCalibration(const mavlink_command_long_t& request);
    void _handleTakeoff(const mavlink_command_long_t &request);
    void _handleLogRequestList(const mavlink_message_t &msg);
    void _handleLogRequestData(const mavlink_message_t &msg);
    void _logDownloadWorker();
    void _sendADSBVehicles();
    void _moveADSBVehicle(int vehicleIndex);
    bool _handleRequestMessage(const mavlink_command_long_t &request, bool &noAck);
    void _sendGeneralMetaData();
    void _sendRemoteIDArmStatus();

private:
    // Private members from the original implementation:
    const MockConfiguration *_mockConfig = nullptr;
    const MAV_AUTOPILOT _firmwareType = MAV_AUTOPILOT_PX4;
    const MAV_TYPE _vehicleType = MAV_TYPE_QUADROTOR;
    const bool _sendStatusText = false;
    const MockConfiguration::FailureMode_t _failureMode = MockConfiguration::FailNone;
    const uint8_t _vehicleSystemId = 0;
    const double _vehicleLatitude = 0.0;
    const double _vehicleLongitude = 0.0;
    const uint16_t _boardVendorId = 0;
    const uint16_t _boardProductId = 0;
    MockLinkMissionItemHandler *const _missionItemHandler = nullptr;
    MockLinkFTP *const _mockLinkFTP = nullptr;

    uint8_t _mavlinkAuxChannel = std::numeric_limits<uint8_t>::max();
    QMutex _mavlinkAuxMutex;

    bool _connected = false;
    bool _inNSH = false;
    uint8_t _mavBaseMode = MAV_MODE_FLAG_MANUAL_INPUT_ENABLED | MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
    uint32_t _mavCustomMode = 0;
    uint8_t _mavState = MAV_STATE_STANDBY;
    QElapsedTimer _runningTime;
    static constexpr int32_t _batteryMaxTimeRemaining = 15 * 60;
    int8_t _battery1PctRemaining = 100;
    int32_t _battery1TimeRemaining = _batteryMaxTimeRemaining;
    MAV_BATTERY_CHARGE_STATE _battery1ChargeState = MAV_BATTERY_CHARGE_STATE_OK;
    int8_t _battery2PctRemaining = 100;
    int32_t _battery2TimeRemaining = _batteryMaxTimeRemaining;
    MAV_BATTERY_CHARGE_STATE _battery2ChargeState = MAV_BATTERY_CHARGE_STATE_OK;
    double _vehicleAltitudeAMSL = 0.0;
    bool _commLost = false;
    bool _highLatencyTransmissionEnabled = true;
    int _sendHomePositionDelayCount = 10;
    int _sendGPSPositionDelayCount = 100;
    int _currentParamRequestListComponentIndex = -1;
    int _currentParamRequestListParamIndex = -1;
    QString _logDownloadFilename;
    uint32_t _logDownloadCurrentOffset = 0;
    uint32_t _logDownloadBytesRemaining = 0;
    RequestMessageFailureMode_t _requestMessageFailureMode = FailRequestMessageNone;
    QMap<MAV_CMD, int> _receivedMavCommandCountMap;
    QMap<int, QMap<QString, QVariant>> _mapParamName2Value;
    QMap<int, QMap<QString, MAV_PARAM_TYPE>> _mapParamName2MavParamType;

    struct ADSBVehicle {
        QGeoCoordinate coordinate;
        double angle = 0.0;
        double altitude = 0.0;
    };
    QList<ADSBVehicle> _adsbVehicles;
    QList<QGeoCoordinate> _adsbVehicleCoordinates;
    static constexpr int _numberOfVehicles = 5;
    double _adsbAngles[_numberOfVehicles]{};

    static int _nextVehicleSystemId;
    static constexpr double _defaultVehicleLatitude = 47.397;
    static constexpr double _defaultVehicleLongitude = 8.5455;
    static constexpr double _defaultVehicleHomeAltitude = 488.056;
    static constexpr const char *_failParam = "COM_FLTMODE6";
    static constexpr uint8_t _vehicleComponentId = MAV_COMP_ID_AUTOPILOT1;
    static constexpr uint16_t _logDownloadLogId = 0;
    static constexpr uint32_t _logDownloadFileSize = 1000;
    static constexpr bool _mavlinkStarted = true;
    
    // Worker thread members:
    QThread*         _workerThread = nullptr;
    MockLinkWorker*  _worker = nullptr;
};

#endif // MOCKLINK_H
