#include "CustomPlugin.h"
#include "CustomOptions.h"
#include "QGCApplication.h"
#include "FactMetaData.h"
#include "QmlComponentInfo.h"
#include "HorizontalFactValueGrid.h"
#include "InstrumentValueData.h"
#include "SettingsManager.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(CustomLog, "qgc.custom.customplugin")

CustomPlugin::CustomPlugin(QGCApplication *app, QGCToolbox *toolbox)
    : QGCCorePlugin(app, toolbox)
    , _options(new CustomOptions(this))
{
    _showAdvancedUI = false;
}

CustomPlugin::~CustomPlugin()
{

}

void CustomPlugin::setToolbox(QGCToolbox *toolbox)
{
    QGCCorePlugin::setToolbox(toolbox);
    _droneFactGroup = new DroneFactGroup(this);
    _gpuFactGroup = new GPUFactGroup(this);
    _nextVisionFactGroup = new NextVisionFactGroup(this);
    _viewproFactGroup = new ViewproFactGroup(this);

    _droneControl = new Drone(_droneFactGroup, this);
}

QVariantList &CustomPlugin::analyzePages()
{
    static QVariantList analyzeList = {
        QVariant::fromValue(new QmlComponentInfo(QStringLiteral("Log Download"), QUrl::fromUserInput("qrc:/qml/LogDownloadPage.qml"), QUrl::fromUserInput("qrc:/qmlimages/LogDownloadIcon"))),
#ifndef QGC_DISABLE_MAVLINK_INSPECTOR
        QVariant::fromValue(new QmlComponentInfo(QStringLiteral("MAVLink Inspector"), QUrl::fromUserInput("qrc:/qml/MAVLinkInspectorPage.qml"), QUrl::fromUserInput("qrc:/qmlimages/MAVLinkInspector"))),
#endif
        QVariant::fromValue(new QmlComponentInfo(QStringLiteral("Vibration"), QUrl::fromUserInput("qrc:/qml/VibrationPage.qml"), QUrl::fromUserInput("qrc:/qmlimages/VibrationPageIcon")))
    };

    return analyzeList;
}

QGCOptions *CustomPlugin::options()
{
    return _options;
}

bool CustomPlugin::overrideSettingsGroupVisibility(QString name)
{
    static const QStringList hideList = {
        ADSBVehicleManagerSettings::name,
        BrandImageSettings::name,
        CustomMavlinkActionsSettings::name,
        FirmwareUpgradeSettings::name,
        FlightModeSettings::name,
        PlanViewSettings::name,
        RemoteIDSettings::name,
        RTKSettings::name,
        Viewer3DSettings::name
    };

    return !hideList.contains(name);
}

