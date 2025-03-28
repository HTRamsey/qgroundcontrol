/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGroundControlQmlGlobal.h"

#include "ADSBVehicleManager.h"
#include "AppSettings.h"
#include "EditPositionDialogController.h"
#include "ElevationMapProvider.h"
#include "FirmwarePluginManager.h"
#include "FlightMapSettings.h"
#include "FlightPathSegment.h"
#include "HorizontalFactValueGrid.h"
#include "InstrumentValueData.h"
#include "LinkManager.h"
#include "MavlinkAction.h"
#include "MavlinkActionManager.h"
#include "MissionCommandTree.h"
#include "MultiVehicleManager.h"
#include "ParameterEditorController.h"
#include "PositionManager.h"
#include "QGCApplication.h"
#include "QGCCorePlugin.h"
#include "QGCFileDialogController.h"
#include "QGCGeoBoundingCube.h"
#include "QGCLoggingCategory.h"
#include "QGCMapCircle.h"
#include "QGCMapEngineManager.h"
#include "QGCMapPalette.h"
#include "QGCMapPolygon.h"
#include "QGCPalette.h"
#include "QmlObjectListModel.h"
#include "QmlUnitsConversion.h"
#include "RCChannelMonitorController.h"
#include "RCToParamDialogController.h"
#include "ScreenToolsController.h"
#include "SettingsManager.h"
#include "TerrainProfile.h"
#include "ToolStripAction.h"
#include "ToolStripActionList.h"
#include "VideoManager.h"
#ifndef QGC_NO_SERIAL_LINK
#include "GPSManager.h"
#include "GPSRtk.h"
#endif
#ifdef QT_DEBUG
#include "MockLink.h"
#endif
#ifndef QGC_AIRLINK_DISABLED
#include "AirLinkManager.h"
#endif
#ifdef QGC_UTM_ADAPTER
#include "UTMSPManager.h"
#endif

#include <QtCore/QLineF>
#include <QtCore/QSettings>

QGC_LOGGING_CATEGORY(QGroundControlQmlGlobalLog, "qgc.qmlcontrols.qgroundcontrolqmlglobal")

void QGroundControlQmlGlobal::registerQmlTypes()
{
    (void) qmlRegisterUncreatableType<FactValueGrid>        ("QGroundControl.Templates",             1, 0, "FactValueGrid",             "Reference only");
    (void) qmlRegisterUncreatableType<FlightPathSegment>    ("QGroundControl",                       1, 0, "FlightPathSegment",         "Reference only");
    (void) qmlRegisterUncreatableType<InstrumentValueData>  ("QGroundControl",                       1, 0, "InstrumentValueData",       "Reference only");
    (void) qmlRegisterUncreatableType<QGCGeoBoundingCube>   ("QGroundControl.FlightMap",             1, 0, "QGCGeoBoundingCube",        "Reference only");
    (void) qmlRegisterUncreatableType<QGCMapPolygon>        ("QGroundControl.FlightMap",             1, 0, "QGCMapPolygon",             "Reference only");
    (void) qmlRegisterUncreatableType<QmlObjectListModel>   ("QGroundControl",                       1, 0, "QmlObjectListModel",        "Reference only");

    (void) qmlRegisterType<EditPositionDialogController>    ("QGroundControl.Controllers",           1, 0, "EditPositionDialogController");
    (void) qmlRegisterType<HorizontalFactValueGrid>         ("QGroundControl.Templates",             1, 0, "HorizontalFactValueGrid");
    (void) qmlRegisterType<MavlinkAction>                   ("QGroundControl.Controllers",           1, 0, "MavlinkAction");
    (void) qmlRegisterType<MavlinkActionManager>            ("QGroundControl.Controllers",           1, 0, "MavlinkActionManager");
    (void) qmlRegisterType<ParameterEditorController>       ("QGroundControl.Controllers",           1, 0, "ParameterEditorController");
    (void) qmlRegisterType<QGCFileDialogController>         ("QGroundControl.Controllers",           1, 0, "QGCFileDialogController");
    (void) qmlRegisterType<QGCMapCircle>                    ("QGroundControl.FlightMap",             1, 0, "QGCMapCircle");
    (void) qmlRegisterType<QGCMapPalette>                   ("QGroundControl.Palette",               1, 0, "QGCMapPalette");
    (void) qmlRegisterType<QGCPalette>                      ("QGroundControl.Palette",               1, 0, "QGCPalette");
    (void) qmlRegisterType<RCChannelMonitorController>      ("QGroundControl.Controllers",           1, 0, "RCChannelMonitorController");
    (void) qmlRegisterType<RCToParamDialogController>       ("QGroundControl.Controllers",           1, 0, "RCToParamDialogController");
    (void) qmlRegisterType<TerrainProfile>                  ("QGroundControl.Controls",              1, 0, "TerrainProfile");
    (void) qmlRegisterType<ToolStripAction>                 ("QGroundControl.Controls",              1, 0, "ToolStripAction");
    (void) qmlRegisterType<ToolStripActionList>             ("QGroundControl.Controls",              1, 0, "ToolStripActionList");

    (void) qmlRegisterSingletonType<QGroundControlQmlGlobal>("QGroundControl", 1, 0, "QGroundControl", [](QQmlEngine *, QJSEngine *) -> QObject * {
        return new QGroundControlQmlGlobal();
    });
    (void) qmlRegisterSingletonType<ScreenToolsController>("QGroundControl.ScreenToolsController", 1, 0, "ScreenToolsController", [](QQmlEngine *, QJSEngine *) -> QObject * {
        return new ScreenToolsController();
    });
}

