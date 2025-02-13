#include "MockLink.h"
#include "MockLinkWorker.h"
#include "MockLinkFTP.h"
#include "LinkManager.h"
#include "QGCApplication.h"
#include "QGCLoggingCategory.h"

#include <QFile>
#include <QMutexLocker>
#include <QRandomGenerator>
#include <QTemporaryFile>
#include <QTimer>
#include <QTextStream>

QGC_LOGGING_CATEGORY(MockLinkLog, "qgc.comms.mocklink.mocklink")
QGC_LOGGING_CATEGORY(MockLinkVerboseLog, "qgc.comms.mocklink.mocklink:verbose")

int MockLink::_nextVehicleSystemId = 128;

MockLink::MockLink(SharedLinkConfigurationPtr &config, QObject *parent)
    : LinkInterface(config, parent)
    , _mockConfig(qobject_cast<const MockConfiguration*>(_config.get()))
    , _firmwareType(_mockConfig->firmwareType())
    , _vehicleType(_mockConfig->vehicleType())
    , _sendStatusText(_mockConfig->sendStatusText())
    , _failureMode(_mockConfig->failureMode())
    , _vehicleSystemId(_mockConfig->incrementVehicleId() ? _nextVehicleSystemId++ : _nextVehicleSystemId)
    , _vehicleLatitude(_defaultVehicleLatitude + ((_vehicleSystemId - 128) * 0.0001))
    , _vehicleLongitude(_defaultVehicleLongitude + ((_vehicleSystemId - 128) * 0.0001))
    , _boardVendorId(_mockConfig->boardVendorId())
    , _boardProductId(_mockConfig->boardProductId())
    , _missionItemHandler(new MockLinkMissionItemHandler(this))
    , _mockLinkFTP(new MockLinkFTP(_vehicleSystemId, _vehicleComponentId, this))
{
    // NOTE: In the original implementation this object moved itself to its own thread.
    // That code is removed. Instead, we create a worker thread.

    // Initialize ADS-B vehicles:
    _adsbVehicles.resize(_numberOfVehicles);
    for (int i = 0; i < _numberOfVehicles; ++i) {
        ADSBVehicle vehicle{};
        vehicle.angle = i * 72;
        const double latOffset = 0.001 * i;
        const double lonOffset = 0.001 * (i % 2 == 0 ? i : -i);
        vehicle.coordinate = QGeoCoordinate(_defaultVehicleLatitude + latOffset, _defaultVehicleLongitude + lonOffset);
        vehicle.altitude = _defaultVehicleHomeAltitude + (i * 5);
        _adsbVehicles.append(vehicle);
    }

    connect(this, &MockLink::writeBytesQueuedSignal, this, &MockLink::_writeBytesQueued, Qt::QueuedConnection);

    // Load parameters and start the elapsed timer.
    _loadParams();
    _runningTime.start();

    // Create and start the worker thread:
    _workerThread = new QThread(this);
    _worker = new MockLinkWorker(this);
    _worker->moveToThread(_workerThread);
    connect(_workerThread, &QThread::started, _worker, &MockLinkWorker::startWork);
    connect(_workerThread, &QThread::finished, _worker, &QObject::deleteLater);
    _workerThread->start();
}

MockLink::~MockLink()
{
    simulateConnectionRemoved();

    if (!_logDownloadFilename.isEmpty()) {
        QFile::remove(_logDownloadFilename);
    }

    // Clean up the worker thread.
    if (_workerThread) {
        _workerThread->quit();
        _workerThread->wait();
    }
}

bool MockLink::_connect()
{
    if (!_connected) {
        _connected = true;
        mavlink_status_t *const mavlinkStatus = mavlink_get_channel_status(mavlinkChannel());
        mavlinkStatus->flags &= ~MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
        mavlink_status_t *const auxStatus = mavlink_get_channel_status(_mavlinkAuxChannel);
        auxStatus->flags &= ~MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
        emit connected();
    }
    return true;
}

void MockLink::disconnect()
{
    if (_connected) {
        _connected = false;
        // In the original implementation, quit() and wait() were called on the QThread.
        // Now, we simply emit disconnected and let the worker thread shut down on destruction.
        emit disconnected();
    }
}

// ----- The following methods are now called by the worker object -----
//
// These methods were formerly called via QTimer connections in run()
// (which has been removed) and are now invoked by MockLinkWorker.

void MockLink::run1HzTasks() {
    if (!_mavlinkStarted || !_connected) {
        return;
    }
    if (linkConfiguration()->isHighLatency() && _highLatencyTransmissionEnabled) {
        _sendHighLatency2();
        return;
    }
    _sendVibration();
    _sendBatteryStatus();
    _sendSysStatus();
    _sendADSBVehicles();
    _sendRemoteIDArmStatus();
    if (!qgcApp()->runningUnitTests()) {
        _sendRCChannels();
    }
    if (_sendHomePositionDelayCount > 0) {
        _sendHomePositionDelayCount--;
    } else {
        _sendHomePosition();
    }
}

void MockLink::run10HzTasks() {
    if (linkConfiguration()->isHighLatency()) {
        return;
    }
    if (_mavlinkStarted && _connected) {
        _sendHeartBeat();
        if (_sendGPSPositionDelayCount > 0) {
            _sendGPSPositionDelayCount--;
        } else {
            _sendGpsRawInt();
            _sendGlobalPositionInt();
            _sendExtendedSysState();
        }
    }
}

void MockLink::run500HzTasks() {
    if (linkConfiguration()->isHighLatency()) {
        return;
    }
    if (_mavlinkStarted && _connected) {
        _paramRequestListWorker();
        _logDownloadWorker();
    }
}

void MockLink::sendStatusTextMessages() {
    _sendStatusTextMessages();
}

// ----- End of worker-called methods -----

// The following methods remain unchanged from the original implementation:

void MockLink::_writeBytes(const QByteArray &bytes)
{
    emit writeBytesQueuedSignal(bytes);
}

