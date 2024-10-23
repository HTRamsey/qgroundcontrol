#include "Viewpro.h"
#include "ViewproFactGroup.h"
#include "QGCApplication.h"
#include "QGCToolbox.h"
#include "MultiVehicleManager.h"
#include "Vehicle.h"
#include "LinkInterface.h"
#include "MAVLinkProtocol.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QtMath>
#include <QtCore/QtMinMax>
#include <QtCore/QLoggingCategory>

QGC_LOGGING_CATEGORY(ViewproLog, "qgc.custom.drone.viewpro")

Viewpro::Viewpro(ViewproFactGroup *viewproFactGroup, QObject *parent)
    : QObject(parent)
    , _viewproFactGroup(viewproFactGroup)
{
    // qCDebug(ViewproLog) << Q_FUNC_INFO << this;
}

Viewpro::~Viewpro()
{
    // qCDebug(ViewproLog) << Q_FUNC_INFO << this;
}

uint8_t Viewpro::s_viewlink_protocol_checksum(QByteArrayView viewlink_data_buf)
{
    const uint8_t len = viewlink_data_buf[3];
    uint8_t checksum = len;
    for(uint16_t i=0; i < len-2; i++)
    {
        checksum ^= viewlink_data_buf.at(4+i);
    }
    return checksum;
}

void Viewpro::_sendSerialCmd(const QByteArray &data)
{
    MultiVehicleManager *const multiVehicleManager = qgcApp()->toolbox()->multiVehicleManager();
    Vehicle *const vehicle = multiVehicleManager->activeVehicle();

    if (!vehicle) {
        qWarning() << "Internal error";
        return;
    }

    SharedLinkInterfacePtr sharedLink = vehicle->vehicleLinkManager()->primaryLink().lock();
    if (!sharedLink) {
        return;
    }

    QByteArray outputData = QByteArray(data);
    // Send maximum sized chunks until the complete buffer is transmitted
    while(outputData.size()) {
        QByteArray chunk{outputData.left(MAVLINK_MSG_SERIAL_CONTROL_FIELD_DATA_LEN)};
        int dataSize = chunk.size();
        // Ensure the buffer is large enough, as the MAVLink parser expects MAVLINK_MSG_SERIAL_CONTROL_FIELD_DATA_LEN bytes
        chunk.append(MAVLINK_MSG_SERIAL_CONTROL_FIELD_DATA_LEN - chunk.size(), '\0');
        uint8_t flags = SERIAL_CONTROL_FLAG_EXCLUSIVE; // | SERIAL_CONTROL_FLAG_BLOCKING;
        auto protocol = qgcApp()->toolbox()->mavlinkProtocol();
        auto link = vehicle->vehicleLinkManager()->primaryLink();
        mavlink_message_t msg;
        mavlink_msg_serial_control_pack_chan(
                    protocol->getSystemId(),
                    protocol->getComponentId(),
                    sharedLink->mavlinkChannel(),
                    &msg,
                    SERIAL_CONTROL_DEV_TELEM2,
                    flags,
                    0,
                    0,
                    dataSize,
                    reinterpret_cast<uint8_t*>(chunk.data()),
                    vehicle->id(), vehicle->defaultComponentId());
        vehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), msg);
        outputData.remove(0, chunk.size());
    }
}

/* U Communication Configuration Control */

void Viewpro::sendUCmd(uint8_t cmd, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint8_t param5, uint8_t param6, uint8_t param7, uint8_t param8, uint8_t param9)
{
    QByteArray packet(HEADER_LEN + BODY_LEN + ID_LEN + U_DATA_LEN + CHECKSUM_LEN, 0x00);
    packet[0] = static_cast<char>(0x55);
    packet[1] = static_cast<char>(0xAA);
    packet[2] = static_cast<char>(0xDC);
    packet[3] = static_cast<char>((BODY_LEN + ID_LEN + U_DATA_LEN + CHECKSUM_LEN) & 0x3F);
    packet[4] = static_cast<char>(0x01);
    packet[5] = static_cast<char>(cmd);
    packet[6] = static_cast<char>(param1);
    packet[7] = static_cast<char>(param2);
    packet[8] = static_cast<char>(param3);
    packet[9] = static_cast<char>(param4);
    packet[10] = static_cast<char>(param5);
    packet[11] = static_cast<char>(param6);
    packet[12] = static_cast<char>(param7);
    packet[13] = static_cast<char>(param8);
    packet[14] = static_cast<char>(param9);
    packet[15] = static_cast<char>(s_viewlink_protocol_checksum(packet));
    _sendSerialCmd(packet);
}