QGroundControlQmlGlobal::QGroundControlQmlGlobal(QObject *parent)
    : QObject(parent)
    , _mapEngineManager(QGCMapEngineManager::instance())
    , _adsbVehicleManager(ADSBVehicleManager::instance())
    , _qgcPositionManager(QGCPositionManager::instance())
    , _missionCommandTree(MissionCommandTree::instance())
    , _videoManager(VideoManager::instance())
    , _linkManager(LinkManager::instance())
    , _multiVehicleManager(MultiVehicleManager::instance())
    , _settingsManager(SettingsManager::instance())
    , _corePlugin(QGCCorePlugin::instance())
    , _globalPalette(new QGCPalette(this))
    , _unitsConversion(new QmlUnitsConversion(this))
#ifndef QGC_NO_SERIAL_LINK
    , _gpsRtkFactGroup(GPSManager::instance()->gpsRtk()->gpsRtkFactGroup())
#endif
#ifndef QGC_AIRLINK_DISABLED
    , _airlinkManager(AirLinkManager::instance())
#endif
#ifdef QGC_UTM_ADAPTER
    , _utmspManager(UTMSPManager::instance())
#endif
{
    // qCDebug(QmlObjectListModelLog) << Q_FUNC_INFO << this;

    QSettings settings;
    settings.beginGroup(_kFlightMapPositionSettingsGroup);
    _coord.setLatitude(settings.value(_kFlightMapPositionLatitudeSettingsKey, _coord.latitude()).toDouble());
    _coord.setLongitude(settings.value(_kFlightMapPositionLongitudeSettingsKey, _coord.longitude()).toDouble());
    _zoom = settings.value(_kFlightMapZoomSettingsKey, _zoom).toDouble();
    settings.endGroup();

    _flightMapPositionSettledTimer.setSingleShot(true);
    _flightMapPositionSettledTimer.setInterval(1000);

    (void) connect(&_flightMapPositionSettledTimer, &QTimer::timeout, [this]() {
        QSettings settings;
        settings.beginGroup(_kFlightMapPositionSettingsGroup);
        settings.setValue(_kFlightMapPositionLatitudeSettingsKey, _coord.latitude());
        settings.setValue(_kFlightMapPositionLongitudeSettingsKey, _coord.longitude());
        settings.setValue(_kFlightMapZoomSettingsKey, _zoom);
        settings.endGroup();
    });

    (void) connect(this, &QGroundControlQmlGlobal::flightMapPositionChanged, this, [this](QGeoCoordinate) {
        if (!_flightMapPositionSettledTimer.isActive()) {
            _flightMapPositionSettledTimer.start();
        }
    });
    (void) connect(this, &QGroundControlQmlGlobal::flightMapZoomChanged, this, [this](double) {
        if (!_flightMapPositionSettledTimer.isActive()) {
            _flightMapPositionSettledTimer.start();
        }
    });
}

