/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtPositioning/QGeoCoordinate>

class ADSBVehicleManager;
class AirLinkManager;
class FactGroup;
class LinkManager;
class MissionCommandTree;
class MultiVehicleManager;
class QGCCorePlugin;
class QGCMapEngineManager;
class QGCPalette;
class QGCPositionManager;
class QmlUnitsConversion;
class SettingsManager;
class UTMSPManager;
class VideoManager;

Q_DECLARE_LOGGING_CATEGORY(QGroundControlQmlGlobalLog)

class QGroundControlQmlGlobal : public QObject
{
    Q_OBJECT
    Q_MOC_INCLUDE("ADSBVehicleManager.h")
    Q_MOC_INCLUDE("FactGroup.h")
    Q_MOC_INCLUDE("LinkManager.h")
    Q_MOC_INCLUDE("MissionCommandTree.h")
    Q_MOC_INCLUDE("MultiVehicleManager.h")
    Q_MOC_INCLUDE("PositionManager.h")
    Q_MOC_INCLUDE("QGCCorePlugin.h")
    Q_MOC_INCLUDE("QGCMapEngineManager.h")
    Q_MOC_INCLUDE("QGCPalette.h")
    Q_MOC_INCLUDE("QmlUnitsConversion.h")
    Q_MOC_INCLUDE("SettingsManager.h")
    Q_MOC_INCLUDE("VideoManager.h")
#ifdef QGC_UTM_ADAPTER
    Q_MOC_INCLUDE("UTMSPManager.h")
#endif
#ifndef QGC_AIRLINK_DISABLED
    Q_MOC_INCLUDE("AirLinkManager.h")
#endif
    Q_PROPERTY(ADSBVehicleManager *adsbVehicleManager READ adsbVehicleManager CONSTANT)
#ifndef QGC_AIRLINK_DISABLED
    Q_PROPERTY(AirLinkManager *airlinkManager READ airlinkManager CONSTANT)
#endif
#ifndef NO_SERIAL_LINK
    Q_PROPERTY(FactGroup *gpsRtk READ gpsRtkFactGroup CONSTANT)
#endif
    Q_PROPERTY(LinkManager *linkManager READ linkManager CONSTANT)
    Q_PROPERTY(MissionCommandTree *missionCommandTree READ missionCommandTree CONSTANT)
    Q_PROPERTY(MultiVehicleManager *multiVehicleManager READ multiVehicleManager CONSTANT)
    Q_PROPERTY(QGCCorePlugin *corePlugin READ corePlugin CONSTANT)
    Q_PROPERTY(QGCMapEngineManager *mapEngineManager READ mapEngineManager CONSTANT)
    Q_PROPERTY(QGCPalette *globalPalette MEMBER _globalPalette CONSTANT)   ///< This palette will always return enabled colors
    Q_PROPERTY(QGCPositionManager *qgcPositionManger READ qgcPositionManger CONSTANT)
    Q_PROPERTY(QmlUnitsConversion *unitsConversion READ unitsConversion CONSTANT)
    Q_PROPERTY(SettingsManager *settingsManager READ settingsManager CONSTANT)
#ifdef QGC_UTM_ADAPTER
    Q_PROPERTY(UTMSPManager *utmspManager READ utmspManager CONSTANT)
#endif
    Q_PROPERTY(VideoManager *videoManager READ videoManager CONSTANT)
    Q_PROPERTY(bool airlinkSupported READ airlinkSupported CONSTANT)
    Q_PROPERTY(bool hasAPMSupport READ hasAPMSupport CONSTANT)
    Q_PROPERTY(bool hasMAVLinkInspector READ hasMAVLinkInspector CONSTANT)
    Q_PROPERTY(bool isVersionCheckEnabled READ isVersionCheckEnabled WRITE setIsVersionCheckEnabled  NOTIFY isVersionCheckEnabledChanged)
    Q_PROPERTY(bool px4ProFirmwareSupported READ px4ProFirmwareSupported CONSTANT)
    Q_PROPERTY(bool singleFirmwareSupport READ singleFirmwareSupport CONSTANT)
    Q_PROPERTY(bool singleVehicleSupport READ singleVehicleSupport CONSTANT)
    Q_PROPERTY(bool skipSetupPage READ skipSetupPage WRITE setSkipSetupPage NOTIFY skipSetupPageChanged)
    Q_PROPERTY(bool utmspSupported READ utmspSupported CONSTANT)
    Q_PROPERTY(double flightMapInitialZoom MEMBER _flightMapInitialZoom CONSTANT)   ///< Zoom level to use when either gcs or vehicle shows up for first time
    Q_PROPERTY(double flightMapZoom READ flightMapZoom WRITE setFlightMapZoom NOTIFY flightMapZoomChanged)
    Q_PROPERTY(int apmFirmwareSupported READ apmFirmwareSupported CONSTANT)
    Q_PROPERTY(int mavlinkSystemID READ mavlinkSystemID WRITE setMavlinkSystemID NOTIFY mavlinkSystemIDChanged)
    Q_PROPERTY(QGeoCoordinate flightMapPosition READ flightMapPosition WRITE setFlightMapPosition NOTIFY flightMapPositionChanged)
    Q_PROPERTY(qreal zOrderMapItems READ zOrderMapItems CONSTANT)
    Q_PROPERTY(qreal zOrderTopMost READ zOrderTopMost CONSTANT) ///< z order for top most items, toolbar, main window sub view
    Q_PROPERTY(qreal zOrderTrajectoryLines READ zOrderTrajectoryLines CONSTANT)
    Q_PROPERTY(qreal zOrderVehicles READ zOrderVehicles CONSTANT)
    Q_PROPERTY(qreal zOrderWaypointIndicators READ zOrderWaypointIndicators CONSTANT)
    Q_PROPERTY(qreal zOrderWaypointLines READ zOrderWaypointLines CONSTANT)
    Q_PROPERTY(qreal zOrderWidgets READ zOrderWidgets CONSTANT) ///< z order value to widgets, for example: zoom controls, hud widgetss
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QString elevationProviderName READ elevationProviderName CONSTANT)
    Q_PROPERTY(QString elevationProviderNotice READ elevationProviderNotice CONSTANT)
    Q_PROPERTY(QString missionFileExtension READ missionFileExtension CONSTANT)
    Q_PROPERTY(QString parameterFileExtension READ parameterFileExtension CONSTANT)
    Q_PROPERTY(QString qgcVersion READ qgcVersion CONSTANT)
    Q_PROPERTY(QString telemetryFileExtension READ telemetryFileExtension CONSTANT)