void MockLink::_writeBytesQueued(const QByteArray &bytes)
{
    if (_inNSH) {
        _handleIncomingNSHBytes(bytes.constData(), bytes.length());
        return;
    }
    if (bytes.startsWith(QByteArrayLiteral("\r\r\r"))) {
        _inNSH = true;
        _handleIncomingNSHBytes(&bytes.constData()[3], bytes.length() - 3);
    }
    _handleIncomingMavlinkBytes(reinterpret_cast<const uint8_t*>(bytes.constData()), bytes.length());
}

void MockLink::_handleIncomingNSHBytes(const char *bytes, int cBytes)
{
    Q_UNUSED(cBytes);
    if ((cBytes == 4) && (bytes[0] == '\r') && (bytes[1] == '\r') && (bytes[2] == '\r')) {
        _inNSH  = false;
        return;
    }
    if (cBytes > 0) {
        qCDebug(MockLinkLog) << "NSH:" << bytes;
    }
}

void MockLink::_handleIncomingMavlinkBytes(const uint8_t *bytes, int cBytes)
{
    mavlink_message_t msg{};
    mavlink_status_t comm{};
    QMutexLocker lock(&_mavlinkAuxMutex);
    for (qint64 i = 0; i < cBytes; i++) {
        const int parsed = mavlink_parse_char(_mavlinkAuxChannel, bytes[i], &msg, &comm);
        if (!parsed) {
            continue;
        }
        lock.unlock();
        _handleIncomingMavlinkMsg(msg);
        lock.relock();
    }
}

void MockLink::_handleIncomingMavlinkMsg(const mavlink_message_t &msg)
{
    if (_missionItemHandler->handleMessage(msg)) {
        return;
    }
    switch (msg.msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT:
        _handleHeartBeat(msg);
        break;
    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
        _handleParamRequestList(msg);
        break;
    case MAVLINK_MSG_ID_SET_MODE:
        _handleSetMode(msg);
        break;
    case MAVLINK_MSG_ID_PARAM_SET:
        _handleParamSet(msg);
        break;
    case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
        _handleParamRequestRead(msg);
        break;
    case MAVLINK_MSG_ID_FILE_TRANSFER_PROTOCOL:
        _handleFTP(msg);
        break;
    case MAVLINK_MSG_ID_COMMAND_LONG:
        _handleCommandLong(msg);
        break;
    case MAVLINK_MSG_ID_MANUAL_CONTROL:
        _handleManualControl(msg);
        break;
    case MAVLINK_MSG_ID_LOG_REQUEST_LIST:
        _handleLogRequestList(msg);
        break;
    case MAVLINK_MSG_ID_LOG_REQUEST_DATA:
        _handleLogRequestData(msg);
        break;
    case MAVLINK_MSG_ID_PARAM_MAP_RC:
        _handleParamMapRC(msg);
        break;
    default:
        break;
    }
}

void MockLink::_handleHeartBeat(const mavlink_message_t &msg)
{
    Q_UNUSED(msg);
    qCDebug(MockLinkLog) << "Heartbeat";
}

void MockLink::_handleParamMapRC(const mavlink_message_t &msg)
{
    mavlink_param_map_rc_t paramMapRC{};
    mavlink_msg_param_map_rc_decode(&msg, &paramMapRC);
    const QString paramName(QString::fromLocal8Bit(paramMapRC.param_id, static_cast<int>(strnlen(paramMapRC.param_id, MAVLINK_MSG_PARAM_MAP_RC_FIELD_PARAM_ID_LEN))));
    if (paramMapRC.param_index == -1) {
        qCDebug(MockLinkLog) << QStringLiteral("MockLink - PARAM_MAP_RC: param(%1) tuningID(%2) centerValue(%3) scale(%4) min(%5) max(%6)")
                                .arg(paramName).arg(paramMapRC.parameter_rc_channel_index).arg(paramMapRC.param_value0)
                                .arg(paramMapRC.scale).arg(paramMapRC.param_value_min).arg(paramMapRC.param_value_max);
    } else if (paramMapRC.param_index == -2) {
        qCDebug(MockLinkLog) << "MockLink - PARAM_MAP_RC: Clear tuningID" << paramMapRC.parameter_rc_channel_index;
    } else {
        qCWarning(MockLinkLog) << "MockLink - PARAM_MAP_RC: Unsupported param_index" << paramMapRC.param_index;
    }
}

void MockLink::_handleSetMode(const mavlink_message_t &msg)
{
    mavlink_set_mode_t request{};
    mavlink_msg_set_mode_decode(&msg, &request);
    Q_ASSERT(request.target_system == _vehicleSystemId);
    _mavBaseMode = request.base_mode;
    _mavCustomMode = request.custom_mode;
}

void MockLink::_handleManualControl(const mavlink_message_t &msg)
{
    mavlink_manual_control_t manualControl{};
    mavlink_msg_manual_control_decode(&msg, &manualControl);
    qCDebug(MockLinkLog) << "MANUAL_CONTROL" << manualControl.x << manualControl.y << manualControl.z << manualControl.r;
}

void MockLink::_setParamFloatUnionIntoMap(int componentId, const QString &paramName, float paramFloat)
{
    Q_ASSERT(_mapParamName2Value.contains(componentId));
    Q_ASSERT(_mapParamName2Value[componentId].contains(paramName));
    Q_ASSERT(_mapParamName2MavParamType[componentId].contains(paramName));
    const MAV_PARAM_TYPE paramType = _mapParamName2MavParamType[componentId][paramName];
    QVariant paramVariant;
    mavlink_param_union_t valueUnion{};
    valueUnion.param_float = paramFloat;
    switch (paramType) {
    case MAV_PARAM_TYPE_REAL32:
        paramVariant = QVariant::fromValue(valueUnion.param_float);
        break;
    case MAV_PARAM_TYPE_UINT32:
        paramVariant = QVariant::fromValue(valueUnion.param_uint32);
        break;
    case MAV_PARAM_TYPE_INT32:
        paramVariant = QVariant::fromValue(valueUnion.param_int32);
        break;
    case MAV_PARAM_TYPE_UINT16:
        paramVariant = QVariant::fromValue(valueUnion.param_uint16);
        break;
    case MAV_PARAM_TYPE_INT16:
        paramVariant = QVariant::fromValue(valueUnion.param_int16);
        break;
    case MAV_PARAM_TYPE_UINT8:
        paramVariant = QVariant::fromValue(valueUnion.param_uint8);
        break;
    case MAV_PARAM_TYPE_INT8:
        paramVariant = QVariant::fromValue(valueUnion.param_int8);
        break;
    default:
        qCCritical(MockLinkLog) << "Invalid parameter type" << paramType;
        paramVariant = QVariant::fromValue(valueUnion.param_int32);
        break;
    }
    qCDebug(MockLinkLog) << "_setParamFloatUnionIntoMap" << paramName << paramVariant;
    _mapParamName2Value[componentId][paramName] = paramVariant;
}