void Viewpro::setProtocolControlSerial(uint8_t LRF, uint8_t NMEA, uint8_t gimbal3E, uint8_t tracking7E, uint8_t sonyVISCA, uint8_t targetPositionCalc, uint8_t mavlink, uint8_t pelco_d, uint8_t feedback)
{ sendUCmd(0x02, LRF, NMEA, gimbal3E, tracking7E, sonyVISCA, targetPositionCalc, mavlink, pelco_d, feedback); }

void Viewpro::setTimeZoneSerial(int8_t hour, int8_t minute) { sendUCmd(0x04, hour, minute); }

void Viewpro::setOSD1Serial(bool on) { sendUCmd(0x05, on ? 0b10000000 : 0b11111111); }

void Viewpro::setOSD2Serial(bool on) { sendUCmd(0x07, on ? 0b10000001 : 0b10110111); }

void Viewpro::setBaudRateSerial() { sendUCmd(0x08, 0x06); } // 115200

void Viewpro::setEODigitalZoomSerial(bool on) { sendUCmd(0x09, on); }

void Viewpro::setEnableTrackingSerial(bool on) { sendUCmd(0x0C, on); }

void Viewpro::sendOutputControlSerial(bool mav_serial, bool mav_udp, bool track_serial, bool reset_ip)
{ sendUCmd(0x0C, (0x00 | (mav_serial ? 0x04 : 0x00) | (mav_udp ? 0x03 : 0x00) | (track_serial ? 0x02 : 0x00) | (reset_ip ? 0x01 : 0x00))); }

void Viewpro::setTrackModeSerial(bool highSpeed) { sendUCmd(0x2d, highSpeed); }

void Viewpro::setFlipImageSerial(bool irOn, bool eoOn) { sendUCmd(0x2e, (0b00000000 | (eoOn ? 0b10 : 0b01) | (irOn ? 0b10000 : 0b1000))); }

void Viewpro::setMavlinkSysIDSerial(int sysID) { sendUCmd(0x32, sysID); }


/* A1 Servo Control Commonly Used */

void Viewpro::sendA1Cmd(uint8_t control, uint16_t param1, uint16_t param2, uint16_t param3, uint16_t param4)
{
    QByteArray packet(HEADER_LEN + BODY_LEN + ID_LEN + A1_DATA_LEN + CHECKSUM_LEN, 0x00);
    packet[0] = static_cast<char>(0x55);
    packet[1] = static_cast<char>(0xAA);
    packet[2] = static_cast<char>(0xDC);
    packet[3] = static_cast<char>((BODY_LEN + ID_LEN + A1_DATA_LEN + CHECKSUM_LEN) & 0x3F);
    packet[4] = static_cast<char>(0x1A);
    packet[5] = static_cast<char>(control & 0xff);
    packet[6] = static_cast<char>((param1 & 0xff00) >> 8);
    packet[7] = static_cast<char>(param1 & 0xff);
    packet[8] = static_cast<char>((param2 & 0xff00) >> 8);
    packet[9] = static_cast<char>(param2 & 0xff);
    packet[10] = static_cast<char>((param3 & 0xff00) >> 8);
    packet[11] = static_cast<char>(param3 & 0xff);
    packet[12] = static_cast<char>((param4 & 0xff00) >> 8);
    packet[13] = static_cast<char>(param4 & 0xff);
    packet[14] = static_cast<char>(s_viewlink_protocol_checksum(packet));
    _sendSerialCmd(packet);
}

void Viewpro::gimbalMotorSerial(bool on) { sendA1Cmd(0x00, on ? 0x0100 : 0x0001); }

void Viewpro::gimbalManualSpeedSerial(float yaw, float tilt)
{
    static float last_yaw = 0;
    static float last_tilt = 0;
    const int scaled_yaw = (((yaw - (-1)) / (1 - (-1))) * (60 - 0)) + 0;
    const int scaled_tilt = (((tilt - (-1)) / (1 - (-1))) * (60 - 0)) + 0;
    if (scaled_yaw != last_yaw || scaled_tilt != last_tilt)
    {
        sendA1Cmd(0x01, scaled_yaw, scaled_tilt);
        last_yaw = yaw;
        last_tilt = tilt;
    }
}

