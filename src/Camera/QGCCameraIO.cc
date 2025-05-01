/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCCameraIO.h"
#include "LinkInterface.h"
#include "MavlinkCameraControl.h"
#include "MAVLinkProtocol.h"
#include "QGCMAVLink.h"
#include "QGCLoggingCategory.h"
#include "Vehicle.h"

#include <QtQml/QQmlEngine>

QGC_LOGGING_CATEGORY(CameraIOLog, "qgc.camera.qgccameraio")
QGC_LOGGING_CATEGORY(CameraIOLogVerbose, "qgc.camera.qgccameraio:verbose")

QGCCameraParamIO::QGCCameraParamIO(MavlinkCameraControl *control, Fact *fact, Vehicle *vehicle)
    : QObject(control)
    , _control(control)
    , _fact(fact)
    , _vehicle(vehicle)
{
    _paramWriteTimer.setSingleShot(true);
    _paramWriteTimer.setInterval(3000);
    _paramRequestTimer.setSingleShot(true);
    _paramRequestTimer.setInterval(3500);

    if (_fact->writeOnly()) {
        // Write mode is always "done" as it won't ever read
        _done = true;
    } else {
        (void) connect(&_paramRequestTimer, &QTimer::timeout, this, &QGCCameraParamIO::_paramRequestTimeout);
    }

    (void) connect(&_paramWriteTimer, &QTimer::timeout, this, &QGCCameraParamIO::_paramWriteTimeout);
    (void) connect(_fact, &Fact::rawValueChanged, this, &QGCCameraParamIO::_factChanged);
    (void) connect(_fact, &Fact::containerRawValueChanged, this, &QGCCameraParamIO::_containerRawValueChanged);

    _mavParamType = QGCMAVLink::valueFromParamExt(_fact->type());
}

void QGCCameraParamIO::setParamRequest()
{
    if (!_fact->writeOnly()) {
        _paramRequestReceived = false;
        _requestRetries = 0;
        _paramRequestTimer.start();
    }
}

void QGCCameraParamIO::_factChanged(const QVariant &value)
{
    Q_UNUSED(value);

    if (!_forceUIUpdate) {
        qCDebug(CameraIOLog) << "UI Fact" << _fact->name() << "changed to" << value;
        _control->factChanged(_fact);
    }
}

void QGCCameraParamIO::_containerRawValueChanged(const QVariant &value)
{
    Q_UNUSED(value);

    if (!_fact->readOnly()) {
        qCDebug(CameraIOLog) << "Update Fact from camera" << _fact->name();
        _sentRetries = 0;
        _sendParameter();
    }
}

void QGCCameraParamIO::sendParameter(bool updateUI)
{
    qCDebug(CameraIOLog) << "Send Fact" << _fact->name();
    _sentRetries = 0;
    _updateOnSet = updateUI;
    _sendParameter();
}

void QGCCameraParamIO::_sendParameter()
{
    SharedLinkInterfacePtr sharedLink = _vehicle->vehicleLinkManager()->primaryLink().lock();
    if (sharedLink) {
        mavlink_param_ext_set_t param{};
        (void) memset(&param, 0, sizeof(mavlink_param_ext_set_t));
        param.param_type = _mavParamType;

        param_ext_union_t union_value{};
        const FactMetaData::ValueType_t factType = _fact->type();
        switch (factType) {
        case FactMetaData::valueTypeUint8:
        case FactMetaData::valueTypeBool:
            union_value.param_uint8 = static_cast<uint8_t>(_fact->rawValue().toUInt());
            break;
        case FactMetaData::valueTypeInt8:
            union_value.param_int8 = static_cast<int8_t>(_fact->rawValue().toInt());
            break;
        case FactMetaData::valueTypeUint16:
            union_value.param_uint16 = static_cast<uint16_t>(_fact->rawValue().toUInt());
            break;
        case FactMetaData::valueTypeInt16:
            union_value.param_int16 = static_cast<int16_t>(_fact->rawValue().toInt());
            break;
        case FactMetaData::valueTypeUint32:
            union_value.param_uint32 = static_cast<uint32_t>(_fact->rawValue().toUInt());
            break;
        case FactMetaData::valueTypeInt64:
            union_value.param_int64 = static_cast<int64_t>(_fact->rawValue().toLongLong());
            break;
        case FactMetaData::valueTypeUint64:
            union_value.param_uint64 = static_cast<uint64_t>(_fact->rawValue().toULongLong());
            break;
        case FactMetaData::valueTypeFloat:
            union_value.param_float = _fact->rawValue().toFloat();
            break;
        case FactMetaData::valueTypeDouble:
            union_value.param_double = _fact->rawValue().toDouble();
            break;
        case FactMetaData::valueTypeString:
            // String and custom are the same for now
        case FactMetaData::valueTypeCustom: {
            const QByteArray custom = _fact->rawValue().toByteArray();
            (void) memcpy(union_value.bytes, custom.data(), static_cast<size_t>(std::max(custom.size(), static_cast<qsizetype>(MAVLINK_MSG_PARAM_EXT_SET_FIELD_PARAM_VALUE_LEN))));
            break;
        }
        default:
            qCCritical(CameraIOLog) << "Unsupported fact type" << factType << "for" << _fact->name();
        case FactMetaData::valueTypeInt32:
            union_value.param_int32 = static_cast<int32_t>(_fact->rawValue().toInt());
            break;
        }

        (void) memcpy(&param.param_value[0], &union_value.bytes[0], MAVLINK_MSG_PARAM_EXT_SET_FIELD_PARAM_VALUE_LEN);
        param.target_system = static_cast<uint8_t>(_vehicle->id());
        param.target_component = static_cast<uint8_t>(_control->compID());
        (void) strncpy(p.param_id, _fact->name().toStdString().c_str(), MAVLINK_MSG_PARAM_EXT_SET_FIELD_PARAM_ID_LEN);

        mavlink_message_t msg{};
        mavlink_msg_param_ext_set_encode_chan(
            static_cast<uint8_t>(MAVLinkProtocol::instance()->getSystemId()),
            static_cast<uint8_t>(MAVLinkProtocol::getComponentId()),
            sharedLink->mavlinkChannel(),
            &msg,
            &p
        );
        (void) _vehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), msg);
    }
    _paramWriteTimer.start();
}