float MockLink::_floatUnionForParam(int componentId, const QString &paramName)
{
    Q_ASSERT(_mapParamName2Value.contains(componentId));
    Q_ASSERT(_mapParamName2Value[componentId].contains(paramName));
    Q_ASSERT(_mapParamName2MavParamType[componentId].contains(paramName));
    const MAV_PARAM_TYPE paramType = _mapParamName2MavParamType[componentId][paramName];
    const QVariant paramVar = _mapParamName2Value[componentId][paramName];
    mavlink_param_union_t valueUnion{};
    switch (paramType) {
    case MAV_PARAM_TYPE_REAL32:
        valueUnion.param_float = paramVar.toFloat();
        break;
    case MAV_PARAM_TYPE_UINT32:
        if (_firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toUInt();
        } else {
            valueUnion.param_uint32 = paramVar.toUInt();
        }
        break;
    case MAV_PARAM_TYPE_INT32:
        if (_firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toInt();
        } else {
            valueUnion.param_int32 = paramVar.toInt();
        }
        break;
    case MAV_PARAM_TYPE_UINT16:
        if (_firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toUInt();
        } else {
            valueUnion.param_uint16 = paramVar.toUInt();
        }
        break;
    case MAV_PARAM_TYPE_INT16:
        if (_firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toInt();
        } else {
            valueUnion.param_int16 = paramVar.toInt();
        }
        break;
    case MAV_PARAM_TYPE_UINT8:
        if (_firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toUInt();
        } else {
            valueUnion.param_uint8 = paramVar.toUInt();
        }
        break;
    case MAV_PARAM_TYPE_INT8:
        if (_firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = static_cast<unsigned char>(paramVar.toChar().toLatin1());
        } else {
            valueUnion.param_int8 = static_cast<unsigned char>(paramVar.toChar().toLatin1());
        }
        break;
    default:
        if (_firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toInt();
        } else {
            valueUnion.param_int32 = paramVar.toInt();
        }
        qCCritical(MockLinkLog) << "Invalid parameter type" << paramType;
    }
    return valueUnion.param_float;
}

void MockLink::_handleParamRequestList(const mavlink_message_t &msg)
{
    if (_failureMode == MockConfiguration::FailParamNoReponseToRequestList) {
        return;
    }
    mavlink_param_request_list_t request{};
    mavlink_msg_param_request_list_decode(&msg, &request);
    Q_ASSERT(request.target_system == _vehicleSystemId);
    Q_ASSERT(request.target_component == MAV_COMP_ID_ALL);
    _currentParamRequestListComponentIndex = 0;
    _currentParamRequestListParamIndex = 0;
}

void MockLink::_paramRequestListWorker()
{
    if (_currentParamRequestListComponentIndex == -1) {
        return;
    }
    const int componentId = _mapParamName2Value.keys()[_currentParamRequestListComponentIndex];
    const int cParameters = _mapParamName2Value[componentId].count();
    const QString paramName = _mapParamName2Value[componentId].keys()[_currentParamRequestListParamIndex];
    if (((_failureMode == MockConfiguration::FailMissingParamOnInitialReqest) || (_failureMode == MockConfiguration::FailMissingParamOnAllRequests)) && (paramName == _failParam)) {
        qCDebug(MockLinkLog) << "Skipping param send:" << paramName;
    } else {
        char paramId[MAVLINK_MSG_ID_PARAM_VALUE_LEN]{};
        mavlink_message_t responseMsg{};
        Q_ASSERT(_mapParamName2Value[componentId].contains(paramName));
        Q_ASSERT(_mapParamName2MavParamType[componentId].contains(paramName));
        const MAV_PARAM_TYPE paramType = _mapParamName2MavParamType[componentId][paramName];
        Q_ASSERT(paramName.length() <= MAVLINK_MSG_ID_PARAM_VALUE_LEN);
        (void) strncpy(paramId, paramName.toLocal8Bit().constData(), MAVLINK_MSG_ID_PARAM_VALUE_LEN);
        qCDebug(MockLinkLog) << "Sending msg_param_value" << componentId << paramId << paramType << _mapParamName2Value[componentId][paramId];
        (void) mavlink_msg_param_value_pack_chan(
            _vehicleSystemId,
            componentId,
            mavlinkChannel(),
            &responseMsg,
            paramId,
            _floatUnionForParam(componentId, paramName),
            paramType,
            cParameters,
            _currentParamRequestListParamIndex
        );
        respondWithMavlinkMessage(responseMsg);
    }
    if (++_currentParamRequestListParamIndex >= cParameters) {
        if (++_currentParamRequestListComponentIndex >= _mapParamName2Value.keys().count()) {
            _currentParamRequestListComponentIndex = -1;
        } else {
            _currentParamRequestListParamIndex = 0;
        }
    }
}