void Viewpro::gimbalFollowYawSerial() { sendA1Cmd(0x03, 0x0000); }

void Viewpro::gimbalHomeSerial() { sendA1Cmd(0x04, 0x0000); }

void Viewpro::trackingModeSerial() { sendA1Cmd(0x06, 0x0000); }

// void Viewpro::gimbalManualAngleRelative() { sendA1Cmd(0x09, }

void Viewpro::gimbalFollowYawDisableSerial() { sendA1Cmd(0x0A, 0x0000); }

// void Viewpro::gimbalManualAngleAbsolute() { sendA1Cmd(0x0B, }

void Viewpro::gimbalManualRCSerial(float yaw, float tilt)
{
    static float last_yaw = 0;
    static float last_tilt = 0;
    if (yaw != last_yaw || tilt != last_tilt)
    {
        uint16_t yaw_angle;
        if(yaw < 0)
        {
            yaw_angle = 1100;
        }
        else if(yaw > 0)
        {
            yaw_angle = 1900;
        }
        else
        {
            yaw_angle = 1500;
        }
        uint16_t tilt_angle;
        if(tilt < 0)
        {
            tilt_angle = 1900;
        }
        else if(tilt > 0)
        {
            tilt_angle = 1100;
        }
        else
        {
            tilt_angle = 1500;
        }
        uint16_t yaw_speed = (std::abs(yaw) * 32768);
        yaw_speed /= (_viewproFactGroup->zoom() * 3);
        yaw_speed = qBound(0U, yaw_speed, 32768U);
        uint16_t tilt_speed = (std::abs(tilt) * 32768);
        tilt_speed /= (_viewproFactGroup->zoom() * 3);
        tilt_speed = qBound(0U, tilt_speed, 32768U);
        sendA1Cmd(0x0D, yaw_speed, yaw_angle, tilt_speed, tilt_angle);
        last_yaw = yaw;
        last_tilt = tilt;
    }
}

void Viewpro::gimbalLookDownSerial() { sendA1Cmd(0x12, 0x0000); }


/* C1 Optical Control Commonly Used */

uint16_t Viewpro::s_makeC1SerialData(uint8_t LRF, uint8_t cmd, uint8_t speed, uint8_t sensor)
{
    uint16_t result = 0;
    result |= ((LRF & 0x7) << 13);
    result |= ((cmd & 0x7F) << 6);
    result |= ((speed & 0x7) << 3);
    result |= ((sensor & 0x7));
    return result;
}

void Viewpro::sendC1Cmd(uint8_t cmd, uint8_t speed, uint8_t sensor, uint8_t LRF=0U)
{
    QByteArray packet(HEADER_LEN + BODY_LEN + ID_LEN + C1_DATA_LEN + CHECKSUM_LEN, 0x00);
    const uint16_t data = s_makeC1SerialData(LRF, cmd, speed, sensor);
    packet[0] = static_cast<char>(0x55);
    packet[1] = static_cast<char>(0xAA);
    packet[2] = static_cast<char>(0xDC);
    packet[3] = static_cast<char>((BODY_LEN + ID_LEN + C1_DATA_LEN + CHECKSUM_LEN) & 0x3F);
    packet[4] = static_cast<char>(0x1C);
    packet[5] = static_cast<char>(data >> 8);
    packet[6] = static_cast<char>(data & 0xff);
    packet[7] = static_cast<char>(s_viewlink_protocol_checksum(packet));
    _sendSerialCmd(packet);
}

void Viewpro::setVideoSourceEO1Serial() { sendC1Cmd(0, 0, 1, 0); }

void Viewpro::setVideoSourceIRSerial() { sendC1Cmd(0, 0, 2, 0); }

void Viewpro::setVideoSourceEO1IRPIPSerial() { sendC1Cmd(0, 0, 3, 0); }

void Viewpro::setVideoSourceIREO1PIPSerial() { sendC1Cmd(0, 0, 4, 0); }

void Viewpro::focusZoomStopSerial() { sendC1Cmd(0x01, 0, _viewproFactGroup->pipMode(), 0); }

// void Viewpro::brightnessUpSerial() { sendC1Cmd(0x02, 0, Viewpro::viewprodata()->pipMode(), 0); }

