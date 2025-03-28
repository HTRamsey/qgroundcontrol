/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>
#include <QtCore/QPointF>
#include <QtCore/QTimer>
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

    Q_PROPERTY(ADSBVehicleManager*  adsbVehicleManager          READ    adsbVehicleManager          CONSTANT)
    Q_PROPERTY(LinkManager*         linkManager                 READ    linkManager                 CONSTANT)
    Q_PROPERTY(MissionCommandTree*  missionCommandTree          READ    missionCommandTree          CONSTANT)
    Q_PROPERTY(MultiVehicleManager* multiVehicleManager         READ    multiVehicleManager         CONSTANT)
    Q_PROPERTY(QGCCorePlugin*       corePlugin                  READ    corePlugin                  CONSTANT)
    Q_PROPERTY(QGCMapEngineManager* mapEngineManager            READ    mapEngineManager            CONSTANT)
    Q_PROPERTY(QGCPalette*          globalPalette               MEMBER  _globalPalette              CONSTANT)
    Q_PROPERTY(QGCPositionManager*  qgcPositionManger           READ    qgcPositionManger           CONSTANT)
    Q_PROPERTY(QmlUnitsConversion*  unitsConversion             READ    unitsConversion             CONSTANT)
    Q_PROPERTY(SettingsManager*     settingsManager             READ    settingsManager             CONSTANT)
    Q_PROPERTY(VideoManager*        videoManager                READ    videoManager                CONSTANT)

    Q_PROPERTY(bool                 singleFirmwareSupport       READ    singleFirmwareSupport       CONSTANT)
    Q_PROPERTY(bool                 singleVehicleSupport        READ    singleVehicleSupport        CONSTANT)
    Q_PROPERTY(bool                 px4ProFirmwareSupported     READ    px4ProFirmwareSupported     CONSTANT)
    Q_PROPERTY(int                  apmFirmwareSupported        READ    apmFirmwareSupported        CONSTANT)

    Q_PROPERTY(QGeoCoordinate       flightMapPosition           READ    flightMapPosition           WRITE setFlightMapPosition  NOTIFY flightMapPositionChanged)
    Q_PROPERTY(double               flightMapZoom               READ    flightMapZoom               WRITE setFlightMapZoom      NOTIFY flightMapZoomChanged)
    Q_PROPERTY(double               flightMapInitialZoom        MEMBER  _flightMapInitialZoom       CONSTANT)

    Q_PROPERTY(qreal                zOrderTopMost               READ    zOrderTopMost               CONSTANT)
    Q_PROPERTY(qreal                zOrderWidgets               READ    zOrderWidgets               CONSTANT)
    Q_PROPERTY(qreal                zOrderMapItems              READ    zOrderMapItems              CONSTANT)
    Q_PROPERTY(qreal                zOrderVehicles              READ    zOrderVehicles              CONSTANT)
    Q_PROPERTY(qreal                zOrderWaypointIndicators    READ    zOrderWaypointIndicators    CONSTANT)
    Q_PROPERTY(qreal                zOrderTrajectoryLines       READ    zOrderTrajectoryLines       CONSTANT)
    Q_PROPERTY(qreal                zOrderWaypointLines         READ    zOrderWaypointLines         CONSTANT)

    Q_PROPERTY(QString              elevationProviderNotice     READ    elevationProviderNotice     CONSTANT)

    Q_PROPERTY(bool                 hasAPMSupport               READ    hasAPMSupport               CONSTANT)
    Q_PROPERTY(bool                 hasMAVLinkInspector         READ    hasMAVLinkInspector         CONSTANT)

    Q_PROPERTY(bool                 utmspSupported              READ    utmspSupported              CONSTANT)
#ifdef QGC_UTM_ADAPTER
    Q_MOC_INCLUDE("UTMSPManager.h")
    Q_PROPERTY(UTMSPManager*        utmspManager                READ    utmspManager                CONSTANT)
#endif
    Q_PROPERTY(bool                 airlinkSupported            READ    airlinkSupported            CONSTANT)
