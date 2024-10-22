#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QVariant>
#include <QtPositioning/QGeoCoordinate>

Q_DECLARE_LOGGING_CATEGORY(ViewproLog)

class ViewproFactGroup;

class Viewpro : public QObject
{
    Q_OBJECT

public:
    explicit Viewpro(ViewproFactGroup *viewproFactGroup, QObject *parent = nullptr);
    ~Viewpro();

                void setProtocolControlSerial(uint8_t LRF=0x00, uint8_t NMEA=0x00, uint8_t gimbal3E=0x3E, uint8_t tracking7E=0x7E, uint8_t sonyVISCA=0x00, uint8_t targetPositionCalc=0x00, uint8_t mavlink=0xED, uint8_t pelco_d=0x00, uint8_t feedback=0x00);
                void setTimeZoneSerial(int8_t hour, int8_t minute);
    Q_INVOKABLE void setOSD1Serial(bool on);
    Q_INVOKABLE void setOSD2Serial(bool on);
                void setBaudRateSerial();
    Q_INVOKABLE void setEODigitalZoomSerial(bool on);
    Q_INVOKABLE void setEnableTrackingSerial(bool on);
                void sendOutputControlSerial(bool mav_serial, bool mav_udp, bool track_serial, bool reset_ip);
    Q_INVOKABLE void setTrackModeSerial(bool highSpeed);
    Q_INVOKABLE void setFlipImageSerial(bool irOn, bool eoOn);
                void setMavlinkSysIDSerial(int sysID);

                void gimbalMotorSerial(bool on);
                void gimbalManualSpeedSerial(float yaw, float tilt);
    Q_INVOKABLE void gimbalFollowYawSerial();
    Q_INVOKABLE void gimbalHomeSerial();
                void trackingModeSerial();
    Q_INVOKABLE void gimbalFollowYawDisableSerial();
    Q_INVOKABLE void gimbalManualRCSerial(float yaw, float tilt);
    Q_INVOKABLE void gimbalLookDownSerial();

    Q_INVOKABLE void setVideoSourceEO1Serial();
    Q_INVOKABLE void setVideoSourceIRSerial();
    Q_INVOKABLE void setVideoSourceEO1IRPIPSerial();
    Q_INVOKABLE void setVideoSourceIREO1PIPSerial();
    // Q_INVOKABLE void setVideoSourceEO2Serial();
    // Q_INVOKABLE void setVideoSourceFusionSerial();
    Q_INVOKABLE void focusZoomStopSerial();
    // Q_INVOKABLE void brightnessUpSerial();
    // Q_INVOKABLE void brightnessDownSerial();
    Q_INVOKABLE void zoomOutSerial(int speed);
    Q_INVOKABLE void zoomInSerial(int speed);
    Q_INVOKABLE void focusInSerial();
    Q_INVOKABLE void focusOutSerial();
    // Q_INVOKABLE void setIRModePseudoSerial();
    Q_INVOKABLE void setIRModeWhiteHotSerial();
    Q_INVOKABLE void setIRModeBlackHotSerial();
    Q_INVOKABLE void rainbowIRSerial();
    Q_INVOKABLE void setAutoFocusSerial(bool enabled);
                void zoomInIRDigitalSerial();
                void zoomOutIRDigitalSerial();
    Q_INVOKABLE void setIRColorExtSerial(int value);

                void digitalZoomEOSerial(bool on);
                void digitalZoomEOMaxSerial(int level);
                void imageEnhanceSerial(bool on);
                void colorBarIRSerial(bool on);
    Q_INVOKABLE void imageFlipEOSerial(bool on);
                void setDefogSerial(bool on);
    Q_INVOKABLE void imageFlipIRSerial(bool on);
                void nearIRSerial(bool on);
    Q_INVOKABLE void zoomToEOSerial(int zoom);
    Q_INVOKABLE void zoomToIRSerial(int zoom);

    Q_INVOKABLE void trackingStopSerial();
                void trackingSearchYawSerial(int yaw, int pitch);
    Q_INVOKABLE void trackingOnSerial();
    Q_INVOKABLE void trackingAISerial(bool enabled);
    Q_INVOKABLE void trackingAIAutotrackSerial();
    Q_INVOKABLE void trackingTemplateSizeSerial(int size);

    Q_INVOKABLE void trackingPointSerial(int yaw, int tilt);
                void trackingIdentifyOnSerial();
                void trackingIdentifyOffSerial();

    Q_INVOKABLE void gimbalToTargetSerial(QGeoCoordinate coord);

                void sendIPQuerySerial(uint8_t ip_first_part, uint8_t ip_second_part, uint8_t ip_third_part, uint8_t ip_fourth_part);

private:
    void sendUCmd(uint8_t cmd, uint8_t param1=0U, uint8_t param2=0U, uint8_t param3=0U, uint8_t param4=0U, uint8_t param5=0U, uint8_t param6=0U, uint8_t param7=0U, uint8_t param8=0U, uint8_t param9=0U);
    void sendA1Cmd(uint8_t control, uint16_t param1=0U, uint16_t param2=0U, uint16_t param3=0U, uint16_t param4=0U);
    static uint16_t s_makeC1SerialData(uint8_t LRF, uint8_t cmd, uint8_t speed, uint8_t sensor);
    void sendC1Cmd(uint8_t cmd, uint8_t speed, uint8_t sensor, uint8_t LRF);
    void sendC2Cmd(uint8_t cmd, uint16_t param);
    static uint8_t s_makeE1SerialData(uint8_t source, uint8_t param1);
    void sendE1Cmd(uint8_t cmd, uint8_t source, uint8_t param1, uint8_t param2);
    void sendE2Cmd(uint8_t cmd, uint16_t param1=0x00, uint16_t param2=0x00);
    void sendS1Cmd(uint8_t cmd, uint32_t param1, uint32_t param2, uint32_t param3);
    void setParamExtGimMode(uint8_t index);
    void setParamExtPIPMode(uint8_t index);
    void setParamExtIRMode(uint8_t index);
    static uint8_t s_viewlink_protocol_checksum(QByteArrayView viewlink_data_buf);

    void _sendSerialCmd(const QByteArray &data);

    ViewproFactGroup *_viewproFactGroup = nullptr;

    static constexpr uint8_t HEADER_LEN   = 3;
    static constexpr uint8_t BODY_LEN     = 1;
    static constexpr uint8_t ID_LEN       = 1;
    static constexpr uint8_t CHECKSUM_LEN = 1;
    static constexpr uint8_t U_DATA_LEN   = 10;
    static constexpr uint8_t A1_DATA_LEN  = 9;
    static constexpr uint8_t C1_DATA_LEN  = 2;
    static constexpr uint8_t C2_DATA_LEN  = 3;
    static constexpr uint8_t E1_DATA_LEN  = 3;
    static constexpr uint8_t E2_DATA_LEN  = 5;
    static constexpr uint8_t S1_DATA_LEN  = 14;
};