// void Viewpro::brightnessDownSerial() { sendC1Cmd(0x03, 0, Viewpro::viewprodata()->pipMode(), 0); }

void Viewpro::zoomOutSerial(int speed) { sendC1Cmd(0x08, static_cast<uint8_t>(speed), _viewproFactGroup->pipMode(), 0); }

void Viewpro::zoomInSerial(int speed) { sendC1Cmd(0x09, static_cast<uint8_t>(speed), _viewproFactGroup->pipMode(), 0); }

void Viewpro::focusInSerial() { sendC1Cmd(0x0A, 0, _viewproFactGroup->pipMode(), 0); }

void Viewpro::focusOutSerial() { sendC1Cmd(0x0B, 0, _viewproFactGroup->pipMode(), 0); }

// void Viewpro::setIRModePseudoSerial() { sendC1Cmd(0x0D, 0, Viewpro::viewprodata()->pipMode(), 0); }

void Viewpro::setIRModeWhiteHotSerial() { sendC1Cmd(0x0E, 0, _viewproFactGroup->pipMode(), 0); }

void Viewpro::setIRModeBlackHotSerial() { sendC1Cmd(0x0F, 0, _viewproFactGroup->pipMode(), 0); }

void Viewpro::rainbowIRSerial() { sendC1Cmd(0x12, 0, _viewproFactGroup->pipMode(), 0); }

// void Viewpro::takePictureSerial() { sendC1Cmd(0x13, 0, Viewpro::viewprodata()->pipMode(), 0); }

// void Viewpro::startRecordSerial() { sendC1Cmd(0x14, 0, Viewpro::viewprodata()->pipMode(), 0); }

// void Viewpro::stopRecordSerial() { sendC1Cmd(0x15, 0, Viewpro::viewprodata()->pipMode(), 0); }

// void Viewpro::pictureModeSerial() { sendC1Cmd(0x16, 0, Viewpro::viewprodata()->pipMode(), 0); }

// void Viewpro::recordModeSerial() { sendC1Cmd(0x17, 0, Viewpro::viewprodata()->pipMode(), 0); }

// void Viewpro::switchModeSerial() { sendC1Cmd(0x18, 0, Viewpro::viewprodata()->pipMode(), 0); }

void Viewpro::setAutoFocusSerial(bool enabled) { sendC1Cmd((enabled ? 0x19 : 0x1A), 0, _viewproFactGroup->pipMode(), 0); }

void Viewpro::zoomInIRDigitalSerial() { sendC1Cmd(0x1B, 0, _viewproFactGroup->pipMode(), 0); }

void Viewpro::zoomOutIRDigitalSerial() { sendC1Cmd(0x1C, 0, _viewproFactGroup->pipMode(), 0); }

void Viewpro::setIRColorExtSerial(int value) { if(value >= 1 && value <= 6) sendC1Cmd(0x20 | value, 0, _viewproFactGroup->pipMode(), 0); }

/* C2 Optical Control Infrequently Used */

void Viewpro::sendC2Cmd(uint8_t cmd, uint16_t param)
{
    QByteArray packet(HEADER_LEN + BODY_LEN + ID_LEN + C2_DATA_LEN + CHECKSUM_LEN, 0x00);
    packet[0] = static_cast<char>(0x55);
    packet[1] = static_cast<char>(0xAA);
    packet[2] = static_cast<char>(0xDC);
    packet[3] = static_cast<char>((BODY_LEN + ID_LEN + C2_DATA_LEN + CHECKSUM_LEN) & 0x3F);
    packet[4] = static_cast<char>(0x2C);
    packet[5] = static_cast<char>(cmd);
    packet[6] = static_cast<char>(param >> 8);
    packet[7] = static_cast<char>(param & 0xff);
    packet[8] = static_cast<char>(s_viewlink_protocol_checksum(packet));
    _sendSerialCmd(packet);
}

void Viewpro::digitalZoomEOSerial(bool on) { sendC2Cmd(on ? 0x06 : 0x07, 0); }

void Viewpro::digitalZoomEOMaxSerial(int level) { sendC2Cmd(0x08, level * 10); }

void Viewpro::imageEnhanceSerial(bool on) { sendC2Cmd(on ? 0x10 : 0x11, 0); }

void Viewpro::colorBarIRSerial(bool on) { sendC2Cmd(on ? 0x12 : 0x13, 0); }

