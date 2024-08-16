#pragma once

#include "Component.h"
#include <QtCore/QUrl>

class NextVisionData : public MavComponent
{
    Q_OBJECT
    // QML_ELEMENT
    // QML_UNCREATABLE("")

    Q_PROPERTY(QUrl stream READ stream NOTIFY streamChanged)

//    Q_PROPERTY(float gndCrsAlt              READ gndCrsAlt              NOTIFY gndCrsAltChanged)

//    Q_PROPERTY(float videoDestCh0           READ videoDestCh0           NOTIFY videoDestCh0Changed)
//    Q_PROPERTY(float videoDestPort          READ videoDestPort          NOTIFY videoDestPortChanged)
//    Q_PROPERTY(float enableBandWidthLimit   READ enableBandWidthLimit   NOTIFY enableBandWidthLimitChanged)
//    Q_PROPERTY(float bandwithLimit          READ bandwithLimit          NOTIFY bandwithLimitChanged)
//    Q_PROPERTY(float streamBitrate          READ streamBitrate          NOTIFY streamBitrateChanged)
//    Q_PROPERTY(float recordingBitrate       READ recordingBitrate       NOTIFY recordingBitrateChanged)
//    Q_PROPERTY(float ethIPAddr              READ ethIPAddr              NOTIFY ethIPAddrChanged)
//    Q_PROPERTY(float ethSubnetMask          READ ethSubnetMask          NOTIFY ethSubnetMaskChanged)
//    Q_PROPERTY(float ethGatewayIP           READ ethGatewayIP           NOTIFY ethGatewayIPChanged)
//    Q_PROPERTY(float MTU                    READ MTU                    NOTIFY MTUChanged)
//    Q_PROPERTY(float enableRollDerot        READ enableRollDerot        NOTIFY enableRollDerotChanged)
//    Q_PROPERTY(float autoRecordEnable       READ autoRecordEnable       NOTIFY autoRecordEnableChanged)
//    Q_PROPERTY(float autoRecordINSTimeout   READ autoRecordINSTimeout   NOTIFY autoRecordINSTimeoutChanged)
//    Q_PROPERTY(float enableAES              READ enableAES              NOTIFY enableAESChanged)
//    Q_PROPERTY(float AES128Key              READ AES128Key              NOTIFY AES128KeyChanged)
//    Q_PROPERTY(float remoteIP               READ remoteIP               NOTIFY remoteIPChanged)
//    Q_PROPERTY(float remotePort             READ remotePort             NOTIFY remotePortChanged)
//    Q_PROPERTY(float localPort              READ localPort              NOTIFY localPortChanged)
//    Q_PROPERTY(float runFlightPlan          READ runFlightPlan          NOTIFY runFlightPlanChanged)
//    Q_PROPERTY(float forceRollToHorizon     READ forceRollToHorizon     NOTIFY forceRollToHorizonChanged)
//    Q_PROPERTY(float OSDMode                READ OSDMode                NOTIFY OSDModeChanged)
//    Q_PROPERTY(float showGndCrsInfo         READ showGndCrsInfo         NOTIFY showGndCrsInfoChanged)
//    Q_PROPERTY(float showGPSInfo            READ showGPSInfo            NOTIFY showGPSInfoChanged)
//    Q_PROPERTY(float showFlightMode         READ showFlightMode         NOTIFY showFlightModeChanged)
//    Q_PROPERTY(float showAirSpeed           READ showAirSpeed           NOTIFY showAirSpeedChanged)
//    Q_PROPERTY(float OSDMiniMap             READ OSDMiniMap             NOTIFY OSDMiniMapChanged)
//    Q_PROPERTY(float VGAResizingThreshold   READ VGAResizingThreshold   NOTIFY VGAResizingThresholdChanged)
//    Q_PROPERTY(float VGAOutputMode          READ VGAOutputMode          NOTIFY VGAOutputModeChanged)
//    Q_PROPERTY(float reportSystemID         READ reportSystemID         NOTIFY reportSystemIDChanged)
//    Q_PROPERTY(float reportComponentID      READ reportComponentID      NOTIFY reportComponentIDChanged)
//    Q_PROPERTY(float ackSystemID            READ ackSystemID            NOTIFY ackSystemIDChanged)
//    Q_PROPERTY(float ackComponentID         READ ackComponentID         NOTIFY ackComponentIDChanged)
//    Q_PROPERTY(float enableMavSequenceNum   READ enableMavSequenceNum   NOTIFY enableMavSequenceNumChanged)
//    Q_PROPERTY(float streamGOPSize          READ streamGOPSize          NOTIFY streamGOPSizeChanged)
//    Q_PROPERTY(float streamQuantizationVal  READ streamQuantizationVal  NOTIFY streamQuantizationValChanged)
//    Q_PROPERTY(float enableObjDet           READ enableObjDet           NOTIFY enableObjDetChanged)
//    Q_PROPERTY(float objDetType             READ objDetType             NOTIFY objDetTypeChanged)
//    Q_PROPERTY(float objDetAutoTrackEnable  READ objDetAutoTrackEnable  NOTIFY objDetAutoTrackEnableChanged)
//    Q_PROPERTY(float objDetAutoZoomOnTarget READ objDetAutoZoomOnTarget NOTIFY objDetAutoZoomOnTargetChanged)
//    Q_PROPERTY(float objDetLoiterAPOnDetect READ objDetLoiterAPOnDetect NOTIFY objDetLoiterAPOnDetectChanged)
//    Q_PROPERTY(float enableALPR             READ enableALPR             NOTIFY enableALPRChanged)
//    Q_PROPERTY(float ALPRCountryProfile     READ ALPRCountryProfile     NOTIFY ALPRCountryProfileChanged)
//    Q_PROPERTY(float AILicenseStatus        READ AILicenseStatus        NOTIFY AILicenseStatusChanged)

//    Q_PROPERTY(uint32_t carCount               READ carCount               NOTIFY carCountChanged)

//    Q_PROPERTY(bool fireDetected           READ fireDetected           NOTIFY fireDetectedChanged)

//    Q_PROPERTY(uint8_t objDetDetections       READ objDetDetections       NOTIFY objDetDetectionsChanged)
//    Q_PROPERTY(uint16_t objDet0ClassID         READ objDet0ClassID         NOTIFY objDet0ClassIDChanged)
//    Q_PROPERTY(uint16_t objDet0UniqueID        READ objDet0UniqueID        NOTIFY objDet0UniqueIDChanged)
//    Q_PROPERTY(float objDet0Lat             READ objDet0Lat             NOTIFY objDet0LatChanged)
//    Q_PROPERTY(float objDet0Lon             READ objDet0Lon             NOTIFY objDet0LonChanged)
//    Q_PROPERTY(float objDet0Alt             READ objDet0Alt             NOTIFY objDet0AltChanged)

//    Q_PROPERTY(uint16_t SDCardDetected         READ SDCardDetected         NOTIFY SDCardDetectedChanged)
//    Q_PROPERTY(float SDCardTotal            READ SDCardTotal            NOTIFY SDCardTotalChanged)
//    Q_PROPERTY(float SDCardAvailable        READ SDCardAvailable        NOTIFY SDCardAvailableChanged)

//    Q_PROPERTY(bool systemTracking         READ systemTracking         NOTIFY systemTrackingChanged)
//    Q_PROPERTY(bool systemRecording        READ systemRecording        NOTIFY systemRecordingChanged)
//    Q_PROPERTY(uint8_t systemMode             READ systemMode             NOTIFY systemModeChanged)
//    Q_PROPERTY(bool systemSnapshotStatus   READ systemSnapshotStatus   NOTIFY systemSnapshotStatusChanged)

//    Q_PROPERTY(float trackingLat            READ trackingLat            NOTIFY trackingLatChanged)
//    Q_PROPERTY(float trackingLon            READ trackingLon            NOTIFY trackingLonChanged)
//    Q_PROPERTY(float trackingAlt            READ trackingAlt            NOTIFY trackingAltChanged)

//    Q_PROPERTY(uint16_t videoBitrateStream0    READ videoBitrateStream0    NOTIFY videoBitrateStream0Changed)
//    Q_PROPERTY(uint16_t videoBitrateRecord0    READ videoBitrateRecord0    NOTIFY videoBitrateRecord0Changed)

//    Q_PROPERTY(float VMDLat                 READ VMDLat                 NOTIFY VMDLatChanged)
//    Q_PROPERTY(float VMDLon                 READ VMDLon                 NOTIFY VMDLonChanged)
//    Q_PROPERTY(float VMDAlt                 READ VMDAlt                 NOTIFY VMDAltChanged)