bool CustomPlugin::adjustSettingMetaData(const QString &settingsGroup, FactMetaData &metaData)
{
    const bool parentResult = QGCCorePlugin::adjustSettingMetaData(settingsGroup, metaData);

    if (settingsGroup == APMMavlinkStreamRateSettings::settingsGroup) {
        if (metaData.name() == APMMavlinkStreamRateSettings::streamRateRawSensorsName) {
            metaData.setRawDefaultValue(-1);
            return true;
        } else if (metaData.name() == APMMavlinkStreamRateSettings::streamRateExtendedStatusName) {
            metaData.setRawDefaultValue(-1);
            return true;
        } else if (metaData.name() == APMMavlinkStreamRateSettings::streamRateRCChannelsName) {
            metaData.setRawDefaultValue(-1);
            return true;
        } else if (metaData.name() == APMMavlinkStreamRateSettings::streamRatePositionName) {
            metaData.setRawDefaultValue(-1);
            return true;
        } else if (metaData.name() == APMMavlinkStreamRateSettings::streamRateExtra1Name) {
            metaData.setRawDefaultValue(-1);
            return true;
        } else if (metaData.name() == APMMavlinkStreamRateSettings::streamRateExtra2Name) {
            metaData.setRawDefaultValue(-1);
            return true;
        } else if (metaData.name() == APMMavlinkStreamRateSettings::streamRateExtra3Name) {
            metaData.setRawDefaultValue(-1);
            return true;
        }
    } else if (settingsGroup == AppSettings::settingsGroup) {
        if (metaData.name() == AppSettings::apmStartMavlinkStreamsName) {
            metaData.setRawDefaultValue(false);
            return true;
        } else if (metaData.name() == AppSettings::followTargetName) {
            metaData.setRawDefaultValue(false);
            return true;
        } else if (metaData.name() == AppSettings::useChecklistName) {
            metaData.setRawDefaultValue(true);
            return true;
        }
    } else if (settingsGroup == AutoConnectSettings::settingsGroup) {
        if (metaData.name() == AutoConnectSettings::autoConnectUDPName) {
            metaData.setRawDefaultValue(true);
            return true;
        } else if (metaData.name() == AutoConnectSettings::autoConnectPixhawkName) {
            metaData.setRawDefaultValue(false);
            return true;
        } else if (metaData.name() == AutoConnectSettings::autoConnectSiKRadioName) {
            metaData.setRawDefaultValue(false);
            return false;
        } else if (metaData.name() == AutoConnectSettings::autoConnectPX4FlowName) {
            metaData.setRawDefaultValue(false);
            return false;
        } else if (metaData.name() == AutoConnectSettings::autoConnectRTKGPSName) {
            metaData.setRawDefaultValue(false);
            return false;
        } else if (metaData.name() == AutoConnectSettings::autoConnectLibrePilotName) {
            metaData.setRawDefaultValue(false);
            return false;
        } else if (metaData.name() == AutoConnectSettings::autoConnectZeroConfName) {
            metaData.setRawDefaultValue(false);
            return false;
        } else if (metaData.name() == AutoConnectSettings::udpListenPortName) {
            metaData.setRawDefaultValue(14570);
            return true;
        } else if (metaData.name() == AutoConnectSettings::udpTargetHostIPName) {
            metaData.setRawDefaultValue(QStringLiteral("192.168.1.5"));
            return true;
        } else if (metaData.name() == AutoConnectSettings::udpTargetHostPortName) {
            metaData.setRawDefaultValue(14570);
            return true;
        }
    } else if (settingsGroup == FirmwareUpgradeSettings::settingsGroup) {
        if (metaData.name() == FirmwareUpgradeSettings::apmVehicleTypeName) {
            metaData.setRawDefaultValue(0);
            return true;
        } else if (metaData.name() == FirmwareUpgradeSettings::apmChibiOSName) {
            metaData.setRawDefaultValue(0);
            return true;
        }
    } else if (settingsGroup == FlightModeSettings::settingsGroup) {
        if (metaData.name() == FlightModeSettings::apmHiddenFlightModesMultiRotorName) {
            metaData.setRawDefaultValue(
                QStringLiteral(
                    "Acro,Circle,Drift,Sport,Flip,Throw,Guided No GPS,"
                    "Flow Hold,ZigZag,Turtle,Autotune,SystemID,AutoRotate,"
                    "Stabilize,Altitude Hold,Auto,Position Hold,Avoid ADSB,"
                    "Smart RTL,Follow,AutoRTL,Loiter"
                )
            );
            return true;
        }
    } else if (settingsGroup == FlyViewSettings::settingsGroup) {
        if (metaData.name() == FlyViewSettings::guidedMinimumAltitudeName) {
            metaData.setRawDefaultValue(10);
            return true;
        } else if (metaData.name() == FlyViewSettings::guidedMaximumAltitudeName) {
            metaData.setRawDefaultValue(121.92);
            return true;
        } else if (metaData.name() == FlyViewSettings::showSimpleCameraControlName) {
            metaData.setRawDefaultValue(false);
            return false;
        } else if (metaData.name() == FlyViewSettings::maxGoToLocationDistanceName) {
            // TODO: Adjust based on Geofence
            metaData.setRawDefaultValue(20);
            return false;
        }
    } else if (settingsGroup == MapsSettings::settingsGroup) {
        if (metaData.name() == MapsSettings::maxCacheDiskSizeName) {
            metaData.setRawDefaultValue(2048);
            return true;
        } else if (metaData.name() == MapsSettings::maxCacheMemorySizeName) {
#ifdef __mobile__
            metaData.setRawDefaultValue(32);
#else
            metaData.setRawDefaultValue(256);
#endif
            return true;
        }
    } else if (settingsGroup == OfflineMapsSettings::settingsGroup) {
        if (metaData.name() == OfflineMapsSettings::minZoomLevelDownloadName) {
            metaData.setRawDefaultValue(13);
            return true;
        } else if (metaData.name() == OfflineMapsSettings::maxZoomLevelDownloadName) {
            metaData.setRawDefaultValue(19);
            return true;
        } else if (metaData.name() == OfflineMapsSettings::maxTilesForDownloadName) {
            metaData.setRawDefaultValue(100000);
            return true;
        }
    } else if (settingsGroup == VideoSettings::settingsGroup) {
        if (metaData.name() == VideoSettings::videoSourceName) {
            // TODO: Change based on viewpro/nextvision
            metaData.setRawDefaultValue(VideoSettings::videoDisabled);
            // metaData.setRawDefaultValue(VideoSettings::videoSourceRTSP);
            return true;
        } else if (metaData.name() == VideoSettings::rtspUrlName) {
            // TODO: Change based on viewpro/nextvision
            // metaData.setRawDefaultValue(QStringLiteral("rtsp://"));
            return true;
        } else if (metaData.name() == VideoSettings::gridLinesName) {
            metaData.setRawDefaultValue(1);
            return true;
        } else if (metaData.name() == VideoSettings::rtspTimeoutName) {
            metaData.setRawDefaultValue(8);
            return true;
        } else if (metaData.name() == VideoSettings::streamEnabledName) {
            // TODO: Change based on viewpro/nextvision
            metaData.setRawDefaultValue(false);
            return true;
        } else if (metaData.name() == VideoSettings::lowLatencyModeName) {
            metaData.setRawDefaultValue(false);
            return true;
        }
    } else if (settingsGroup == Viewer3DSettings::settingsGroup) {
        if (metaData.name() == Viewer3DSettings::enabledName) {
            metaData.setRawDefaultValue(false);
            return false;
        }
    }

    return parentResult;
}

