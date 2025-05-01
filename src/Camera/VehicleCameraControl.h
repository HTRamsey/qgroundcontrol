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
#include "QmlObjectListModel.h"

class QGCVideoStreamInfo;
class QNetworkAccessManager;
class QDomNode;
class QDomNodeList;

/*===========================================================================*/
class QGCCameraOptionExclusion
{
public:
    QGCCameraOptionExclusion(const QString &param_, const QString &value_, const QStringList &exclusions_);
    const QString param;
    const QString value;
    const QStringList exclusions;
};

/*===========================================================================*/
class QGCCameraOptionRange : public QObject
{
public:
    QGCCameraOptionRange(QObject *parent, const QString &param_, const QString &value_, const QString &targetParam_, const QString &condition_, const QStringList &optNames_, const QStringList &optValues_);
    const QString param;
    const QString value;
    const QString targetParam;
    const QString condition;
    const QStringList optNames;
    const QStringList optValues;
    const QVariantList optVariants;
};

/*===========================================================================*/
/// MAVLink Camera API controller
class VehicleCameraControl : public MavlinkCameraControl
{
public:
    explicit VehicleCameraControl(const mavlink_camera_information_t &info, Vehicle *vehicle, int compID, QObject *parent = nullptr);
    ~VehicleCameraControl();

    Q_INVOKABLE void setCameraModeVideo();
    Q_INVOKABLE void setCameraModePhoto();
    Q_INVOKABLE void toggleCameraMode();
    Q_INVOKABLE bool takePhoto();
    Q_INVOKABLE bool stopTakePhoto();
    Q_INVOKABLE bool startVideoRecording();
    Q_INVOKABLE bool stopVideoRecording();
    Q_INVOKABLE bool toggleVideoRecording();
    Q_INVOKABLE void resetSettings();
    Q_INVOKABLE void formatCard(int id = 1);
    Q_INVOKABLE void stepZoom(int direction);
    Q_INVOKABLE void startZoom(int direction);
    Q_INVOKABLE void stopZoom();
    Q_INVOKABLE void stopStream();
    Q_INVOKABLE void resumeStream();
    Q_INVOKABLE void startTracking(QRectF rec);
    Q_INVOKABLE void startTracking(QPointF point, double radius);
    Q_INVOKABLE void stopTracking();

    int version() { return _version; }
    QString modelName() { return _modelName; }
    QString vendor() { return _vendor; }
    QString firmwareVersion();
    qreal focalLength() { return static_cast<qreal>(_info.focal_length); }
    QSizeF sensorSize() { return QSizeF(static_cast<qreal>(_info.sensor_size_h), static_cast<qreal>(_info.sensor_size_v)); }
    QSize resolution() { return QSize(_info.resolution_h, _info.resolution_v); }
    bool capturesVideo() { return (_info.flags & CAMERA_CAP_FLAGS_CAPTURE_VIDEO); }
    bool capturesPhotos() { return (_info.flags & CAMERA_CAP_FLAGS_CAPTURE_IMAGE); }
    bool hasModes() { return (_info.flags & CAMERA_CAP_FLAGS_HAS_MODES); }
    bool hasZoom() { return (_info.flags & CAMERA_CAP_FLAGS_HAS_BASIC_ZOOM); }
    bool hasFocus() { return (_info.flags & CAMERA_CAP_FLAGS_HAS_BASIC_FOCUS); }
    bool hasTracking() { return (_trackingStatus & TRACKING_SUPPORTED); }
    bool hasVideoStream() { return (_info.flags & CAMERA_CAP_FLAGS_HAS_VIDEO_STREAM); }
    bool photosInVideoMode() { return (_info.flags & CAMERA_CAP_FLAGS_CAN_CAPTURE_IMAGE_IN_VIDEO_MODE); }
    bool videoInPhotoMode() { return (_info.flags & CAMERA_CAP_FLAGS_CAN_CAPTURE_VIDEO_IN_IMAGE_MODE); }

    int compID() { return _compID; }
    bool isBasic() { return _settings.isEmpty(); }
    VideoCaptureStatus videoCaptureStatus();
    PhotoCaptureStatus photoCaptureStatus();
    PhotoCaptureMode photoCaptureMode() { return _photoMode; }
    qreal photoLapse() { return _photoLapse; }
    int photoLapseCount() { return _photoLapseCount; }
    CameraMode cameraMode() { return _cameraMode; }
    StorageStatus storageStatus() { return _storageStatus; }
    QStringList activeSettings();
    quint32 storageFree() { return _storageFree;  }
    QString storageFreeStr();
    quint32 storageTotal() { return _storageTotal; }
    int batteryRemaining() { return _batteryRemaining; }
    QString batteryRemainingStr();
    bool paramComplete() { return _paramComplete; }
    qreal zoomLevel() { return _zoomLevel; }
    qreal focusLevel() { return _focusLevel; }

