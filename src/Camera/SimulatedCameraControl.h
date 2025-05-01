/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "MavlinkCameraControl.h"

class Vehicle;

/// Creates a simulated Camera Control which supports:
///     Video record if a manual stream is available
///     Photo capture using DO_DIGICAM_CONTROL if the setting is enabled
///     It does not support time lapse capture
class SimulatedCameraControl : public MavlinkCameraControl
{
    Q_OBJECT

public:
    explicit SimulatedCameraControl(Vehicle *vehicle, QObject *parent = nullptr);

    void setCameraModeVideo() final;
    void setCameraModePhoto() final;
    void toggleCameraMode() final;
    bool takePhoto() final;
    bool startVideoRecording() final;
    bool stopVideoRecording() final;
    bool toggleVideoRecording() final;
    bool capturesVideo() const final;
    bool capturesPhotos() const final;
    bool hasVideoStream() const final;
    bool photosInVideoMode() const final { return true; }
    bool videoInPhotoMode() const final { return false; }
    void setCameraMode(CameraMode mode) final;
    void setPhotoCaptureMode(PhotoCaptureMode mode) final;
    void setPhotoLapse(qreal interval) final {}
    void setPhotoLapseCount(int count) final {}
    bool hasModes() const final;
    quint32 recordTime() const final;
    QString recordTimeStr() const final;
    VideoCaptureStatus videoCaptureStatus() const final;
    PhotoCaptureStatus photoCaptureStatus() const final { return _photoCaptureStatus; }
    PhotoCaptureMode photoCaptureMode() const final { return _photoCaptureMode; }
    CameraMode cameraMode() const final { return _cameraMode; }
    qreal photoLapse() const final { return _photoLapse; }
    int photoLapseCount() const final { return _photoLapseCount; }

    void resetSettings() final {}
    void formatCard(int id = 1) final { Q_UNUSED(id);}
    void stepZoom(int /*direction*/) final {}
    void startZoom(int /*direction*/) final {}
    void stopZoom() final {}
    void stopStream() final {}
    bool stopTakePhoto() final { return false;}
    void resumeStream() final {}
    void startTracking(QRectF /*rec*/) final {}
    void startTracking(QPointF /*point*/, double /*radius*/) final {}
    void stopTracking() final {}
    int version() const final { return 0; }
    QString modelName() const final { return QStringLiteral("Simulated Camera"); }
    QString vendor() const final { return QStringLiteral("QGroundControl"); }
    QString firmwareVersion() const final { return QStringLiteral("1.0"); }
    qreal focalLength() const final { return qQNaN(); }
    QSizeF sensorSize() const final { return QSizeF(qQNaN(), qQNaN()); }
    QSize resolution() const final { return QSize(0, 0); }
    bool hasZoom() const final { return false; }
    bool hasFocus() const final { return false; }
    bool hasTracking() const final { return false; }
    int compID() const final { return 0; }
    bool isBasic() const final { return true; }
    StorageStatus storageStatus() const final { return STORAGE_NOT_SUPPORTED; }
    QStringList activeSettings() const final { return QStringList(); }
    quint32 storageFree() const final { return 0; }
    QString storageFreeStr() const final { return QString(); }
    quint32 storageTotal() const final { return 0; }
    int batteryRemaining() const final { return -1;}
    QString batteryRemainingStr() const final { return QString(); }
    bool paramComplete() const final { return true; }
    qreal zoomLevel() const final { return 1.0; }
    qreal focusLevel() const final { return 1.0; }
    QGCVideoStreamInfo *thermalStreamInstance() final { return nullptr; }
    QGCVideoStreamInfo *currentStreamInstance() final { return nullptr; }
    int currentStream() const final { return 0; }
    QmlObjectListModel *streams() final { return nullptr; }
    void setCurrentStream(int /*stream*/) final {}
    bool autoStream() const final { return false; }
    Fact *exposureMode() final { return nullptr; }
    Fact *ev() final { return nullptr; }
    Fact *iso() final { return nullptr; }
    Fact *shutterSpeed() final { return nullptr; }
    Fact *aperture() final { return nullptr; }
    Fact *wb() final { return nullptr; }
    Fact *mode() final { return nullptr; }
    QStringList streamLabels() const final { return QStringList(); }
    ThermalViewMode thermalMode() const final { return THERMAL_OFF; }
    void setThermalMode(ThermalViewMode /*mode*/) final {}
    double thermalOpacity() const final { return 0.0; }
    void setThermalOpacity(double /*val*/) final {}
    void setZoomLevel(qreal /*level*/) final {}
    void setFocusLevel(qreal /*level*/) final {}
    bool trackingEnabled() const final { return false; }
    void setTrackingEnabled(bool /*set*/) final {}
    TrackingStatus trackingStatus() const final { return TRACKING_UNKNOWN; }
    bool trackingImageStatus() const final { return false; }
    QRectF trackingImageRect() const final { return QRectF(); }
    void factChanged(Fact* /*pFact*/) final {};
    bool incomingParameter(Fact* /*pFact*/, QVariant& /*newValue*/) final { return false; }
    bool validateParameter(Fact* /*pFact*/, QVariant& /*newValue*/) final { return false; }
    void handleSettings(const mavlink_camera_settings_t& /*settings*/) final {}
    void handleCaptureStatus(const mavlink_camera_capture_status_t& /*capStatus*/) final {}
    void handleParamAck(const mavlink_param_ext_ack_t& /*ack*/) final {}
    void handleParamValue(const mavlink_param_ext_value_t& /*value*/) final {}
    void handleStorageInfo(const mavlink_storage_information_t& /*st*/) final {}
    void handleBatteryStatus(const mavlink_battery_status_t& /*bs*/) final {}
    void handleTrackingImageStatus(const mavlink_camera_tracking_image_status_t* /*tis*/) final {}
    void handleVideoInfo(const mavlink_video_stream_information_t* /*vi*/) final {}
    void handleVideoStatus(const mavlink_video_stream_status_t* /*vs*/) final {}

protected slots:
    void _paramDone() final {};
};