    typedef enum
    {
        MavExtReport_System     = 0,
        MavExtReport_LOS        = 1,
        MavExtReport_GndCrs     = 2,
        MavExtReport_Snapshot   = 5,
        MavExtReport_SDCard     = 6,
        MavExtReport_Video      = 7,
        MavExtReport_LOSDirRate = 8,
        MavExtReport_ObjDet     = 9,
        MavExtReport_IMU        = 10,
        MavExtReport_Fire       = 11,
        MavExtReport_Tracking   = 12,
        MavExtReport_LPR        = 13,
        MavExtReport_ARMarkers  = 14,
        MavExtReport_Parameter  = 15,
        MavExtReport_CarCount   = 16,
        MavExtReport_OGLR       = 17,
        MavExtReport_VMD        = 18,
        MavExtReport_PLR        = 19,
        MavExtReport_RangeFinder = 20,
        MavExtReport_AuxCamObjDet = 21,
    }MavlinkExtSetReportArgs;

    typedef enum
    {
        ObjDetClassifier_HumanAndVehicle = 0,
        ObjDetClassifier_FireAndSmoke,
        ObjDetClassifier_HumanOverboard,
        ObjDetClassifier_MarineVessel,
        ObjDetClassifier_AircraftDetector
    }ObjDetClassifierIDs;

