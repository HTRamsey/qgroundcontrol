#include "CustomPlugin.h"
#include "CustomOptions.h"
#include "QGCApplication.h"
#include "FactMetaData.h"
#include "QmlComponentInfo.h"
#include "HorizontalFactValueGrid.h"
#include "InstrumentValueData.h"
#include "SettingsManager.h"
#include "MAVLinkProtocol.h"
#include "Vehicle.h"
#include "LinkInterface.h"
#include "MultiVehicleManager.h"
#include "JoystickManager.h"
#include "ParameterManager.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(CustomLog, "qgc.custom.customplugin")

CustomPlugin::CustomPlugin(QGCApplication *app, QGCToolbox *toolbox)
    : QGCCorePlugin(app, toolbox)
    , _options(new CustomOptions(this))
{
    // qCDebug(CustomLog) << Q_FUNC_INFO << this;

    _showAdvancedUI = false;
}

CustomPlugin::~CustomPlugin()
{
    // qCDebug(CustomLog) << Q_FUNC_INFO << this;
}

void CustomPlugin::setToolbox(QGCToolbox *toolbox)
{
    QGCCorePlugin::setToolbox(toolbox);

    _droneFactGroup = new DroneFactGroup(this);
    _gpuFactGroup = new GPUFactGroup(this);
    _nextVisionFactGroup = new NextVisionFactGroup(this);
    _viewproFactGroup = new ViewproFactGroup(this);
    _gcsFactGroup = new GCSFactGroup(this);

    _droneControl = new Drone(_droneFactGroup, this);
    _gpuControl = new GPU(_gpuFactGroup, this);
    _viewproControl = new Viewpro(_viewproFactGroup, this);

    MAVLinkProtocol *const mavlink = qgcApp()->toolbox()->mavlinkProtocol();
    mavlink->setSystemId(_mavsys.sysid);
    (void) connect(mavlink, &MAVLinkProtocol::messageReceived, this, [this](LinkInterface *link, mavlink_message_t message) {
        Q_UNUSED(link);
        if (_gpuFactGroup) {
            _gpuFactGroup->handleMessage(nullptr, message);
        }
    }, Qt::AutoConnection);

    MultiVehicleManager *const multiVehicleManager = qgcApp()->toolbox()->multiVehicleManager();
    (void) connect(multiVehicleManager, &MultiVehicleManager::vehicleAdded, this, [this](Vehicle *vehicle) {
        // TODO: Add these to drones fact groups
        (void) connect(_toolbox->multiVehicleManager(), &MultiVehicleManager::vehicleAdded, this, [this](Vehicle *vehicle) {
            vehicle->_addFactGroup(dynamic_cast<FactGroup*>(_droneFactGroup), QStringLiteral("Drone"));
            vehicle->_addFactGroup(_gpuFactGroup, QStringLiteral("GPU"));
            vehicle->_addFactGroup(_nextVisionFactGroup, QStringLiteral("NextVision"));
            vehicle->_addFactGroup(_viewproFactGroup, QStringLiteral("Viewpro"));
            vehicle->_addFactGroup(_gcsFactGroup, QStringLiteral("GCS"));
        });
        // _requestHomePosition(vehicle);
    }, Qt::AutoConnection);

    JoystickManager *const joystickManager = qgcApp()->toolbox()->joystickManager();
    (void) connect(joystickManager, &JoystickManager::activeJoystickChanged, this, [this](Joystick *joystick) {
        if (!joystick) {
            return;
        }
        // joystick->totalButtonCount();
        // joystick->buttonActions()
    }, Qt::AutoConnection);

    /*ParameterManager *const parameterManager = qgcApp()->toolbox()->multiVehicleManager()->activeVehicle()->parameterManager();
    (void) connect(parameterManager, &ParameterManager::parametersReadyChanged, this, [parameterManager](bool parametersReady) {
        if (!parametersReady) {
            return;
        }
        if (parameterManager->parameterExists(ParameterManager::defaultComponentId, "FENCE_ALT_MAX")) {

        }
        if (parameterManager->parameterExists(ParameterManager::defaultComponentId, "FENCE_RADIUS")) {

        }
        if (parameterManager->parameterExists(ParameterManager::defaultComponentId, "AHRS_GPS_MINSATS")) {

        }
        if (parameterManager->parameterExists(ParameterManager::defaultComponentId, "BATT_LOW_VOLT")) {

        }
        if (parameterManager->parameterExists(ParameterManager::defaultComponentId, "BARO_ALT_OFFSET")) {

        }
        if (parameterManager->parameterExists(ParameterManager::defaultComponentId, "ATC_RATE_Y_MAX")) {

        }
    }, Qt::AutoConnection);*/
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
        } else if (metaData.name() == AppSettings::indoorPaletteName) {
            metaData.setRawDefaultValue(1);
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
            // TODO: Adjust based on Geofence - FENCE_RADIUS
            metaData.setRawDefaultValue(40);
            return false;
        }
    } else if (settingsGroup == MapsSettings::settingsGroup) {
        if (metaData.name() == MapsSettings::maxCacheDiskSizeName) {
            metaData.setRawDefaultValue(4096);
            return true;
        } else if (metaData.name() == MapsSettings::maxCacheMemorySizeName) {
#ifdef Q_OS_ANDROID
            metaData.setRawDefaultValue(256);
#else
            metaData.setRawDefaultValue(512);
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
            metaData.setRawDefaultValue(0);
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
    } else if (settingsGroup == CustomMavlinkActionsSettings::settingsGroup) {
        metaData.setRawDefaultValue(false);
        return false;
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
    value->setFact("Vehicle", "ThrottlePct");
    value->setText("Thr");
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
    value->setFact("Vehicle", "FlightTime");
    value->setIcon("timer.svg");
    value->setText(value->fact()->shortDescription());
    value->setShowUnits(false);

    /*value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Gpu", "tetherTension");
    value->setText("Tension");*/

    rowIndex = 0;
    factValueGrid.appendColumn();
    column = factValueGrid.columns()->value<QmlObjectListModel*>(columnIndex++);

    /*value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Battery0", "instantPower");
    value->setText(value->fact()->shortDescription());
    value->setShowUnits(true);

    value = column->value<InstrumentValueData*>(rowIndex++);
    value->setFact("Battery0", "voltage");
    value->setText(value->fact()->shortDescription());
    value->setShowUnits(true);*/
}

bool CustomPlugin::mavlinkMessage(Vehicle *vehicle, LinkInterface *link, mavlink_message_t message)
{
    if (_droneFactGroup) {
        _droneFactGroup->handleMessage(vehicle, message);
    }
    if (_gpuFactGroup) {
        _gpuFactGroup->handleMessage(vehicle, message);
    }
    if (_nextVisionFactGroup) {
        _nextVisionFactGroup->handleMessage(vehicle, message);
    }
    if (_viewproFactGroup) {
        _viewproFactGroup->handleMessage(vehicle, message);
    }
    if (_gcsFactGroup) {
        _gcsFactGroup->handleMessage(vehicle, message);
    }

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

/*void CustomPlugin::_activeJoystickChanged(Joystick *activeJoystick)
{
    if (activeJoystick) {
        (void) connect(activeCamJoystick, &Joystick::manualControlCam, this, &CameraManagement::manualCamControl);
        this->_activeJoystick = activeJoystick;

        for ( int i = 0; i < 32;i++ ) {
            _buttonFuncState[i] = JoyBtnReleased;
            _buttonFuncValue[i] = 0;
        }
    } else {
        if (_activeJoystick) {
            (void) disconnect(this->_activeCamJoystick, &Joystick::manualControlCam, this, &CameraManagement::manualCamControl);
        }
        _activeCamJoystick = activeCamJoystick;
    }
}*/

/*void CustomPlugin::_requestHomePositionCommandResultHandler(void *resultHandlerData, int compId, const mavlink_command_ack_t &ack, MavCmdResultFailureCode_t failureCode)
{
    static int attempts = 0;
    if ((ack.result != MAV_RESULT_ACCEPTED) || (failureCode != MavCmdResultCommandResultOnly)) {
        Vehicle *const vehicle = static_cast<Vehicle*>(resultHandlerData);
        if ((attempts++ < 3) && vehicle) {
            _requestHomePosition(vehicle)
        }
    }
}

void CustomPlugin::_requestHomePosition(Vehicle *vehicle)
{
    const MavCmdAckHandlerInfo_t handlerInfo = {
        &CustomPlugin::_requestHomePositionCommandResultHandler,
        vehicle,
        nullptr,
        nullptr
    };

    vehicle->sendMavCommandWithHandler(
        &handlerInfo,
        compId,
        MAV_CMD_REQUEST_MESSAGE,
        msgId,
        MAVLINK_MSG_ID_HOME_POSITION
    );
}*/