QString CustomPlugin::brandImageIndoor() const
{
    return QStringLiteral("/custom/img/ZATBrandImage.png");
}

QString CustomPlugin::brandImageOutdoor() const
{
    return QStringLiteral("/custom/img/ZATBrandImage.png");
}

void CustomPlugin::factValueGridCreateDefaultSettings(const QString &defaultSettingsGroup)
{
    HorizontalFactValueGrid factValueGrid(defaultSettingsGroup);

    factValueGrid.setFontSize(FactValueGrid::LargeFontSize);

    factValueGrid.appendColumn();
    int columnIndex = 0;
    factValueGrid.appendRow();
    int rowIndex = 0;
    QmlObjectListModel *column = factValueGrid.columns()->value<QmlObjectListModel*>(columnIndex++);

    InstrumentValueData *value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Vehicle", "AltitudeRelative");
    value->setIcon("arrow-thick-up.svg");
    value->setText(value->fact()->shortDescription());
    value->setShowUnits(true);

    value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Vehicle", "DistanceToHome");
    value->setIcon("bookmark copy 3.svg");
    value->setText(value->fact()->shortDescription());
    value->setShowUnits(true);

    rowIndex = 0;
    factValueGrid.appendColumn();
    column = factValueGrid.columns()->value<QmlObjectListModel*>(columnIndex++);

    value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Vehicle", "ClimbRate");
    value->setIcon("arrow-simple-up.svg");
    value->setText(value->fact()->shortDescription());
    value->setShowUnits(true);

    value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Vehicle", "GroundSpeed");
    value->setIcon("arrow-simple-right.svg");
    value->setText(value->fact()->shortDescription());
    value->setShowUnits(true);

    rowIndex = 0;
    factValueGrid.appendColumn();
    column = factValueGrid.columns()->value<QmlObjectListModel*>(columnIndex++);

    value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Vehicle", "AirSpeed");
    value->setText("AirSpd");
    value->setShowUnits(true);

    value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Vehicle", "ThrottlePct");
    value->setText("Thr");
    value->setShowUnits(true);

    rowIndex = 0;
    factValueGrid.appendColumn();
    column = factValueGrid.columns()->value<QmlObjectListModel*>(columnIndex++);

    value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Vehicle", "FlightTime");
    value->setIcon("timer.svg");
    value->setText(value->fact()->shortDescription());
    value->setShowUnits(false);

    value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Vehicle", "FlightDistance");
    value->setIcon("travel-walk.svg");
    value->setText(value->fact()->shortDescription());
    value->setShowUnits(true);
}

bool CustomPlugin::mavlinkMessage(Vehicle *vehicle, LinkInterface *link, mavlink_message_t message)
{
    _droneFactGroup->handleMessage(vehicle, message);
    _gpuFactGroup->handleMessage(vehicle, message);
    _nextVisionFactGroup->handleMessage(vehicle, message);
    _viewproFactGroup->handleMessage(vehicle, message);

    return true;
}

QList<int> CustomPlugin::firstRunPromptStdIds()
{
    static const QList<int> rgStdIds = { unitsFirstRunPromptId };
    return rgStdIds;
}

const QVariantList &CustomPlugin::toolBarIndicators()
{
    static const QVariantList toolBarIndicatorList = {
        QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/DroneIndicator.qml")),
        QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/GPUIndicator.qml")),
    };

    return toolBarIndicatorList;
}