/* TODO: Doesn't Work */
void Viewpro::imageFlipEOSerial(bool on) { sendC2Cmd(0x15, on); }

void Viewpro::setDefogSerial(bool on) { sendC2Cmd(on ? 0x17 : 0x16, 0); }

void Viewpro::imageFlipIRSerial(bool on) { sendC2Cmd(on ? 0x1B : 0x1A, 0); }

void Viewpro::nearIRSerial(bool on) { sendC2Cmd(on ? 0x4A : 0x4B, 0); }

void Viewpro::zoomToEOSerial(int zoom) { sendC2Cmd(0x53, zoom * 10); }

void Viewpro::zoomToIRSerial(int zoom) { sendC2Cmd(0x56, zoom * 10); }


/* E1 Tracking Command Commonly Used */

uint8_t Viewpro::s_makeE1SerialData(uint8_t source, uint8_t param1)
{
    uint8_t result = 0;
    result |= ((param1 & 0x1F) << 3);
    result |= ((source & 0x7));
    return result;
}

void Viewpro::sendE1Cmd(uint8_t cmd, uint8_t source, uint8_t param1, uint8_t param2)
{
    QByteArray packet(HEADER_LEN + BODY_LEN + ID_LEN + E1_DATA_LEN + CHECKSUM_LEN, 0x00);
    packet[0] = static_cast<char>(0x55);
    packet[1] = static_cast<char>(0xAA);
    packet[2] = static_cast<char>(0xDC);
    packet[3] = static_cast<char>((BODY_LEN + ID_LEN + E1_DATA_LEN + CHECKSUM_LEN) & 0x3F);
    packet[4] = static_cast<char>(0x1E);
    packet[5] = static_cast<char>(s_makeE1SerialData(source, param1));
    packet[6] = static_cast<char>(cmd);
    packet[7] = static_cast<char>(param2);
    packet[8] = static_cast<char>(s_viewlink_protocol_checksum(packet));
    _sendSerialCmd(packet);
}

void Viewpro::trackingStopSerial() { sendE1Cmd(0x01, _viewproFactGroup->pipMode(), 0, 0); }

void Viewpro::trackingSearchYawSerial(int yaw, int pitch) { sendE1Cmd(0x02, _viewproFactGroup->pipMode(), static_cast<uint8_t>(yaw), static_cast<uint8_t>(pitch)); }

void Viewpro::trackingOnSerial() { sendE1Cmd(0x03, _viewproFactGroup->pipMode(), 0, 0); }

void Viewpro::trackingAISerial(bool enabled) { sendE1Cmd(0x05, _viewproFactGroup->pipMode(), 0, enabled); }

void Viewpro::trackingAIAutotrackSerial() { sendE1Cmd(0x08, _viewproFactGroup->pipMode(), 0, 0); }

void Viewpro::trackingTemplateSizeSerial(int size) { sendE1Cmd(0x20 | (size + 1), _viewproFactGroup->pipMode(), 0, 0); }


/* E2 Tracking Command Infrequently Used */

void Viewpro::sendE2Cmd(uint8_t cmd, uint16_t param1, uint16_t param2)
{
    QByteArray packet(HEADER_LEN + BODY_LEN + ID_LEN + E2_DATA_LEN + CHECKSUM_LEN, 0x00);
    packet[0] = static_cast<char>(0x55);
    packet[1] = static_cast<char>(0xAA);
    packet[2] = static_cast<char>(0xDC);
    packet[3] = static_cast<char>((BODY_LEN + ID_LEN + E2_DATA_LEN + CHECKSUM_LEN) & 0x3F);
    packet[4] = static_cast<char>(0x2E);
    packet[5] = static_cast<char>(cmd);
    packet[6] = static_cast<char>(param1 >> 8);
    packet[7] = static_cast<char>(param1 & 0xff);
    packet[8] = static_cast<char>(param2 >> 8);
    packet[9] = static_cast<char>(param2 & 0xff);
    packet[10] = static_cast<char>(s_viewlink_protocol_checksum(packet));
    _sendSerialCmd(packet);
}

void Viewpro::trackingPointSerial(int yaw, int tilt) { sendE2Cmd(0x0A, yaw, tilt); }

void Viewpro::trackingIdentifyOnSerial() { sendE2Cmd(0x20); }

