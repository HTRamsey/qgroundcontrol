#pragma once

#include "MavCameraData.h"
#include <QtCore/QUrl>

class ViewproData : public MavCameraData
{
    Q_OBJECT
//    QML_ELEMENT
//    QML_UNCREATABLE("")

//    Q_PROPERTY(uint32_t gimMode READ gimMode NOTIFY gimModeChanged)
//    Q_PROPERTY(uint32_t gimSpeed READ gimSpeed NOTIFY gimSpeedChanged)
//    Q_PROPERTY(float pitchAngle READ pitchAngle NOTIFY pitchAngleChanged)
//    Q_PROPERTY(float yawAngle READ yawAngle NOTIFY yawAngleChanged)
//    Q_PROPERTY(uint32_t camMode READ camMode NOTIFY camModeChanged)
//    Q_PROPERTY(bool trackSW READ trackSW NOTIFY trackSWChanged)
//    Q_PROPERTY(uint32_t osdSet READ osdSet NOTIFY osdSetChanged)
//    Q_PROPERTY(uint32_t irDZoom READ irDZoom NOTIFY irDZoomChanged)
//    Q_PROPERTY(uint32_t pipMode READ pipMode NOTIFY pipModeChanged)
    //Q_PROPERTY(uint32_t targetCali READ targetCali NOTIFY targetCaliChanged)
    //Q_PROPERTY(float tagUPDWCali READ tagUPDWCali NOTIFY tagUPDWCaliChanged)
    //Q_PROPERTY(float tagLTRTCali READ tagLTRTCali NOTIFY tagLTRTCaliChanged)

    typedef enum {
        GIM_MODE = 1,
        GIM_SPEED,
        PITCH_ANGLE,
        YAW_ANGLE,
        CAM_MODE,
        TRACK_SW,
        OSD_SET,
        IR_DZOOM,
        PIP_MODE,
        BLANK = 10,
        USER_COMMAND = 16,
        TARGET_CALI,
        TAG_UPDW_CALI,
        TAG_LTRT_CALI,
        LRF_CALI,
        LRF_UPDW_CALI,
    }ViewproParams;

signals:
    void gimModeChanged(uint32_t);
    void gimSpeedChanged(uint32_t);
    void pitchAngleChanged(float);
    void yawAngleChanged(float);
    void camModeChanged(uint32_t);
    void trackSWChanged(float);
    void osdSetChanged(uint32_t);
    void irDZoomChanged(uint32_t);
    void pipModeChanged(uint32_t);
    void targetCaliChanged(uint32_t);
    void tagUPDWCaliChanged(float);
    void tagLTRTCaliChanged(float);

    void streamChanged();

public:
    static ViewproData* viewprodata()
    {
        static ViewproData instance;
        return &instance;
    }
    ~ViewproData();

    uint32_t gimMode() const { return m_gimMode; }
    uint32_t gimSpeed() const { return m_gimSpeed; }
    float pitchAngle() const { return m_pitchAngle; }
    float yawAngle() const { return m_yawAngle; }
    uint32_t camMode2() const { return m_camMode2; }
    bool trackSW() const { return m_trackSW; }
    uint32_t osdSet() const { return m_osdSet; }
    uint32_t irDZoom() const { return m_irDZoom; }
    uint32_t pipMode() const { return m_pipMode; }
    uint32_t targetCali() const { return m_targetCali; }
    float tagUPDWCali() const { return m_tagUPDWCali; }
    float tagLTRTCali() const { return m_tagLTRTCali; }

    static constexpr uint8_t sysID() { return 1; }
    static constexpr uint8_t compID() { return gimbalCompID(); }
    static constexpr uint8_t cameraCompID() { return MAV_COMP_ID_CAMERA; }
    static constexpr uint8_t gimbalCompID() { return MAV_COMP_ID_GIMBAL; }

private:
    explicit ViewproData(QObject* parent = nullptr);

    uint32_t m_gimMode = 0;
    uint32_t m_gimSpeed = 0;
    float m_pitchAngle = 0.0f;
    float m_yawAngle = 0.0f;
    uint32_t m_camMode2 = 0;
    bool m_trackSW = false;
    uint32_t m_osdSet = 0;
    uint32_t m_irDZoom = 0;
    uint32_t m_pipMode = 0;
    uint32_t m_targetCali = 0;
    float m_tagUPDWCali = 0.0f;
    float m_tagLTRTCali = 0.0f;

    QUrl m_stream{};
    const QString m_streamUrl = "rtsp://%1:554/stream0";

    void _handleParamExtValue(char param_id[16], uint16_t param_index, const QVariant value) final;
};