void MockLink::_handleParamSet(const mavlink_message_t &msg)
{
    mavlink_param_set_t request{};
    mavlink_msg_param_set_decode(&msg, &request);
    Q_ASSERT(request.target_system == _vehicleSystemId);
    const int componentId = request.target_component;
    char paramId[MAVLINK_MSG_PARAM_SET_FIELD_PARAM_ID_LEN + 1]{};
    paramId[MAVLINK_MSG_PARAM_SET_FIELD_PARAM_ID_LEN] = 0;
    (void) strncpy(paramId, request.param_id, MAVLINK_MSG_PARAM_SET_FIELD_PARAM_ID_LEN);
    qCDebug(MockLinkLog) << "_handleParamSet" << componentId << paramId << request.param_type;
    Q_ASSERT(_mapParamName2Value.contains(componentId));
    Q_ASSERT(_mapParamName2MavParamType.contains(componentId));
    Q_ASSERT(_mapParamName2Value[componentId].contains(paramId));
    Q_ASSERT(request.param_type == _mapParamName2MavParamType[componentId][paramId]);
    _setParamFloatUnionIntoMap(componentId, paramId, request.param_value);
    mavlink_message_t responseMsg;
    mavlink_msg_param_value_pack_chan(
        _vehicleSystemId,
        componentId,
        mavlinkChannel(),
        &responseMsg,
        paramId,
        request.param_value,
        request.param_type,
        _mapParamName2Value[componentId].count(),
        _mapParamName2Value[componentId].keys().indexOf(paramId)
    );
    respondWithMavlinkMessage(responseMsg);
}

void MockLink::_handleParamRequestRead(const mavlink_message_t &msg)
{
    mavlink_message_t responseMsg{};
    mavlink_param_request_read_t request{};
    mavlink_msg_param_request_read_decode(&msg, &request);
    const QString paramName(QString::fromLocal8Bit(request.param_id, static_cast<int>(strnlen(request.param_id, MAVLINK_MSG_PARAM_REQUEST_READ_FIELD_PARAM_ID_LEN))));
    const int componentId = request.target_component;
    if ((request.target_component == MAV_COMP_ID_ALL) && (paramName == "_HASH_CHECK")) {
        mavlink_param_union_t valueUnion{};
        valueUnion.type = MAV_PARAM_TYPE_UINT32;
        valueUnion.param_uint32 = 0;
        (void) mavlink_msg_param_value_pack_chan(
            _vehicleSystemId,
            componentId,
            mavlinkChannel(),
            &responseMsg,
            request.param_id,
            valueUnion.param_float,
            MAV_PARAM_TYPE_UINT32,
            0,
            -1
        );
        respondWithMavlinkMessage(responseMsg);
        return;
    }
    Q_ASSERT(_mapParamName2Value.contains(componentId));
    char paramId[MAVLINK_MSG_PARAM_REQUEST_READ_FIELD_PARAM_ID_LEN + 1]{};
    paramId[0] = 0;
    Q_ASSERT(request.target_system == _vehicleSystemId);
    if (request.param_index == -1) {
        (void) strncpy(paramId, request.param_id, MAVLINK_MSG_PARAM_REQUEST_READ_FIELD_PARAM_ID_LEN);
    } else {
        Q_ASSERT(request.param_index >= 0 && request.param_index < _mapParamName2Value[componentId].count());
        const QString key = _mapParamName2Value[componentId].keys().at(request.param_index);
        Q_ASSERT(key.length() <= MAVLINK_MSG_PARAM_REQUEST_READ_FIELD_PARAM_ID_LEN);
        strcpy(paramId, key.toLocal8Bit().constData());
    }
    Q_ASSERT(_mapParamName2Value[componentId].contains(paramId));
    Q_ASSERT(_mapParamName2MavParamType[componentId].contains(paramId));
    if ((_failureMode == MockConfiguration::FailMissingParamOnAllRequests) && (strcmp(paramId, _failParam) == 0)) {
        qCDebug(MockLinkLog) << "Ignoring request read for " << _failParam;
        return;
    }
    (void) mavlink_msg_param_value_pack_chan(
        _vehicleSystemId,
        componentId,
        mavlinkChannel(),
        &responseMsg,
        paramId,
        _floatUnionForParam(componentId, paramId),
        _mapParamName2MavParamType[componentId][paramId],
        _mapParamName2Value[componentId].count(),
        _mapParamName2Value[componentId].keys().indexOf(paramId)
    );
    respondWithMavlinkMessage(responseMsg);
}

void MockLink::emitRemoteControlChannelRawChanged(int channel, uint16_t raw)
{
    uint16_t chanRaw[18]{};
    for (int i = 0; i < 18; i++) {
        chanRaw[i] = UINT16_MAX;
    }
    chanRaw[channel] = raw;
    mavlink_message_t responseMsg{};
    mavlink_msg_rc_channels_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &responseMsg,
        0,
        18,
        chanRaw[0],
        chanRaw[1],
        chanRaw[2],
        chanRaw[3],
        chanRaw[4],
        chanRaw[5],
        chanRaw[6],
        chanRaw[7],
        chanRaw[8],
        chanRaw[9],
        chanRaw[10],
        chanRaw[11],
        chanRaw[12],
        chanRaw[13],
        chanRaw[14],
        chanRaw[15],
        chanRaw[16],
        chanRaw[17],
        0
    );
    respondWithMavlinkMessage(responseMsg);
}

void MockLink::_handleFTP(const mavlink_message_t &msg)
{
    _mockLinkFTP->mavlinkMessageReceived(msg);
}

void MockLink::_handleInProgressCommandLong(const mavlink_command_long_t &request)
{
    uint8_t commandResult = MAV_RESULT_UNSUPPORTED;
    switch (request.command) {
    case MockLink::MAV_CMD_MOCKLINK_RESULT_IN_PROGRESS_ACCEPTED:
        commandResult = MAV_RESULT_ACCEPTED;
        break;
    case MockLink::MAV_CMD_MOCKLINK_RESULT_IN_PROGRESS_FAILED:
        commandResult = MAV_RESULT_FAILED;
        break;
    case MockLink::MAV_CMD_MOCKLINK_RESULT_IN_PROGRESS_NO_ACK:
        break;
    }
    mavlink_message_t commandAck{};
    mavlink_msg_command_ack_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &commandAck,
        request.command,
        MAV_RESULT_IN_PROGRESS,
        1,
        0,
        0,
        0
    );
    respondWithMavlinkMessage(commandAck);
    if (request.command != MockLink::MAV_CMD_MOCKLINK_RESULT_IN_PROGRESS_NO_ACK) {
        mavlink_msg_command_ack_pack_chan(
            _vehicleSystemId,
            _vehicleComponentId,
            mavlinkChannel(),
            &commandAck,
            request.command,
            commandResult,
            0,
            0,
            0,
            0
        );
        respondWithMavlinkMessage(commandAck);
    }
}

