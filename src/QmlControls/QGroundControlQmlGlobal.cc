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
#ifndef QGC_AIRLINK_DISABLED
#include "AirLinkManager.h"
#endif
#include "AppSettings.h"
#include "CustomAction.h"
#include "CustomActionManager.h"
#include "EditPositionDialogController.h"
#include "FirmwarePluginManager.h"
#include "FlightMapSettings.h"
#include "FlightPathSegment.h"
#ifndef NO_SERIAL_LINK
#include "GPSManager.h"
#include "GPSRtk.h"
#endif
#include "HorizontalFactValueGrid.h"
#include "InstrumentValueData.h"
#include "LinkManager.h"
#include "MAVLinkProtocol.h"
#ifdef QT_DEBUG
#include "MockLink.h"
#endif
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
#ifdef QGC_UTM_ADAPTER
#include "UTMSPManager.h"
#endif
#include "VideoManager.h"

#include <QtCore/QLineF>
#include <QtCore/QSettings>
#include <QtCore/QTimer>

QGC_LOGGING_CATEGORY(QGroundControlQmlGlobalLog, "qgc.qmlcontrols.qgroundcontrolqmlglobal");

static QObject *screenToolsControllerSingletonFactory(QQmlEngine*, QJSEngine*)
{
    ScreenToolsController *const screenToolsController = new ScreenToolsController();
    return screenToolsController;
}

static QObject *qgroundcontrolQmlGlobalSingletonFactory(QQmlEngine*, QJSEngine*)
{
    QGroundControlQmlGlobal *const qmlGlobal = new QGroundControlQmlGlobal();
    return qmlGlobal;
}

QGroundControlQmlGlobal::QGroundControlQmlGlobal(QObject *parent)
    : QObject(parent)
    , _adsbVehicleManager(ADSBVehicleManager::instance())
#ifndef QGC_AIRLINK_DISABLED
    , _airlinkManager(AirLinkManager::instance())
#endif
    , _corePlugin(QGCCorePlugin::instance())
    , _globalPalette(new QGCPalette(this))
#ifndef NO_SERIAL_LINK
    , _gpsRtkFactGroup(GPSManager::instance()->gpsRtk()->gpsRtkFactGroup())
#endif
    , _linkManager(LinkManager::instance())
    , _mapEngineManager(QGCMapEngineManager::instance())
    , _missionCommandTree(MissionCommandTree::instance())
    , _multiVehicleManager(MultiVehicleManager::instance())
    , _qgcPositionManager(QGCPositionManager::instance())
    , _settingsManager(SettingsManager::instance())
    , _unitsConversion(new QmlUnitsConversion(this))
#ifdef QGC_UTM_ADAPTER
    , _utmspManager(UTMSPManager::instance())
#endif
    , _videoManager(VideoManager::instance())
{
    // qCDebug(QGroundControlQmlGlobalLog) << Q_FUNC_INFO << this;

    QSettings settings;
    settings.beginGroup(_flightMapPositionSettingsGroup);
    _coord.setLatitude(settings.value(_flightMapPositionLatitudeSettingsKey, _coord.latitude()).toDouble());
    _coord.setLongitude(settings.value(_flightMapPositionLongitudeSettingsKey, _coord.longitude()).toDouble());
    _zoom = settings.value(_flightMapZoomSettingsKey, _zoom).toDouble();
    settings.endGroup();

    QTimer *const flightMapPositionSettledTimer = new QTimer(this);
    flightMapPositionSettledTimer->setSingleShot(true);
    flightMapPositionSettledTimer->setInterval(1000);

    (void) connect(flightMapPositionSettledTimer, &QTimer::timeout, [this, flightMapPositionSettledTimer]() {
        QSettings settings;
        settings.beginGroup(_flightMapPositionSettingsGroup);
        settings.setValue(_flightMapPositionLatitudeSettingsKey, _coord.latitude());
        settings.setValue(_flightMapPositionLongitudeSettingsKey, _coord.longitude());
        settings.setValue(_flightMapZoomSettingsKey, _zoom);
        settings.endGroup();
    });

    (void) connect(this, &QGroundControlQmlGlobal::flightMapPositionChanged, this, [flightMapPositionSettledTimer](QGeoCoordinate coord) {
        Q_UNUSED(coord);
        if (!flightMapPositionSettledTimer->isActive()) {
            flightMapPositionSettledTimer->start();
        }
    });

    (void) connect(this, &QGroundControlQmlGlobal::flightMapZoomChanged, this, [flightMapPositionSettledTimer](double zoom) {
        Q_UNUSED(zoom);
        if (!flightMapPositionSettledTimer->isActive()) {
            flightMapPositionSettledTimer->start();
        }
    });
}