#ifndef QGC_AIRLINK_DISABLED
    Q_MOC_INCLUDE("AirLinkManager.h")
    Q_PROPERTY(AirLinkManager*      airlinkManager              READ    airlinkManager              CONSTANT)
#endif
#ifndef QGC_NO_SERIAL_LINK
    Q_PROPERTY(FactGroup*           gpsRtk                      READ    gpsRtkFactGroup             CONSTANT)
#endif

public:
    explicit QGroundControlQmlGlobal(QObject *parent = nullptr);
    ~QGroundControlQmlGlobal();

    static void registerQmlTypes();

    Q_INVOKABLE static void saveGlobalSetting(QStringView key, const QString &value);
    Q_INVOKABLE static QString loadGlobalSetting(QStringView key, const QString &defaultValue);
    Q_INVOKABLE static void saveBoolGlobalSetting(QStringView key, bool value);
    Q_INVOKABLE static bool loadBoolGlobalSetting(QStringView key, bool defaultValue);

    Q_INVOKABLE void deleteAllSettingsNextBoot();
    Q_INVOKABLE void clearDeleteAllSettingsNextBoot();

    Q_INVOKABLE static void startPX4MockLink(bool sendStatusText);
    Q_INVOKABLE static void startGenericMockLink(bool sendStatusText);
    Q_INVOKABLE static void startAPMArduCopterMockLink(bool sendStatusText);
    Q_INVOKABLE static void startAPMArduPlaneMockLink(bool sendStatusText);
    Q_INVOKABLE static void startAPMArduSubMockLink(bool sendStatusText);
    Q_INVOKABLE static void startAPMArduRoverMockLink(bool sendStatusText);
    Q_INVOKABLE static void stopOneMockLink();

    /// Returns the list of available logging category names.
    Q_INVOKABLE static QStringList loggingCategories();

    /// Turns on/off logging for the specified category. State is saved in app settings.
    Q_INVOKABLE static void setCategoryLoggingOn(const QString &category, bool enable);

    /// Returns true if logging is turned on for the specified category.
    Q_INVOKABLE static bool categoryLoggingOn(const QString &category);

    /// Updates the logging filter rules after settings have changed
    Q_INVOKABLE static void updateLoggingFilterRules();

    Q_INVOKABLE static bool linesIntersect(const QPointF &xLine1, const QPointF &yLine1, const QPointF &xLine2, const QPointF &yLine2);

    enum AltMode {
        AltitudeModeMixed,              // Used by global altitude mode for mission planning
        AltitudeModeRelative,           // MAV_FRAME_GLOBAL_RELATIVE_ALT
        AltitudeModeAbsolute,           // MAV_FRAME_GLOBAL
        AltitudeModeCalcAboveTerrain,   // Absolute altitude above terrain calculated from terrain data
        AltitudeModeTerrainFrame,       // MAV_FRAME_GLOBAL_TERRAIN_ALT
        AltitudeModeNone,               // Being used as distance value unrelated to ground (for example distance to structure)
    };
    Q_ENUM(AltMode)
    Q_INVOKABLE QString altitudeModeExtraUnits(AltMode altMode);        ///< String shown in the FactTextField.extraUnits ui
    Q_INVOKABLE static QString altitudeModeShortDescription(AltMode altMode);  ///< String shown when a user needs to select an altitude mode

    LinkManager *linkManager() const { return _linkManager; }
    MultiVehicleManager *multiVehicleManager() const { return _multiVehicleManager; }
    QGCMapEngineManager *mapEngineManager() const { return _mapEngineManager; }
    QGCPositionManager *qgcPositionManger() const { return _qgcPositionManager; }
    MissionCommandTree *missionCommandTree() const { return _missionCommandTree; }
    VideoManager *videoManager() const { return _videoManager; }
    QGCCorePlugin *corePlugin() const { return _corePlugin; }
    SettingsManager *settingsManager() const { return _settingsManager; }
    ADSBVehicleManager *adsbVehicleManager() const { return _adsbVehicleManager; }
    QmlUnitsConversion *unitsConversion() const { return _unitsConversion; }

    QGeoCoordinate flightMapPosition() const { return _coord; }
    double flightMapZoom() const { return _zoom; }

    static qreal zOrderTopMost() { return 1000; }  ///< z order for top most items, toolbar, main window sub view
    static qreal zOrderWidgets() { return 100; }  ///< z order value to widgets, for example: zoom controls, hud widgetss
    static qreal zOrderMapItems() { return 50; }
    static qreal zOrderWaypointIndicators() { return 50; }
    static qreal zOrderVehicles() { return 49; }
    static qreal zOrderTrajectoryLines() { return 48; }
    static qreal zOrderWaypointLines() { return 47; }

    static QString elevationProviderNotice();

    bool singleFirmwareSupport();
    bool singleVehicleSupport();
    bool px4ProFirmwareSupported();
    bool apmFirmwareSupported();

    void setFlightMapPosition(QGeoCoordinate coordinate);
    void setFlightMapZoom(double zoom);