    typedef enum
    {
        ObjDetHumanAndVehicleClass_Human = 0,
        ObjDetHumanAndVehicleClass_Vehicle = 2,
        ObjDetHumanAndVehicleClass_Truck = 7
    }ObjDetHumanAndVehicleClassIDs;

    typedef enum
    {
        ObjDetFireAndSmokeClass_Smoke = 80,
        ObjDetFireAndSmokeClass_Fire = 81
    }ObjDetFireAndSmokeClassIDs;

    typedef enum
    {
        ObjDetHumanOverboardClass_Human = 0
    }ObjDetHumanOverboardClassIDs;

    typedef enum
    {
        ObjDetMarineVesselClass_MarineVessel = 0
    }ObjDetMarineVesselClassIDs;

    typedef enum
    {
        ObjDetAircraftDetectorClass_Helicopter = 0,
        ObjDetAircraftDetectorClass_Airplane
    }ObjDetAircraftDetectorClassIDs;

public:
    static NextVisionData* nextvisiondata()
    {
        static NextVisionData instance;
        return &instance;
    }
    ~NextVisionData();

    void _handleMsg(const mavlink_message_t& msg) final;
    static constexpr uint8_t sysID() { return 255; }
    static constexpr MAV_COMPONENT compID() { return MAV_COMP_ID_MISSIONPLANNER; }

    QUrl stream() const { return m_stream; }

    float gndCrsAlt()              const { return _gndCrsAlt; }