void MockLink::_handleCommandLong(const mavlink_message_t &msg)
{
    static bool firstCmdUser3 = true;
    static bool firstCmdUser4 = true;
    mavlink_command_long_t request{};
    mavlink_msg_command_long_decode(&msg, &request);
    _receivedMavCommandCountMap[static_cast<MAV_CMD>(request.command)]++;
    uint8_t commandResult = MAV_RESULT_UNSUPPORTED;
    switch (request.command) {
    case MAV_CMD_COMPONENT_ARM_DISARM:
        if (request.param1 == 0.0f) {
            _mavBaseMode &= ~MAV_MODE_FLAG_SAFETY_ARMED;
        } else {
            _mavBaseMode |= MAV_MODE_FLAG_SAFETY_ARMED;
        }
        commandResult = MAV_RESULT_ACCEPTED;
        break;
    case MAV_CMD_PREFLIGHT_CALIBRATION:
        _handlePreFlightCalibration(request);
        commandResult = MAV_RESULT_ACCEPTED;
        break;
    case MAV_CMD_DO_MOTOR_TEST:
        commandResult = MAV_RESULT_ACCEPTED;
        break;
    case MAV_CMD_CONTROL_HIGH_LATENCY:
        if (linkConfiguration()->isHighLatency()) {
            _highLatencyTransmissionEnabled = static_cast<int>(request.param1) != 0;
            emit highLatencyTransmissionEnabledChanged(_highLatencyTransmissionEnabled);
            commandResult = MAV_RESULT_ACCEPTED;
        } else {
            commandResult = MAV_RESULT_FAILED;
        }
        break;
    case MAV_CMD_PREFLIGHT_STORAGE:
        commandResult = MAV_RESULT_ACCEPTED;
        break;
    case MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES:
        commandResult = MAV_RESULT_ACCEPTED;
        _respondWithAutopilotVersion();
        break;
    case MAV_CMD_REQUEST_MESSAGE:
    {
        bool noAck = false;
        if (_handleRequestMessage(request, noAck)) {
            if (noAck) {
                return;
            }
            commandResult = MAV_RESULT_ACCEPTED;
        }
        break;
    }
    case MAV_CMD_NAV_TAKEOFF:
        _handleTakeoff(request);
        commandResult = MAV_RESULT_ACCEPTED;
        break;
    case MAV_CMD_MOCKLINK_ALWAYS_RESULT_ACCEPTED:
        commandResult = MAV_RESULT_ACCEPTED;
        break;
    case MAV_CMD_MOCKLINK_ALWAYS_RESULT_FAILED:
        commandResult = MAV_RESULT_FAILED;
        break;
    case MAV_CMD_MOCKLINK_SECOND_ATTEMPT_RESULT_ACCEPTED:
        if (firstCmdUser3) {
            firstCmdUser3 = false;
            return;
        } else {
            firstCmdUser3 = true;
            commandResult = MAV_RESULT_ACCEPTED;
        }
        break;
    case MAV_CMD_MOCKLINK_SECOND_ATTEMPT_RESULT_FAILED:
        if (firstCmdUser4) {
            firstCmdUser4 = false;
            return;
        } else {
            firstCmdUser4 = true;
            commandResult = MAV_RESULT_FAILED;
        }
        break;
    case MAV_CMD_MOCKLINK_NO_RESPONSE:
    case MAV_CMD_MOCKLINK_NO_RESPONSE_NO_RETRY:
        return;
    case MockLink::MAV_CMD_MOCKLINK_RESULT_IN_PROGRESS_ACCEPTED:
    case MockLink::MAV_CMD_MOCKLINK_RESULT_IN_PROGRESS_FAILED:
    case MockLink::MAV_CMD_MOCKLINK_RESULT_IN_PROGRESS_NO_ACK:
        _handleInProgressCommandLong(request);
        return;
    }
    mavlink_message_t commandAck{};
    mavlink_msg_command_ack_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &commandAck,
        request.command,
        commandResult,
        0,
        0,
        0,
        0
    );
    respondWithMavlinkMessage(commandAck);
}

void MockLink::sendUnexpectedCommandAck(MAV_CMD command, MAV_RESULT ackResult)
{
    mavlink_message_t commandAck{};
    mavlink_msg_command_ack_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &commandAck,
        command,
        ackResult,
        0,
        0,
        0,
        0
    );
    respondWithMavlinkMessage(commandAck);
}

void MockLink::_respondWithAutopilotVersion()
{
    uint32_t flightVersion = 0;
#ifndef QGC_NO_ARDUPILOT_DIALECT
    if (_firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
        if (_vehicleType == MAV_TYPE_FIXED_WING) {
            flightVersion |= 9 << (8*2);
        } else if (_vehicleType == MAV_TYPE_SUBMARINE ) {
            flightVersion |= 5 << (8*2);
        } else if (_vehicleType == MAV_TYPE_GROUND_ROVER ) {
            flightVersion |= 5 << (8*2);
        } else {
            flightVersion |= 6 << (8*2);
        }
        flightVersion |= 3 << (8*3);
        flightVersion |= 0 << (8*1);
        flightVersion |= FIRMWARE_VERSION_TYPE_DEV << (8*0);
    } else if (_firmwareType == MAV_AUTOPILOT_PX4) {
#endif
        flightVersion |= 1 << (8*3);
        flightVersion |= 4 << (8*2);
        flightVersion |= 1 << (8*1);
        flightVersion |= FIRMWARE_VERSION_TYPE_DEV << (8*0);
#ifndef QGC_NO_ARDUPILOT_DIALECT
    }
#endif
    const uint8_t customVersion[8]{};
    const uint64_t capabilities = MAV_PROTOCOL_CAPABILITY_MAVLINK2 |
                                  MAV_PROTOCOL_CAPABILITY_MISSION_FENCE |
                                  MAV_PROTOCOL_CAPABILITY_MISSION_RALLY |
                                  MAV_PROTOCOL_CAPABILITY_MISSION_INT |
                                  ((_firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) ? MAV_PROTOCOL_CAPABILITY_TERRAIN : 0);
    mavlink_message_t msg{};
    mavlink_msg_autopilot_version_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &msg,
        capabilities,
        flightVersion,
        0,
        0,
        0,
        reinterpret_cast<const uint8_t*>(&customVersion),
        reinterpret_cast<const uint8_t*>(&customVersion),
        reinterpret_cast<const uint8_t*>(&customVersion),
        _boardVendorId,
        _boardProductId,
        0,
        0
    );
    respondWithMavlinkMessage(msg);
}