void Viewpro::trackingIdentifyOffSerial() { sendE2Cmd(0x21); }


/* S1 TGCC Control Commonly Used */

void Viewpro::sendS1Cmd(uint8_t cmd, uint32_t param1, uint32_t param2, uint32_t param3)
{
    QByteArray packet(HEADER_LEN + BODY_LEN + ID_LEN + S1_DATA_LEN + CHECKSUM_LEN, 0x00);
    packet[0] = static_cast<char>(0x55);
    packet[1] = static_cast<char>(0xAA);
    packet[2] = static_cast<char>(0xDC);
    packet[3] = static_cast<char>((BODY_LEN + ID_LEN + S1_DATA_LEN + CHECKSUM_LEN) & 0x3F);
    packet[4] = static_cast<char>(0x16);
    packet[5] = static_cast<char>(cmd);
    packet[6] = static_cast<char>(param1 >> 24);
    packet[7] = static_cast<char>(param1 >> 16);
    packet[8] = static_cast<char>(param1 >> 8);
    packet[9] = static_cast<char>(param1 & 0xff);
    packet[10] = static_cast<char>(param2 >> 24);
    packet[11] = static_cast<char>(param2 >> 16);
    packet[12] = static_cast<char>(param2 >> 8);
    packet[13] = static_cast<char>(param2 & 0xff);
    packet[14] = static_cast<char>(param3 >> 24);
    packet[15] = static_cast<char>(param3 >> 16);
    packet[16] = static_cast<char>(param3 >> 8);
    packet[17] = static_cast<char>(param3 & 0xff);
    packet[18] = s_viewlink_protocol_checksum(packet);
    _sendSerialCmd(packet);
}

void Viewpro::gimbalToTargetSerial(QGeoCoordinate coord) { sendS1Cmd(0x01, coord.latitude(), coord.longitude(), coord.altitude()); }

/* IP query command 0x29 */

void Viewpro::sendIPQuerySerial(uint8_t ip_first_part, uint8_t ip_second_part, uint8_t ip_third_part, uint8_t ip_fourth_part)
{
    QByteArray packet(HEADER_LEN + BODY_LEN + ID_LEN + 4 + CHECKSUM_LEN, 0x00);
    packet[0] = static_cast<char>(0x55);
    packet[1] = static_cast<char>(0xAA);
    packet[2] = static_cast<char>(0xDC);
    packet[3] = static_cast<char>((BODY_LEN + ID_LEN + 4 + CHECKSUM_LEN) & 0x3F);
    packet[4] = static_cast<char>(0x92);
    packet[5] = static_cast<char>(ip_first_part);
    packet[6] = static_cast<char>(ip_second_part);
    packet[7] = static_cast<char>(ip_third_part);
    packet[8] = static_cast<char>(ip_fourth_part);
    packet[9] = static_cast<char>(s_viewlink_protocol_checksum(packet));
    _sendSerialCmd(packet);
}

// void Viewpro::sbusModeSerial(bool enabled)
// {
//     QByteArray packet(5, 0x00);
//     packet[0] = static_cast<char>(0xAA);
//     packet[1] = static_cast<char>(0x55);
//     packet[2] = static_cast<char>(0x08);
//     packet[3] = static_cast<char>(enabled ? 0x07 : 0x06);
//     packet[4] = static_cast<char>(0xFF);
//     s_sendSerialCmd(packet);
// }

// void Viewpro::setTimeZoneSerial(char zone)
// {
//     QByteArray packet(5, 0x00);
//     packet[0] = static_cast<char>(0xAA);
//     packet[1] = static_cast<char>(0x55);
//     packet[2] = static_cast<char>(0x04);
//     packet[3] = static_cast<char>(zone);
//     packet[4] = static_cast<char>(0xFF);
//     s_sendSerialCmd(packet);
// }

// void setOSDDataSerial(char param1, char param2)
// {
//     QByteArray packet(48, 0x00);
//     packet[0] = static_cast<char>(0x7E);
//     packet[1] = static_cast<char>(0x7E);
//     packet[2] = static_cast<char>(0x44);
//     packet[3] = static_cast<char>(param1);
//     packet[4] = static_cast<char>(0);
//     packet[5] = static_cast<char>(0x83);
//     packet[6] = static_cast<char>(param2);
//     CalculateCheckSumV2(packet);
// }