    float videoDestCh0()           const { return _videoDestCh0; }
    float videoDestPort()          const { return _videoDestPort; }
    float enableBandWidthLimit()   const { return _enableBandWidthLimit; }
    float bandwithLimit()          const { return _bandwithLimit; }
    float streamBitrate()          const { return _streamBitrate; }
    float recordingBitrate()       const { return _recordingBitrate; }
    float ethIPAddr()              const { return _ethIPAddr; }
    float ethSubnetMask()          const { return _ethSubnetMask; }
    float ethGatewayIP()           const { return _ethGatewayIP; }
    float MTU()                    const { return _MTU; }
    float enableRollDerot()        const { return _enableRollDerot; }
    float autoRecordEnable()       const { return _autoRecordEnable; }
    float autoRecordINSTimeout()   const { return _autoRecordINSTimeout; }
    float enableAES()              const { return _enableAES; }
    float AES128Key()              const { return _AES128Key; }
    float remoteIP()               const { return _remoteIP; }
    float remotePort()             const { return _remotePort; }
    float localPort()              const { return _localPort; }
    float runFlightPlan()          const { return _runFlightPlan; }
    float forceRollToHorizon()     const { return _forceRollToHorizon; }
    float OSDMode()                const { return _OSDMode; }
    float showGndCrsInfo()         const { return _showGndCrsInfo; }
    float showGPSInfo()            const { return _showGPSInfo; }
    float showFlightMode()         const { return _showFlightMode; }
    float showAirSpeed()           const { return _showAirSpeed; }
    float OSDMiniMap()             const { return _OSDMiniMap; }
    float VGAResizingThreshold()   const { return _VGAResizingThreshold; }
    float VGAOutputMode()          const { return _VGAOutputMode; }
    float reportSystemID()         const { return _reportSystemID; }
    float reportComponentID()      const { return _reportComponentID; }
    float ackSystemID()            const { return _ackSystemID; }
    float ackComponentID()         const { return _ackComponentID; }
    float enableMavSequenceNum()   const { return _enableMavSequenceNum; }
    float streamGOPSize()          const { return _streamGOPSize; }
    float streamQuantizationVal()  const { return _streamQuantizationVal; }
    float enableObjDet()           const { return _enableObjDet; }
    float objDetType()             const { return _objDetType; }
    float objDetAutoTrackEnable()  const { return _objDetAutoTrackEnable; }
    float objDetAutoZoomOnTarget() const { return _objDetAutoZoomOnTarget; }
    float objDetLoiterAPOnDetect() const { return _objDetLoiterAPOnDetect; }
    float enableALPR()             const { return _enableALPR; }
    float ALPRCountryProfile()     const { return _ALPRCountryProfile; }
    float AILicenseStatus()        const { return _AILicenseStatus; }

    uint32_t carCount()               const { return _carCount; }

    bool fireDetected()           const { return _fireDetected; }

    uint8_t objDetDetections()       const { return _objDetDetections; }
    uint16_t objDet0ClassID()         const { return _objDet0ClassID; }
    uint16_t objDet0UniqueID()        const { return _objDet0UniqueID; }
    float objDet0Lat()             const { return _objDet0Lat; }
    float objDet0Lon()             const { return _objDet0Lon; }
    float objDet0Alt()             const { return _objDet0Alt; }

    uint16_t SDCardDetected()         const { return _SDCardDetected; }
    float SDCardTotal()            const { return _SDCardTotal; }
    float SDCardAvailable()        const { return _SDCardAvailable; }

    bool systemTracking()         const { return _systemTracking; }
    bool systemRecording()        const { return _systemRecording; }
    uint8_t systemMode()             const { return _systemMode; }
    bool systemSnapshotStatus()   const { return _systemSnapshotStatus; }

    float trackingLat()            const { return _trackingLat; }
    float trackingLon()            const { return _trackingLon; }
    float trackingAlt()            const { return _trackingAlt; }

    uint16_t videoBitrateStream0()    const { return _videoBitrateStream0; }
    uint16_t videoBitrateRecord0()    const { return _videoBitrateRecord0; }