public:
    QGroundControlQmlGlobal(QObject *parent = nullptr);
    ~QGroundControlQmlGlobal();

    static void registerQmlTypes();

    enum AltMode {
        AltitudeModeMixed,              ///< Used by global altitude mode for mission planning
        AltitudeModeRelative,           ///< MAV_FRAME_GLOBAL_RELATIVE_ALT
        AltitudeModeAbsolute,           ///< MAV_FRAME_GLOBAL
        AltitudeModeCalcAboveTerrain,   ///< Absolute altitude above terrain calculated from terrain data
        AltitudeModeTerrainFrame,       ///< MAV_FRAME_GLOBAL_TERRAIN_ALT
        AltitudeModeNone,               ///< Being used as distance value unrelated to ground (for example distance to structure)
    };
    Q_ENUM(AltMode)

    Q_INVOKABLE void saveGlobalSetting(const QString &key, const QString &value) const;
    Q_INVOKABLE QString loadGlobalSetting(const QString &key, const QString &defaultValue) const;
    Q_INVOKABLE void saveBoolGlobalSetting(const QString &key, bool value) const;
    Q_INVOKABLE bool loadBoolGlobalSetting(const QString &key, bool defaultValue) const;

    Q_INVOKABLE void deleteAllSettingsNextBoot() const;
    Q_INVOKABLE void clearDeleteAllSettingsNextBoot() const;

    Q_INVOKABLE void startPX4MockLink(bool sendStatusText) const;
    Q_INVOKABLE void startGenericMockLink(bool sendStatusText) const;
    Q_INVOKABLE void startAPMArduCopterMockLink(bool sendStatusText) const;
    Q_INVOKABLE void startAPMArduPlaneMockLink(bool sendStatusText) const;
    Q_INVOKABLE void startAPMArduSubMockLink(bool sendStatusText) const;
    Q_INVOKABLE void startAPMArduRoverMockLink(bool sendStatusText) const;
    Q_INVOKABLE void stopOneMockLink() const;

    /// Returns the list of available logging category names.
    Q_INVOKABLE QStringList loggingCategories() const;

    /// Turns on/off logging for the specified category. State is saved in app settings.
    Q_INVOKABLE void setCategoryLoggingOn(const QString &category, bool enable) const;

    /// Returns true if logging is turned on for the specified category.
    Q_INVOKABLE bool categoryLoggingOn(const QString &category) const;

    /// Updates the logging filter rules after settings have changed
    Q_INVOKABLE void updateLoggingFilterRules() const;

    Q_INVOKABLE bool linesIntersect(QPointF xLine1, QPointF yLine1, QPointF xLine2, QPointF yLine2) const;

    /// String shown in the FactTextField.extraUnits ui
    Q_INVOKABLE QString altitudeModeExtraUnits(AltMode altMode) const;

    Q_INVOKABLE QString altitudeModeShortDescription(AltMode altMode) const;

    QString appName() const;
    LinkManager *linkManager() const { return _linkManager; }
    MultiVehicleManager *multiVehicleManager() const { return _multiVehicleManager; }
    QGCMapEngineManager *mapEngineManager() const { return _mapEngineManager; }
    QGCPositionManager *qgcPositionManger() const { return _qgcPositionManager; }
    MissionCommandTree *missionCommandTree() const { return _missionCommandTree; }
    VideoManager *videoManager() const { return _videoManager; }
    QGCCorePlugin *corePlugin() const { return _corePlugin; }
    SettingsManager *settingsManager() const { return _settingsManager; }