void QGCCameraParamIO::_paramWriteTimeout()
{
    if (++_sentRetries > 3) {
        qCWarning(CameraIOLog) << "No response for param set:" << _fact->name();
        _updateOnSet = false;
    } else {
        // Send it again
        qCDebug(CameraIOLog) << "Param set retry:" << _fact->name() << _sentRetries;
        _sendParameter();
        _paramWriteTimer.start();
    }
}

void QGCCameraParamIO::handleParamAck(const mavlink_param_ext_ack_t &ack)
{
    _paramWriteTimer.stop();
    switch (ack.param_result) {
    case PARAM_ACK_ACCEPTED: {
        const QVariant val = QGCMAVLink::valueFromParamExt(ack.param_value, ack.param_type);
        if (_fact->rawValue() != val) {
            _fact->containerSetRawValue(val);
            if (_updateOnSet) {
                _updateOnSet = false;
                _control->factChanged(_fact);
            }
        }
        break;
    }
    case PARAM_ACK_IN_PROGRESS:
        // Wait a bit longer for this one
        qCDebug(CameraIOLogVerbose) << "Param set in progress:" << _fact->name();
        _paramWriteTimer.start();
        break;
    case PARAM_ACK_FAILED:
        if (++_sentRetries < 3) {
            // Try again
            qCWarning(CameraIOLog) << "Param set failed:" << _fact->name() << _sentRetries;
            _paramWriteTimer.start();
        }
        return;
    case PARAM_ACK_VALUE_UNSUPPORTED:
        qCWarning(CameraIOLog) << "Param set unsuported:" << _fact->name();
        // If UI changed and value was not set, restore UI
        const QVariant val = QGCMAVLink::valueFromParamExt(ack.param_value, ack.param_type);
        if (_fact->rawValue() != val) {
            if (_control->validateParameter(_fact, val)) {
                _fact->containerSetRawValue(val);
            }
        }
        break;
    default:
        qCCritical(CameraIOLog) << "Unsupported Param Ack Type" << ack.param_result;
        break;
    }
}

void QGCCameraParamIO::handleParamValue(const mavlink_param_ext_value_t &value)
{
    _paramRequestTimer.stop();
    const QVariant newValue = QGCMAVLink::valueFromParamExt(value.param_value, value.param_type);
    if (_control->incomingParameter(_fact, newValue)) {
        _fact->containerSetRawValue(newValue);
        _control->factChanged(_fact);
    }

    _paramRequestReceived = true;
    if (_forceUIUpdate) {
        emit _fact->rawValueChanged(_fact->rawValue());
        emit _fact->valueChanged(_fact->rawValue());
        _forceUIUpdate = false;
    }

    if (!_done) {
        _done = true;
        _control->_paramDone();
    }

    qCDebug(CameraIOLog) << _fact->name() << _fact->rawValueString();
}

void QGCCameraParamIO::_paramRequestTimeout()
{
    if (++_requestRetries > 3) {
        qCWarning(CameraIOLog) << "No response for param request:" << _fact->name();
        if (!_done) {
            _done = true;
            _control->_paramDone();
        }
    } else {
        // Request it again
        qCDebug(CameraIOLog) << "Param request retry:" << _fact->name();
        paramRequest(false);
        _paramRequestTimer.start();
    }
}

void QGCCameraParamIO::paramRequest(bool reset)
{
    // If it's write only, we don't request it.
    if (_fact->writeOnly()) {
        if (!_done) {
            _done = true;
            _control->_paramDone();
        }
        return;
    }

    if (reset) {
        _requestRetries = 0;
        _forceUIUpdate = true;
    }
    qCDebug(CameraIOLog) << "Request parameter:" << _fact->name();

    SharedLinkInterfacePtr sharedLink = _vehicle->vehicleLinkManager()->primaryLink().lock();
    if (sharedLink) {
        char param_id[MAVLINK_MSG_PARAM_EXT_REQUEST_READ_FIELD_PARAM_ID_LEN + 1]{};
        (void) memset(param_id, 0, sizeof(param_id));
        (void) strncpy(param_id, _fact->name().toStdString().c_str(), MAVLINK_MSG_PARAM_EXT_REQUEST_READ_FIELD_PARAM_ID_LEN);
        mavlink_message_t msg{};
        (void) mavlink_msg_param_ext_request_read_pack_chan(
            static_cast<uint8_t>(MAVLinkProtocol::instance()->getSystemId()),
            static_cast<uint8_t>(MAVLinkProtocol::getComponentId()),
            sharedLink->mavlinkChannel(),
            &msg,
            static_cast<uint8_t>(_vehicle->id()),
            static_cast<uint8_t>(_control->compID()),
            param_id,
            -1
        );
        (void) _vehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), msg);
    }
    _paramRequestTimer.start();
}