QGroundControlQmlGlobal::~QGroundControlQmlGlobal()
{
    // qCDebug(QGroundControlQmlGlobalLog) << Q_FUNC_INFO << this;
}

void QGroundControlQmlGlobal::registerQmlTypes()
{
    (void) qmlRegisterUncreatableType<FactValueGrid>("QGroundControl.Templates", 1, 0, "FactValueGrid", "Reference only");
    (void) qmlRegisterUncreatableType<FlightPathSegment>("QGroundControl", 1, 0, "FlightPathSegment", "Reference only");
    (void) qmlRegisterUncreatableType<InstrumentValueData>("QGroundControl", 1, 0, "InstrumentValueData", "Reference only");
    (void) qmlRegisterUncreatableType<QGCGeoBoundingCube>("QGroundControl.FlightMap", 1, 0, "QGCGeoBoundingCube", "Reference only");
    (void) qmlRegisterUncreatableType<QGCMapPolygon>("QGroundControl.FlightMap", 1, 0, "QGCMapPolygon", "Reference only");
    (void) qmlRegisterUncreatableType<QmlObjectListModel>("QGroundControl", 1, 0, "QmlObjectListModel", "Reference only");

    (void) qmlRegisterType<CustomAction>("QGroundControl.Controllers", 1, 0, "CustomAction");
    (void) qmlRegisterType<CustomActionManager>("QGroundControl.Controllers", 1, 0, "CustomActionManager");
    (void) qmlRegisterType<EditPositionDialogController>("QGroundControl.Controllers", 1, 0, "EditPositionDialogController");
    (void) qmlRegisterType<HorizontalFactValueGrid>("QGroundControl.Templates", 1, 0, "HorizontalFactValueGrid");
    (void) qmlRegisterType<ParameterEditorController>("QGroundControl.Controllers", 1, 0, "ParameterEditorController");
    (void) qmlRegisterType<QGCFileDialogController>("QGroundControl.Controllers", 1, 0, "QGCFileDialogController");
    (void) qmlRegisterType<QGCMapCircle>("QGroundControl.FlightMap", 1, 0, "QGCMapCircle");
    (void) qmlRegisterType<QGCMapPalette>("QGroundControl.Palette", 1, 0, "QGCMapPalette");
    (void) qmlRegisterType<QGCPalette>("QGroundControl.Palette", 1, 0, "QGCPalette");
    (void) qmlRegisterType<RCChannelMonitorController>("QGroundControl.Controllers", 1, 0, "RCChannelMonitorController");
    (void) qmlRegisterType<RCToParamDialogController>("QGroundControl.Controllers", 1, 0, "RCToParamDialogController");
    (void) qmlRegisterType<ScreenToolsController>("QGroundControl.Controllers", 1, 0, "ScreenToolsController");
    (void) qmlRegisterType<TerrainProfile>("QGroundControl.Controls", 1, 0, "TerrainProfile");
    (void) qmlRegisterType<ToolStripAction>("QGroundControl.Controls", 1, 0, "ToolStripAction");
    (void) qmlRegisterType<ToolStripActionList>("QGroundControl.Controls", 1, 0, "ToolStripActionList");

    (void) qmlRegisterSingletonType<QGroundControlQmlGlobal>("QGroundControl", 1, 0, "QGroundControl", qgroundcontrolQmlGlobalSingletonFactory);
    (void) qmlRegisterSingletonType<ScreenToolsController>("QGroundControl.ScreenToolsController", 1, 0, "ScreenToolsController", screenToolsControllerSingletonFactory);
}