void MockLink::_sendHomePosition()
{
    const float bogus[4]{};
    mavlink_message_t msg{};
    mavlink_msg_home_position_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &msg,
        static_cast<int32_t>(_vehicleLatitude * 1E7),
        static_cast<int32_t>(_vehicleLongitude * 1E7),
        static_cast<int32_t>(_defaultVehicleHomeAltitude * 1000),
        0.0f, 0.0f, 0.0f,
        &bogus[0],
        0.0f, 0.0f, 0.0f,
        0
    );
    respondWithMavlinkMessage(msg);
}

void MockLink::_sendGpsRawInt()
{
    static uint64_t timeTick = 0;
    mavlink_message_t msg{};
    mavlink_msg_gps_raw_int_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &msg,
        timeTick++,
        GPS_FIX_TYPE_3D_FIX,
        static_cast<int32_t>(_vehicleLatitude * 1E7),
        static_cast<int32_t>(_vehicleLongitude * 1E7),
        static_cast<int32_t>(_vehicleAltitudeAMSL * 1000),
        3 * 100,
        3 * 100,
        UINT16_MAX,
        UINT16_MAX,
        8,
        0, 0, 0, 0, 0, 65535
    );
    respondWithMavlinkMessage(msg);
}

void MockLink::_sendGlobalPositionInt()
{
    static uint64_t timeTick = 0;
    mavlink_message_t msg{};
    mavlink_msg_global_position_int_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &msg,
        timeTick++,
        static_cast<int32_t>(_vehicleLatitude * 1E7),
        static_cast<int32_t>(_vehicleLongitude * 1E7),
        static_cast<int32_t>(_vehicleAltitudeAMSL * 1000),
        static_cast<int32_t>((_vehicleAltitudeAMSL - _defaultVehicleHomeAltitude) * 1000),
        0, 0, 0,
        UINT16_MAX
    );
    respondWithMavlinkMessage(msg);
}

void MockLink::_sendExtendedSysState()
{
    mavlink_message_t msg{};
    mavlink_msg_extended_sys_state_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &msg,
        MAV_VTOL_STATE_UNDEFINED,
        (_vehicleAltitudeAMSL > _defaultVehicleHomeAltitude) ? MAV_LANDED_STATE_IN_AIR : MAV_LANDED_STATE_ON_GROUND
    );
    respondWithMavlinkMessage(msg);
}

void MockLink::_sendChunkedStatusText(uint16_t chunkId, bool missingChunks)
{
    constexpr int cChunks = 4;
    int num = 0;
    for (int i = 0; i < cChunks; i++) {
        if (missingChunks && (i & 1)) {
            continue;
        }
        int cBuf = MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN;
        char msgBuf[MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN]{};
        if (i == cChunks - 1) {
            cBuf /= 2;
        }
        for (int j = 0; j < cBuf - 1; j++) {
            msgBuf[j] = '0' + num++;
            if (num > 9) {
                num = 0;
            }
        }
        msgBuf[cBuf-1] = 'A' + i;
        mavlink_message_t msg{};
        mavlink_msg_statustext_pack_chan(
            _vehicleSystemId,
            _vehicleComponentId,
            mavlinkChannel(),
            &msg,
            MAV_SEVERITY_INFO,
            msgBuf,
            chunkId,
            i
        );
        respondWithMavlinkMessage(msg);
    }
}

void MockLink::_sendStatusTextMessages()
{
    struct StatusMessage {
        MAV_SEVERITY severity;
        const char *msg;
    };
    static constexpr struct StatusMessage rgMessages[] = {
        { MAV_SEVERITY_INFO,        "#Testing audio output" },
        { MAV_SEVERITY_EMERGENCY,   "Status text emergency" },
        { MAV_SEVERITY_ALERT,       "Status text alert" },
        { MAV_SEVERITY_CRITICAL,    "Status text critical" },
        { MAV_SEVERITY_ERROR,       "Status text error" },
        { MAV_SEVERITY_WARNING,     "Status text warning" },
        { MAV_SEVERITY_NOTICE,      "Status text notice" },
        { MAV_SEVERITY_INFO,        "Status text info" },
        { MAV_SEVERITY_DEBUG,       "Status text debug" },
    };
    mavlink_message_t msg{};
    for (size_t i = 0; i < std::size(rgMessages); i++) {
        const struct StatusMessage *status = &rgMessages[i];
        mavlink_msg_statustext_pack_chan(
            _vehicleSystemId,
            _vehicleComponentId,
            mavlinkChannel(),
            &msg,
            status->severity,
            status->msg,
            0,
            0
        );
        respondWithMavlinkMessage(msg);
    }
    _sendChunkedStatusText(1, false);
    _sendChunkedStatusText(2, true);
    _sendChunkedStatusText(3, false);
    _sendChunkedStatusText(4, true);
}

MockLink *MockLink::_startMockLinkWorker(const QString &configName, MAV_AUTOPILOT firmwareType, MAV_TYPE vehicleType, bool sendStatusText, MockConfiguration::FailureMode_t failureMode)
{
    MockConfiguration *const mockConfig = new MockConfiguration(configName);
    mockConfig->setFirmwareType(firmwareType);
    mockConfig->setVehicleType(vehicleType);
    mockConfig->setSendStatusText(sendStatusText);
    mockConfig->setFailureMode(failureMode);
    return _startMockLink(mockConfig);
}

MockLink *MockLink::_startMockLink(MockConfiguration *mockConfig)
{
    mockConfig->setDynamic(true);
    SharedLinkConfigurationPtr config = LinkManager::instance()->addConfiguration(mockConfig);
    if (LinkManager::instance()->createConnectedLink(config)) {
        return qobject_cast<MockLink*>(config->link());
    }
    return nullptr;
}