QGroundControlQmlGlobal::~QGroundControlQmlGlobal()
{
    // qCDebug(QGroundControlQmlGlobalLog) << Q_FUNC_INFO << this;
}

void QGroundControlQmlGlobal::saveGlobalSetting(QStringView key, const QString &value)
{
    QSettings settings;
    settings.beginGroup(_kQmlGlobalKeyName);
    settings.setValue(key, value);
}

QString QGroundControlQmlGlobal::loadGlobalSetting(QStringView key, const QString &defaultValue)
{
    QSettings settings;
    settings.beginGroup(_kQmlGlobalKeyName);
    return settings.value(key, defaultValue).toString();
}

void QGroundControlQmlGlobal::saveBoolGlobalSetting(QStringView key, bool value)
{
    QSettings settings;
    settings.beginGroup(_kQmlGlobalKeyName);
    settings.setValue(key, value);
}

bool QGroundControlQmlGlobal::loadBoolGlobalSetting(QStringView key, bool defaultValue)
{
    QSettings settings;
    settings.beginGroup(_kQmlGlobalKeyName);
    return settings.value(key, defaultValue).toBool();
}

void QGroundControlQmlGlobal::startPX4MockLink(bool sendStatusText)
{
#ifdef QT_DEBUG
    MockLink::startPX4MockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::startGenericMockLink(bool sendStatusText)
{
#ifdef QT_DEBUG
    MockLink::startGenericMockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::startAPMArduCopterMockLink(bool sendStatusText)
{
#ifdef QT_DEBUG
    MockLink::startAPMArduCopterMockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::startAPMArduPlaneMockLink(bool sendStatusText)
{
#ifdef QT_DEBUG
    MockLink::startAPMArduPlaneMockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::startAPMArduSubMockLink(bool sendStatusText)
{
#ifdef QT_DEBUG
    MockLink::startAPMArduSubMockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::startAPMArduRoverMockLink(bool sendStatusText)
{
#ifdef QT_DEBUG
    MockLink::startAPMArduRoverMockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::stopOneMockLink()
{
#ifdef QT_DEBUG
    QList<SharedLinkInterfacePtr> sharedLinks = LinkManager::instance()->links();

    for (int i=0; i<sharedLinks.count(); i++) {
        LinkInterface* link = sharedLinks[i].get();
        MockLink* mockLink = qobject_cast<MockLink*>(link);
        if (mockLink) {
            mockLink->disconnect();
            return;
        }
    }
#endif
}

bool QGroundControlQmlGlobal::singleFirmwareSupport()
{
    return (FirmwarePluginManager::instance()->supportedFirmwareClasses().count() == 1);
}

bool QGroundControlQmlGlobal::singleVehicleSupport()
{
    if (singleFirmwareSupport()) {
        return (FirmwarePluginManager::instance()->supportedVehicleClasses(FirmwarePluginManager::instance()->supportedFirmwareClasses()[0]).count() == 1);
    }

    return false;
}

bool QGroundControlQmlGlobal::px4ProFirmwareSupported()
{
    return FirmwarePluginManager::instance()->supportedFirmwareClasses().contains(QGCMAVLink::FirmwareClassPX4);
}

bool QGroundControlQmlGlobal::apmFirmwareSupported()
{
    return FirmwarePluginManager::instance()->supportedFirmwareClasses().contains(QGCMAVLink::FirmwareClassArduPilot);
}

QStringList QGroundControlQmlGlobal::loggingCategories()
{
    return QGCLoggingCategoryRegister::instance()->registeredCategories();
}

void QGroundControlQmlGlobal::setCategoryLoggingOn(const QString &category, bool enable)
{
    QGCLoggingCategoryRegister::instance()->setCategoryLoggingOn(category, enable);
}

bool QGroundControlQmlGlobal::categoryLoggingOn(const QString &category)
{
    return QGCLoggingCategoryRegister::instance()->categoryLoggingOn(category);
}

void QGroundControlQmlGlobal::updateLoggingFilterRules()
{
    QGCLoggingCategoryRegister::instance()->setFilterRulesFromSettings(QString());
}

bool QGroundControlQmlGlobal::linesIntersect(const QPointF &line1A, const QPointF &line1B, const QPointF &line2A, const QPointF &line2B)
{
    QPointF intersectPoint;
    const QLineF::IntersectionType intersect = QLineF(line1A, line1B).intersects(QLineF(line2A, line2B), &intersectPoint);

    return ((intersect == QLineF::BoundedIntersection) && (intersectPoint != line1A) && (intersectPoint != line1B));
}

void QGroundControlQmlGlobal::setFlightMapPosition(QGeoCoordinate coordinate)
{
    if (coordinate != flightMapPosition()) {
        _coord.setLatitude(coordinate.latitude());
        _coord.setLongitude(coordinate.longitude());
        emit flightMapPositionChanged(coordinate);
    }
}

void QGroundControlQmlGlobal::setFlightMapZoom(double zoom)
{
    if (zoom != flightMapZoom()) {
        _zoom = zoom;
        emit flightMapZoomChanged(zoom);
    }
}

QString QGroundControlQmlGlobal::altitudeModeExtraUnits(AltMode altMode)
{
    switch (altMode) {
    case AltitudeModeAbsolute:
        return tr("(AMSL)");
    case AltitudeModeCalcAboveTerrain:
        return tr("(CalcT)");
    case AltitudeModeTerrainFrame:
        return tr("(TerrF)");
    case AltitudeModeMixed:
        qCWarning(QGroundControlQmlGlobalLog) << "Internal Error: QGroundControlQmlGlobal::altitudeModeExtraUnits called with altMode == AltitudeModeMixed";
        return QString();
    case AltitudeModeNone:
    case AltitudeModeRelative:
        // Showing (Rel) all the time ends up being too noisy
    default:
        return QString();
    }
}

QString QGroundControlQmlGlobal::altitudeModeShortDescription(AltMode altMode)
{
    switch (altMode) {
    case AltitudeModeRelative:
        return tr("Relative To Launch");
    case AltitudeModeAbsolute:
        return tr("AMSL");
    case AltitudeModeCalcAboveTerrain:
        return tr("Calc Above Terrain");
    case AltitudeModeTerrainFrame:
        return tr("Terrain Frame");
    case AltitudeModeMixed:
        return tr("Mixed Modes");
    case AltitudeModeNone:
    default:
        return QString();
    }
}

QString QGroundControlQmlGlobal::elevationProviderNotice()
{
    if (SettingsManager::instance()->flightMapSettings()->elevationMapProvider()->rawValue().toString() == QString(CopernicusElevationProvider::kProviderKey)) {
        return QString(CopernicusElevationProvider::kProviderNotice);
    }
    return QString();
}

void QGroundControlQmlGlobal::deleteAllSettingsNextBoot()
{
    QGCApplication::deleteAllSettingsNextBoot();
}

void QGroundControlQmlGlobal::clearDeleteAllSettingsNextBoot()
{
    QGCApplication::clearDeleteAllSettingsNextBoot();
}