void QGroundControlQmlGlobal::saveGlobalSetting(const QString &key, const QString &value) const
{
    QSettings settings;
    settings.beginGroup(kQmlGlobalKeyName);
    settings.setValue(key, value);
    settings.endGroup();
}

QString QGroundControlQmlGlobal::loadGlobalSetting(const QString &key, const QString &defaultValue) const
{
    QSettings settings;
    settings.beginGroup(kQmlGlobalKeyName);
    return settings.value(key, defaultValue).toString();
}

void QGroundControlQmlGlobal::saveBoolGlobalSetting(const QString &key, bool value) const
{
    QSettings settings;
    settings.beginGroup(kQmlGlobalKeyName);
    settings.setValue(key, value);
    settings.endGroup();
}

bool QGroundControlQmlGlobal::loadBoolGlobalSetting(const QString &key, bool defaultValue) const
{
    QSettings settings;
    settings.beginGroup(kQmlGlobalKeyName);
    return settings.value(key, defaultValue).toBool();
}

void QGroundControlQmlGlobal::startPX4MockLink(bool sendStatusText) const
{
#ifdef QT_DEBUG
    MockLink::startPX4MockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::startGenericMockLink(bool sendStatusText) const
{
#ifdef QT_DEBUG
    MockLink::startGenericMockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::startAPMArduCopterMockLink(bool sendStatusText) const
{
#ifdef QT_DEBUG
    MockLink::startAPMArduCopterMockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::startAPMArduPlaneMockLink(bool sendStatusText) const
{
#ifdef QT_DEBUG
    MockLink::startAPMArduPlaneMockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::startAPMArduSubMockLink(bool sendStatusText) const
{
#ifdef QT_DEBUG
    MockLink::startAPMArduSubMockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::startAPMArduRoverMockLink(bool sendStatusText) const
{
#ifdef QT_DEBUG
    MockLink::startAPMArduRoverMockLink(sendStatusText);
#else
    Q_UNUSED(sendStatusText);
#endif
}

void QGroundControlQmlGlobal::stopOneMockLink() const
{
#ifdef QT_DEBUG
    const QList<SharedLinkInterfacePtr> sharedLinks = LinkManager::instance()->links();
    for (SharedLinkInterfacePtr sharedLink : sharedLinks) {
        LinkInterface *const link = sharedLink.get();
        MockLink *const mockLink = qobject_cast<MockLink*>(link);
        if (mockLink) {
            mockLink->disconnect();
            return;
        }
    }
#endif
}

bool QGroundControlQmlGlobal::singleFirmwareSupport() const
{
    return (FirmwarePluginManager::instance()->supportedFirmwareClasses().count() == 1);
}

bool QGroundControlQmlGlobal::singleVehicleSupport() const
{
    return (singleFirmwareSupport() ? (FirmwarePluginManager::instance()->supportedVehicleClasses(FirmwarePluginManager::instance()->supportedFirmwareClasses()[0]).count() == 1) : false);
}

bool QGroundControlQmlGlobal::px4ProFirmwareSupported() const
{
    return FirmwarePluginManager::instance()->supportedFirmwareClasses().contains(QGCMAVLink::FirmwareClassPX4);
}

bool QGroundControlQmlGlobal::apmFirmwareSupported() const
{
    return FirmwarePluginManager::instance()->supportedFirmwareClasses().contains(QGCMAVLink::FirmwareClassArduPilot);
}

QStringList QGroundControlQmlGlobal::loggingCategories() const
{
    return QGCLoggingCategoryRegister::instance()->registeredCategories();
}

void QGroundControlQmlGlobal::setCategoryLoggingOn(const QString &category, bool enable) const
{
    QGCLoggingCategoryRegister::instance()->setCategoryLoggingOn(category, enable);
}

bool QGroundControlQmlGlobal::categoryLoggingOn(const QString &category) const
{
    return QGCLoggingCategoryRegister::instance()->categoryLoggingOn(category);
}

void QGroundControlQmlGlobal::updateLoggingFilterRules() const
{
    QGCLoggingCategoryRegister::instance()->setFilterRulesFromSettings(QString());
}

bool QGroundControlQmlGlobal::linesIntersect(QPointF line1A, QPointF line1B, QPointF line2A, QPointF line2B) const
{
    QPointF intersectPoint;
    const QLineF::IntersectionType intersect = QLineF(line1A, line1B).intersects(QLineF(line2A, line2B), &intersectPoint);
    return ((intersect == QLineF::BoundedIntersection) && (intersectPoint != line1A) && (intersectPoint != line1B));
}

QString QGroundControlQmlGlobal::qgcVersion() const
{
    QString versionStr = qgcApp()->applicationVersion();
    if (QSysInfo::buildAbi().contains("32")) {
        versionStr += QStringLiteral(" %1").arg(tr("32 bit"));
    } else if(QSysInfo::buildAbi().contains("64")) {
        versionStr += QStringLiteral(" %1").arg(tr("64 bit"));
    }

    return versionStr;
}

QString QGroundControlQmlGlobal::altitudeModeExtraUnits(AltMode altMode) const
{
    switch (altMode) {
    case AltitudeModeNone:
        return QString();
    case AltitudeModeRelative:
        return QString();
    case AltitudeModeAbsolute:
        return tr("(AMSL)");
    case AltitudeModeCalcAboveTerrain:
        return tr("(CalcT)");
    case AltitudeModeTerrainFrame:
        return tr("(TerrF)");
    case AltitudeModeMixed:
    default:
        qCWarning(QGroundControlQmlGlobalLog) << "Internal Error:" << Q_FUNC_INFO << "called with altMode == AltitudeModeMixed";
        return QString();
    }
}

QString QGroundControlQmlGlobal::altitudeModeShortDescription(AltMode altMode) const
{
    switch (altMode) {
    case AltitudeModeNone:
    default:
        return QString();
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
    }
}

bool QGroundControlQmlGlobal::isVersionCheckEnabled() const
{
    return MAVLinkProtocol::instance()->versionCheckEnabled();
}

int QGroundControlQmlGlobal::mavlinkSystemID() const
{
    return MAVLinkProtocol::instance()->getSystemId();
}

QString QGroundControlQmlGlobal::elevationProviderName() const
{
    return _settingsManager->flightMapSettings()->elevationMapProvider()->rawValue().toString();
}

QString QGroundControlQmlGlobal::elevationProviderNotice() const
{
    return _settingsManager->flightMapSettings()->elevationMapProvider()->rawValue().toString();
}

QString QGroundControlQmlGlobal::parameterFileExtension() const
{
    return AppSettings::parameterFileExtension;
}

QString QGroundControlQmlGlobal::missionFileExtension() const
{
    return AppSettings::missionFileExtension;
}

QString QGroundControlQmlGlobal::telemetryFileExtension() const
{
    return AppSettings::telemetryFileExtension;
}

QString QGroundControlQmlGlobal::appName() const
{
    return qgcApp()->applicationName();
}

void QGroundControlQmlGlobal::deleteAllSettingsNextBoot() const
{
    qgcApp()->deleteAllSettingsNextBoot();
}

void QGroundControlQmlGlobal::clearDeleteAllSettingsNextBoot() const
{
    qgcApp()->clearDeleteAllSettingsNextBoot();
}

void QGroundControlQmlGlobal::setIsVersionCheckEnabled(bool enable)
{
    if (enable != MAVLinkProtocol::instance()->versionCheckEnabled()) {
        MAVLinkProtocol::instance()->enableVersionCheck(enable);
        emit isVersionCheckEnabledChanged(enable);
    }
}

void QGroundControlQmlGlobal::setMavlinkSystemID(int id)
{
    if (id != MAVLinkProtocol::instance()->getSystemId()) {
        MAVLinkProtocol::instance()->setSystemId(id);
        emit mavlinkSystemIDChanged(id);
    }
}

void QGroundControlQmlGlobal::setSkipSetupPage(bool skip)
{
    if (skip != _skipSetupPage) {
        _skipSetupPage = skip;
        emit skipSetupPageChanged();
    }
}

void QGroundControlQmlGlobal::setFlightMapPosition(const QGeoCoordinate &coordinate)
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