#ifndef NO_SERIAL_LINK
    FactGroup *gpsRtkFactGroup() const { return _gpsRtkFactGroup; }
#endif
    ADSBVehicleManager *adsbVehicleManager() const { return _adsbVehicleManager; }
    QmlUnitsConversion *unitsConversion() const { return _unitsConversion; }
    QGeoCoordinate flightMapPosition() const { return _coord; }
    double flightMapZoom() const { return _zoom; }

#ifndef QGC_AIRLINK_DISABLED
    AirLinkManager *airlinkManager() const { return _airlinkManager; }
    bool airlinkSupported() const { return true; }
#else
    bool airlinkSupported() const { return false; }
#endif

    qreal zOrderTopMost() const { return 1000; }
    qreal zOrderWidgets() const { return 100; }
    qreal zOrderMapItems() const { return 50; }
    qreal zOrderWaypointIndicators() const { return 50; }
    qreal zOrderVehicles() const { return 49; }
    qreal zOrderTrajectoryLines() const { return 48; }
    qreal zOrderWaypointLines() const { return 47; }

    bool isVersionCheckEnabled() const;
    int mavlinkSystemID() const;

#if defined(NO_ARDUPILOT_DIALECT)
    bool hasAPMSupport() const { return false; }
#else
    bool hasAPMSupport() const { return true; }
#endif

#if defined(QGC_DISABLE_MAVLINK_INSPECTOR)
    bool hasMAVLinkInspector() const { return false; }
#else
    bool hasMAVLinkInspector() const { return true; }
#endif

    QString elevationProviderName() const;
    QString elevationProviderNotice() const;

    bool singleFirmwareSupport() const;
    bool singleVehicleSupport() const;
    bool px4ProFirmwareSupported() const;
    bool apmFirmwareSupported() const;
    bool skipSetupPage() const { return _skipSetupPage; }
    void setSkipSetupPage(bool skip);

    void setIsVersionCheckEnabled(bool enable);
    void setMavlinkSystemID(int id);
    void setFlightMapPosition(const QGeoCoordinate &coordinate);
    void setFlightMapZoom(double zoom);

    QString parameterFileExtension() const;
    QString missionFileExtension() const;
    QString telemetryFileExtension() const;

    QString qgcVersion() const;

#ifdef QGC_UTM_ADAPTER
    UTMSPManager *utmspManager() const { return _utmspManager; }
    bool utmspSupported() const { return true; }
#else
    bool utmspSupported() const { return false; }
#endif

signals:
    void isMultiplexingEnabledChanged(bool enabled);
    void isVersionCheckEnabledChanged(bool enabled);
    void mavlinkSystemIDChanged(int id);
    void flightMapPositionChanged(QGeoCoordinate flightMapPosition);
    void flightMapZoomChanged(double flightMapZoom);
    void skipSetupPageChanged();

private:
    ADSBVehicleManager *_adsbVehicleManager = nullptr;
#ifndef QGC_AIRLINK_DISABLED
    AirLinkManager *_airlinkManager = nullptr;
#endif
    QGCCorePlugin *_corePlugin = nullptr;
    QGCPalette *_globalPalette = nullptr;
#ifndef NO_SERIAL_LINK
    FactGroup *_gpsRtkFactGroup = nullptr;
#endif
    LinkManager *_linkManager = nullptr;
    QGCMapEngineManager *_mapEngineManager = nullptr;
    MissionCommandTree *_missionCommandTree = nullptr;
    MultiVehicleManager *_multiVehicleManager = nullptr;
    QGCPositionManager *_qgcPositionManager = nullptr;
    SettingsManager *_settingsManager = nullptr;
    QmlUnitsConversion *_unitsConversion = nullptr;
#ifdef QGC_UTM_ADAPTER
    UTMSPManager *_utmspManager = nullptr;
#endif
    VideoManager *_videoManager = nullptr;

    QStringList _altitudeModeEnumString;
    double _flightMapInitialZoom = 17.0;
    bool _skipSetupPage = false;

    QGeoCoordinate _coord = QGeoCoordinate(0.0,0.0);
    double _zoom = 2.0;

    static constexpr const char *kQmlGlobalKeyName = "QGCQml";
    static constexpr const char *_flightMapPositionSettingsGroup = "FlightMapPosition";
    static constexpr const char *_flightMapPositionLatitudeSettingsKey = "Latitude";
    static constexpr const char *_flightMapPositionLongitudeSettingsKey = "Longitude";
    static constexpr const char *_flightMapZoomSettingsKey = "FlightMapZoom";
};