MockLink *MockLink::startPX4MockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode)
{
    return _startMockLinkWorker(QStringLiteral("PX4 MultiRotor MockLink"), MAV_AUTOPILOT_PX4, MAV_TYPE_QUADROTOR, sendStatusText, failureMode);
}

MockLink *MockLink::startGenericMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode)
{
    return _startMockLinkWorker(QStringLiteral("Generic MockLink"), MAV_AUTOPILOT_GENERIC, MAV_TYPE_QUADROTOR, sendStatusText, failureMode);
}

MockLink *MockLink::startNoInitialConnectMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode)
{
    return _startMockLinkWorker(QStringLiteral("No Initial Connect MockLink"), MAV_AUTOPILOT_PX4, MAV_TYPE_GENERIC, sendStatusText, failureMode);
}

MockLink *MockLink::startAPMArduCopterMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode)
{
    return _startMockLinkWorker(QStringLiteral("ArduCopter MockLink"), MAV_AUTOPILOT_ARDUPILOTMEGA, MAV_TYPE_QUADROTOR, sendStatusText, failureMode);
}

MockLink *MockLink::startAPMArduPlaneMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode)
{
    return _startMockLinkWorker(QStringLiteral("ArduPlane MockLink"), MAV_AUTOPILOT_ARDUPILOTMEGA, MAV_TYPE_FIXED_WING, sendStatusText, failureMode);
}

MockLink *MockLink::startAPMArduSubMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode)
{
    return _startMockLinkWorker(QStringLiteral("ArduSub MockLink"), MAV_AUTOPILOT_ARDUPILOTMEGA, MAV_TYPE_SUBMARINE, sendStatusText, failureMode);
}

MockLink *MockLink::startAPMArduRoverMockLink(bool sendStatusText, MockConfiguration::FailureMode_t failureMode)
{
    return _startMockLinkWorker(QStringLiteral("ArduRover MockLink"), MAV_AUTOPILOT_ARDUPILOTMEGA, MAV_TYPE_GROUND_ROVER, sendStatusText, failureMode);
}

void MockLink::_sendRCChannels()
{
    mavlink_message_t msg{};
    mavlink_msg_rc_channels_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &msg,
        0,
        16,
        1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500,
        1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500,
        UINT16_MAX, UINT16_MAX,
        0
    );
    respondWithMavlinkMessage(msg);
}

void MockLink::_handlePreFlightCalibration(const mavlink_command_long_t& request)
{
    static constexpr const char *gyroCalResponse = "[cal] calibration started: 2 gyro";
    static constexpr const char *magCalResponse = "[cal] calibration started: 2 mag";
    static constexpr const char *accelCalResponse = "[cal] calibration started: 2 accel";
    const char *pCalMessage;
    if (request.param1 == 1) {
        pCalMessage = gyroCalResponse;
    } else if (request.param2 == 1) {
        pCalMessage = magCalResponse;
    } else if (request.param5 == 1) {
        pCalMessage = accelCalResponse;
    } else {
        return;
    }
    mavlink_message_t msg{};
    mavlink_msg_statustext_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &msg,
        MAV_SEVERITY_INFO,
        pCalMessage,
        0,
        0
    );
    respondWithMavlinkMessage(msg);
}

void MockLink::_handleTakeoff(const mavlink_command_long_t &request)
{
    _vehicleAltitudeAMSL = request.param7 + _defaultVehicleHomeAltitude;
    _mavBaseMode |= MAV_MODE_FLAG_SAFETY_ARMED;
}

void MockLink::_handleLogRequestList(const mavlink_message_t &msg)
{
    mavlink_log_request_list_t request{};
    mavlink_msg_log_request_list_decode(&msg, &request);
    if ((request.start != 0) && (request.end != 0xffff)) {
        qCWarning(MockLinkLog) << "_handleLogRequestList cannot handle partial requests";
        return;
    }
    mavlink_message_t responseMsg{};
    mavlink_msg_log_entry_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &responseMsg,
        _logDownloadLogId,
        1,
        1,
        0,
        _logDownloadFileSize
    );
    respondWithMavlinkMessage(responseMsg);
}

QString MockLink::_createRandomFile(uint32_t byteCount)
{
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    if (!tempFile.open()) {
        qCWarning(MockLinkLog) << "MockLink::createRandomFile open failed" << tempFile.errorString();
        return QString();
    }
    for (uint32_t bytesWritten = 0; bytesWritten < byteCount; bytesWritten++) {
        const unsigned char byte = (QRandomGenerator::global()->generate() * 0xFF) / RAND_MAX;
        tempFile.write(reinterpret_cast<const char*>(&byte), 1);
    }
    tempFile.close();
    return tempFile.fileName();
}

void MockLink::_handleLogRequestData(const mavlink_message_t &msg)
{
    mavlink_log_request_data_t request{};
    mavlink_msg_log_request_data_decode(&msg, &request);
#ifdef QGC_UNITTEST_BUILD
    if (_logDownloadFilename.isEmpty()) {
        _logDownloadFilename = _createRandomFile(_logDownloadFileSize);
    }
#endif
    if (request.id != 0) {
        qCWarning(MockLinkLog) << "_handleLogRequestData id must be 0";
        return;
    }
    if (request.ofs > (_logDownloadFileSize - 1)) {
        qCWarning(MockLinkLog) << "_handleLogRequestData offset past end of file request.ofs:size" << request.ofs << _logDownloadFileSize;
        return;
    }
    _logDownloadCurrentOffset = request.ofs;
    if (request.ofs + request.count > _logDownloadFileSize) {
        request.count = _logDownloadFileSize - request.ofs;
    }
    _logDownloadBytesRemaining = request.count;
}