    QmlObjectListModel *streams() { return &_streams; }
    QGCVideoStreamInfo *currentStreamInstance();
    QGCVideoStreamInfo *thermalStreamInstance();
    int currentStream() { return _currentStream; }
    void setCurrentStream(int stream);
    bool autoStream();
    quint32 recordTime() { return _recordTime; }
    QString recordTimeStr();

    QStringList streamLabels() { return _streamLabels; }

    ThermalViewMode thermalMode() { return _thermalMode; }
    void setThermalMode(ThermalViewMode mode);
    double thermalOpacity() { return _thermalOpacity; }
    void setThermalOpacity(double val);

    void setZoomLevel(qreal level);
    void setFocusLevel(qreal level);
    void setCameraMode(CameraMode mode);
    void setPhotoCaptureMode(PhotoCaptureMode mode);
    void setPhotoLapse(qreal interval);
    void setPhotoLapseCount(int count);

    void handleSettings(const mavlink_camera_settings_t &settings);
    void handleCaptureStatus(const mavlink_camera_capture_status_t &capStatus);
    void handleParamAck(const mavlink_param_ext_ack_t &ack);
    void handleParamValue(const mavlink_param_ext_value_t &value);
    void handleStorageInfo(const mavlink_storage_information_t &st);
    void handleBatteryStatus(const mavlink_battery_status_t &bs);
    void handleTrackingImageStatus(const mavlink_camera_tracking_image_status_t &tis);
    void handleVideoInfo(const mavlink_video_stream_information_t &vi);
    void handleVideoStatus(const mavlink_video_stream_status_t &vs);

    bool        trackingEnabled     () { return _trackingStatus & TRACKING_ENABLED; }
    void        setTrackingEnabled  (bool set);

    TrackingStatus trackingStatus   () { return _trackingStatus; }

    bool trackingImageStatus() { return (_trackingImageStatus.tracking_status == 1); }
    QRectF trackingImageRect() { return _trackingImageRect; }

    Fact *exposureMode();
    Fact *ev();
    Fact *iso();
    Fact *shutterSpeed();
    Fact *aperture();
    Fact *wb();
    Fact *mode();
    void factChanged(Fact *pFact);
    bool incomingParameter(Fact *pFact, QVariant &newValue);
    bool validateParameter(Fact *pFact, QVariant &newValue);

    static constexpr const char *kCondition       = "condition";
    static constexpr const char *kControl         = "control";
    static constexpr const char *kDefault         = "default";
    static constexpr const char *kDefnition       = "definition";
    static constexpr const char *kDescription     = "description";
    static constexpr const char *kExclusion       = "exclude";
    static constexpr const char *kExclusions      = "exclusions";
    static constexpr const char *kLocale          = "locale";
    static constexpr const char *kLocalization    = "localization";
    static constexpr const char *kMax             = "max";
    static constexpr const char *kMin             = "min";
    static constexpr const char *kModel           = "model";
    static constexpr const char *kName            = "name";
    static constexpr const char *kOption          = "option";
    static constexpr const char *kOptions         = "options";
    static constexpr const char *kOriginal        = "original";
    static constexpr const char *kParameter       = "parameter";
    static constexpr const char *kParameterrange  = "parameterrange";
    static constexpr const char *kParameterranges = "parameterranges";
    static constexpr const char *kParameters      = "parameters";
    static constexpr const char *kReadOnly        = "readonly";
    static constexpr const char *kWriteOnly       = "writeonly";
    static constexpr const char *kRoption         = "roption";
    static constexpr const char *kStep            = "step";
    static constexpr const char *kDecimalPlaces   = "decimalPlaces";
    static constexpr const char *kStrings         = "strings";
    static constexpr const char *kTranslated      = "translated";
    static constexpr const char *kType            = "type";
    static constexpr const char *kUnit            = "unit";
    static constexpr const char *kUpdate          = "update";
    static constexpr const char *kUpdates         = "updates";
    static constexpr const char *kValue           = "value";
    static constexpr const char *kVendor          = "vendor";
    static constexpr const char *kVersion         = "version";

    static constexpr const char *kPhotoMode       = "PhotoCaptureMode";
    static constexpr const char *kPhotoLapse      = "PhotoLapse";
    static constexpr const char *kPhotoLapseCount = "PhotoLapseCount";
    static constexpr const char *kThermalOpacity  = "ThermalOpacity";
    static constexpr const char *kThermalMode     = "ThermalMode";