    float VMDLat()                 const { return _VMDLat; }
    float VMDLon()                 const { return _VMDLon; }
    float VMDAlt()                 const { return _VMDAlt; }

signals:
    void streamChanged();
    void gndCrsReceived(float lat, float lon);

private:
    explicit NextVisionData(QObject* parent = nullptr);

    void _handleARMarkersMsg    (const mavlink_message_t* message);
    void _handleCarCountMsg     (const mavlink_message_t* message);
    void _handleFireMsg         (const mavlink_message_t* message);
    void _handleGndCrsMsg       (const mavlink_message_t* message);
    void _handleIMUMsg          (const mavlink_message_t* message);
    void _handleLosMsg          (const mavlink_message_t* message);
    void _handleLosDirRateMsg   (const mavlink_message_t* message);
    void _handleLPRMsg          (const mavlink_message_t* message);
    void _handleObjDetMsg       (const mavlink_message_t* message);
    void _handleOGLRMsg         (const mavlink_message_t* message);
    void _handleParameterMsg    (const mavlink_message_t* message);
    void _handleSDCardMsg       (const mavlink_message_t* message);
    void _handleSnapshotMsg     (const mavlink_message_t* message);
    void _handleSysMsg          (const mavlink_message_t* message);
    void _handleTrackingMsg     (const mavlink_message_t* message);
    void _handleVideoMsg        (const mavlink_message_t* message);
    void _handleVMDMsg          (const mavlink_message_t* message);
    void _handlePLRMsg          (const mavlink_message_t* message);
    void _handleRangeFinderMsg  (const mavlink_message_t* message);
    void _handleAuxCamObjDetMsg (const mavlink_message_t* message);
    uint16_t _get_report_type   (const mavlink_message_t* message);

    float _gndCrsAlt;

    float _videoDestCh0;
    float _videoDestPort;
    float _enableBandWidthLimit;
    float _bandwithLimit;
    float _streamBitrate;
    float _recordingBitrate;
    float _ethIPAddr;
    float _ethSubnetMask;
    float _ethGatewayIP;
    float _MTU;
    float _enableRollDerot;
    float _autoRecordEnable;
    float _autoRecordINSTimeout;
    float _enableAES;
    float _AES128Key;
    float _remoteIP;
    float _remotePort;
    float _localPort;
    float _runFlightPlan;
    float _forceRollToHorizon;
    float _OSDMode;
    float _showGndCrsInfo;
    float _showGPSInfo;
    float _showFlightMode;
    float _showAirSpeed;
    float _OSDMiniMap;
    float _VGAResizingThreshold;
    float _VGAOutputMode;
    float _reportSystemID;
    float _reportComponentID;
    float _ackSystemID;
    float _ackComponentID;
    float _enableMavSequenceNum;
    float _streamGOPSize;
    float _streamQuantizationVal;
    float _enableObjDet;
    float _objDetType;
    float _objDetAutoTrackEnable;
    float _objDetAutoZoomOnTarget;
    float _objDetLoiterAPOnDetect;
    float _enableALPR;
    float _ALPRCountryProfile;
    float _AILicenseStatus;

    uint32_t _carCount;

    bool _fireDetected;

    uint8_t _objDetDetections;
    uint16_t _objDet0ClassID;
    uint16_t _objDet0UniqueID;
    float _objDet0Lat;
    float _objDet0Lon;
    float _objDet0Alt;

    uint16_t _SDCardDetected;
    float _SDCardTotal;
    float _SDCardAvailable;

    bool _systemTracking;
    bool _systemRecording;
    uint8_t _systemMode;
    bool _systemSnapshotStatus;

    float _trackingLat;
    float _trackingLon;
    float _trackingAlt;

    uint16_t _videoBitrateStream0;
    uint16_t _videoBitrateRecord0;

    float _VMDLat;
    float _VMDLon;
    float _VMDAlt;

    QUrl m_stream{};
    const QString m_streamUrl = "rtsp://%1:554/video";
};