void MockLink::_logDownloadWorker()
{
    if (_logDownloadBytesRemaining == 0) {
        return;
    }
    QFile file(_logDownloadFilename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(MockLinkLog) << "_logDownloadWorker open failed" << file.errorString();
        return;
    }
    uint8_t buffer[MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN]{};
    const qint64 bytesToRead = qMin(_logDownloadBytesRemaining, (uint32_t)MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN);
    Q_ASSERT(file.seek(_logDownloadCurrentOffset));
    Q_ASSERT(file.read(reinterpret_cast<char*>(buffer), bytesToRead) == bytesToRead);
    qCDebug(MockLinkLog) << "_logDownloadWorker" << _logDownloadCurrentOffset << _logDownloadBytesRemaining;
    mavlink_message_t responseMsg{};
    mavlink_msg_log_data_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &responseMsg,
        _logDownloadLogId,
        _logDownloadCurrentOffset,
        bytesToRead,
        &buffer[0]
    );
    respondWithMavlinkMessage(responseMsg);
    _logDownloadCurrentOffset += bytesToRead;
    _logDownloadBytesRemaining -= bytesToRead;
    file.close();
}

void MockLink::_sendADSBVehicles()
{
    for (int i = 0; i < _adsbVehicles.size(); ++i) {
        _adsbVehicles[i].angle += (i + 1);
        _adsbVehicles[i].coordinate = _adsbVehicles[i].coordinate.atDistanceAndAzimuth(5, _adsbVehicles[i].angle);
        _adsbVehicles[i].altitude += (i % 2 == 0 ? 0.5 : -0.5);
        mavlink_message_t responseMsg{};
        mavlink_msg_adsb_vehicle_pack_chan(
            _vehicleSystemId,
            _vehicleComponentId,
            mavlinkChannel(),
            &responseMsg,
            12345 + i,
            _adsbVehicles[i].coordinate.latitude() * 1e7,
            _adsbVehicles[i].coordinate.longitude() * 1e7,
            ADSB_ALTITUDE_TYPE_GEOMETRIC,
            _adsbVehicles[i].altitude * 1000,
            static_cast<uint16_t>(_adsbVehicles[i].angle * 100),
            0, 0,
            QString("N12345%1").arg(i, 2, 10, QChar('0')).toStdString().c_str(),
            ADSB_EMITTER_TYPE_ROTOCRAFT,
            1,
            ADSB_FLAGS_VALID_COORDS | ADSB_FLAGS_VALID_ALTITUDE | ADSB_FLAGS_VALID_HEADING | ADSB_FLAGS_VALID_CALLSIGN | ADSB_FLAGS_SIMULATED,
            0
        );
        respondWithMavlinkMessage(responseMsg);
    }
}

void MockLink::_moveADSBVehicle(int vehicleIndex)
{
    _adsbAngles[vehicleIndex] += 10;
    QGeoCoordinate &coord = _adsbVehicleCoordinates[vehicleIndex];
    coord = QGeoCoordinate(coord.latitude(), coord.longitude()).atDistanceAndAzimuth(500, _adsbAngles[vehicleIndex]);
    coord.setAltitude(100);
}

bool MockLink::_handleRequestMessage(const mavlink_command_long_t &request, bool &noAck)
{
    noAck = false;
    switch (static_cast<int>(request.param1)) {
    case MAVLINK_MSG_ID_AUTOPILOT_VERSION:
    {
        switch (_failureMode) {
        case MockConfiguration::FailNone:
            break;
        case MockConfiguration::FailInitialConnectRequestMessageAutopilotVersionFailure:
            return false;
        case MockConfiguration::FailInitialConnectRequestMessageAutopilotVersionLost:
            return true;
        default:
            break;
        }
        _respondWithAutopilotVersion();
        return true;
    }
    case MAVLINK_MSG_ID_PROTOCOL_VERSION:
    {
        switch (_failureMode) {
        case MockConfiguration::FailNone:
            break;
        case MockConfiguration::FailInitialConnectRequestMessageProtocolVersionFailure:
            return false;
        case MockConfiguration::FailInitialConnectRequestMessageProtocolVersionLost:
            return true;
        default:
            break;
        }
        const uint8_t nullHash[8]{};
        mavlink_message_t responseMsg{};
        mavlink_msg_protocol_version_pack_chan(
            _vehicleSystemId,
            _vehicleComponentId,
            mavlinkChannel(),
            &responseMsg,
            200,
            100,
            200,
            nullHash,
            nullHash
        );
        respondWithMavlinkMessage(responseMsg);
        return true;
    }
    case MAVLINK_MSG_ID_COMPONENT_METADATA:
        if (_firmwareType == MAV_AUTOPILOT_PX4) {
            _sendGeneralMetaData();
            return true;
        }
        break;
    case MAVLINK_MSG_ID_DEBUG:
    {
        switch (_requestMessageFailureMode) {
        case FailRequestMessageNone:
            break;
        case FailRequestMessageCommandAcceptedMsgNotSent:
            return true;
        case FailRequestMessageCommandUnsupported:
            return false;
        case FailRequestMessageCommandNoResponse:
            noAck = true;
            return true;
        }
        mavlink_message_t responseMsg{};
        mavlink_msg_debug_pack_chan(
            _vehicleSystemId,
            _vehicleComponentId,
            mavlinkChannel(),
            &responseMsg,
            0, 0, 0
        );
        respondWithMavlinkMessage(responseMsg);
        return true;
    }
    }
    return false;
}

void MockLink::_sendGeneralMetaData()
{
    static constexpr const char metaDataURI[MAVLINK_MSG_COMPONENT_METADATA_FIELD_URI_LEN] = "mftp://[;comp=1]general.json";
    mavlink_message_t responseMsg{};
    mavlink_msg_component_metadata_pack_chan(
        _vehicleSystemId,
        _vehicleComponentId,
        mavlinkChannel(),
        &responseMsg,
        0,
        100,
        metaDataURI
    );
    respondWithMavlinkMessage(responseMsg);
}

void MockLink::_sendRemoteIDArmStatus()
{
    mavlink_message_t msg{};
    mavlink_msg_open_drone_id_arm_status_pack(
        _vehicleSystemId,
        MAV_COMP_ID_ODID_TXRX_1,
        &msg,
        MAV_ODID_ARM_STATUS_GOOD_TO_ARM,
        QStringLiteral("No Error").toStdString().c_str()
    );
    respondWithMavlinkMessage(msg);
}

void MockLink::simulateConnectionRemoved()
{
    _commLost = true;
    // _connectionRemoved() would be called here if defined.
}

MockLinkFTP *MockLink::mockLinkFTP() const
{
    return _mockLinkFTP;
}