    static constexpr const char *kCAM_EV          = "CAM_EV";
    static constexpr const char *kCAM_EXPMODE     = "CAM_EXPMODE";
    static constexpr const char *kCAM_ISO         = "CAM_ISO";
    static constexpr const char *kCAM_SHUTTERSPD  = "CAM_SHUTTERSPD";
    static constexpr const char *kCAM_APERTURE    = "CAM_APERTURE";
    static constexpr const char *kCAM_WBMODE      = "CAM_WBMODE";
    static constexpr const char *kCAM_MODE        = "CAM_MODE";

protected:
    void _setVideoStatus(VideoCaptureStatus status);
    void _setPhotoStatus(PhotoCaptureStatus status);
    void _setCameraMode(CameraMode mode);
    void _requestStreamInfo(uint8_t streamID);
    void _requestStreamStatus(uint8_t streamID);
    void _requestTrackingStatus();
    QGCVideoStreamInfo *_findStream(uint8_t streamID, bool report = true);
    QGCVideoStreamInfo *_findStream(const QString &name);

protected slots:
    void _initWhenReady();
    void _requestCameraSettings();
    void _requestAllParameters();
    void _requestParamUpdates();
    void _requestCaptureStatus();
    void _requestStorageInfo();
    void _downloadFinished();
    void _mavCommandResult(int vehicleId, int component, int command, int result, int failureCode);
    void _dataReady(QByteArray data);
    void _paramDone();
    void _streamInfoTimeout();
    void _streamStatusTimeout();
    void _recTimerHandler();
    void _checkForVideoStreams();

private:
    bool _handleLocalization(QByteArray& bytes);
    bool _replaceLocaleStrings(const QDomNode node, QByteArray& bytes);
    bool _loadCameraDefinitionFile(QByteArray& bytes);
    bool _loadConstants(const QDomNodeList nodeList);
    bool _loadSettings(const QDomNodeList nodeList);
    void _processRanges();
    bool _processCondition(const QString condition);
    bool _processConditionTest(const QString conditionTest);
    bool _loadNameValue(QDomNode option, const QString factName, FactMetaData* metaData, QString& optName, QString& optValue, QVariant& optVariant);
    bool _loadRanges(QDomNode option, const QString factName, QString paramValue);
    void _updateActiveList();
    void _updateRanges(Fact* pFact);
    void _httpRequest(const QString& url);
    void _handleDefinitionFile(const QString& url);
    void _ftpDownloadComplete(const QString& fileName, const QString& errorMsg);

    QStringList _loadExclusions(QDomNode option);
    QStringList _loadUpdates(QDomNode option);
    QString _getParamName(const char* param_id);

protected:
    int                                 _compID             = 0;
    mavlink_camera_information_t        _info;
    int                                 _version            = 0;
    bool                                _cached             = false;
    bool                                _paramComplete      = false;
    qreal                               _zoomLevel          = 0.0;
    qreal                               _focusLevel         = 0.0;
    uint32_t                            _storageFree        = 0;
    uint32_t                            _storageTotal       = 0;
    int                                 _batteryRemaining   = -1;
    QNetworkAccessManager*              _netManager         = nullptr;
    QString                             _modelName;
    QString                             _vendor;
    QString                             _cacheFile;
    CameraMode                          _cameraMode         = CAM_MODE_UNDEFINED;
    StorageStatus                       _storageStatus      = STORAGE_NOT_SUPPORTED;
    PhotoCaptureMode                    _photoMode          = PHOTO_CAPTURE_SINGLE;
    qreal                               _photoLapse         = 1.0;
    int                                 _photoLapseCount    = 0;
    VideoCaptureStatus                  _video_status       = VIDEO_CAPTURE_STATUS_UNDEFINED;
    PhotoCaptureStatus                  _photo_status       = PHOTO_CAPTURE_STATUS_UNDEFINED;
    QStringList                         _activeSettings;
    QStringList                         _settings;
    QTimer                              _captureStatusTimer;
    QList<QGCCameraOptionExclusion*>    _valueExclusions;
    QList<QGCCameraOptionRange*>        _optionRanges;
    QMap<QString, QStringList>          _originalOptNames;
    QMap<QString, QVariantList>         _originalOptValues;
    QMap<QString, QGCCameraParamIO*>    _paramIO;
    int                                 _cameraSettingsRetries = 0;
    int                                 _cameraCaptureStatusRetries = 0;
    int                                 _storageInfoRetries = 0;
    int                                 _captureInfoRetries = 0;
    bool                                _resetting          = false;
    QTimer                              _recTimer;
    QTime                               _recTime;
    uint32_t                            _recordTime         = 0;
    QMap<QString, QStringList>          _requestUpdates;
    QStringList                         _updatesToRequest;
    int                                 _videoStreamInfoRetries   = 0;
    int                                 _videoStreamStatusRetries = 0;
    int                                 _requestCount       = 0;
    int                                 _currentStream      = 0;
    int                                 _expectedCount      = 1;
    QTimer                              _streamInfoTimer;
    QTimer                              _streamStatusTimer;
    QmlObjectListModel                  _streams;
    QStringList                         _streamLabels;
    ThermalViewMode                     _thermalMode        = THERMAL_BLEND;
    double                              _thermalOpacity     = 85.0;
    TrackingStatus                      _trackingStatus     = TRACKING_UNKNOWN;
    QRectF                              _trackingMarquee;
    QPointF                             _trackingPoint;
    double                              _trackingRadius     = 0.0;
    mavlink_camera_tracking_image_status_t  _trackingImageStatus;
    QRectF                                  _trackingImageRect;
};