#ifdef QGC_NO_ARDUPILOT_DIALECT
    static bool hasAPMSupport() { return false; }
#else
    static bool hasAPMSupport() { return true; }
#endif

#ifdef QGC_DISABLE_MAVLINK_INSPECTOR
    static bool hasMAVLinkInspector() { return false; }
#else
    static bool hasMAVLinkInspector() { return true; }
#endif

#ifndef QGC_NO_SERIAL_LINK
    FactGroup *gpsRtkFactGroup() const { return _gpsRtkFactGroup; }
#endif

#ifndef QGC_AIRLINK_DISABLED
    AirLinkManager *airlinkManager() const { return _airlinkManager; }
    static bool airlinkSupported() { return true; }
#else
    static bool airlinkSupported() { return false; }
#endif

#ifdef QGC_UTM_ADAPTER
    UTMSPManager *utmspManager() const { return _utmspManager; }
    static bool utmspSupported() { return true; }
#else
    static bool utmspSupported() { return false; }
#endif

signals:
    void isMultiplexingEnabledChanged(bool enabled);
    void mavlinkSystemIDChanged(int id);
    void flightMapPositionChanged(QGeoCoordinate flightMapPosition);
    void flightMapZoomChanged(double flightMapZoom);

private:
    QGCMapEngineManager *_mapEngineManager = nullptr;
    ADSBVehicleManager *_adsbVehicleManager = nullptr;
    QGCPositionManager *_qgcPositionManager = nullptr;
    MissionCommandTree *_missionCommandTree = nullptr;
    VideoManager *_videoManager = nullptr;
    LinkManager *_linkManager = nullptr;
    MultiVehicleManager *_multiVehicleManager = nullptr;
    SettingsManager *_settingsManager = nullptr;
    QGCCorePlugin *_corePlugin = nullptr;
    QGCPalette *_globalPalette = nullptr; ///< This palette will always return enabled colors
    QmlUnitsConversion *_unitsConversion = nullptr;
#ifndef QGC_NO_SERIAL_LINK
    FactGroup *_gpsRtkFactGroup = nullptr;
#endif
#ifndef QGC_AIRLINK_DISABLED
    AirLinkManager *_airlinkManager = nullptr;
#endif
#ifdef QGC_UTM_ADAPTER
    UTMSPManager *_utmspManager = nullptr;
#endif

    QTimer _flightMapPositionSettledTimer;
    QGeoCoordinate _coord = QGeoCoordinate(0.0,0.0);
    double _zoom = 2.0;
    static constexpr double _flightMapInitialZoom = 17.0;

    static constexpr const char *_kQmlGlobalKeyName = "QGCQml";
    static constexpr const char *_kFlightMapPositionSettingsGroup = "FlightMapPosition";
    static constexpr const char *_kFlightMapPositionLatitudeSettingsKey = "Latitude";
    static constexpr const char *_kFlightMapPositionLongitudeSettingsKey = "Longitude";
    static constexpr const char *_kFlightMapZoomSettingsKey = "FlightMapZoom";
};
