#include "CustomPlugin.h"
#include "QGCApplication.h"
#include "AppSettings.h"
#include "BrandImageSettings.h"
#include "FactMetaData.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(CustomLog, "qgc.custom.customplugin")

CustomFlyViewOptions::CustomFlyViewOptions(CustomOptions *options, QObject *parent)
    : QGCFlyViewOptions(options, parent)
{

}

CustomOptions::CustomOptions(CustomPlugin*, QObject *parent)
    : QGCOptions(parent)
{

}

QGCFlyViewOptions *CustomOptions::flyViewOptions()
{
    if (!_flyViewOptions) {
        _flyViewOptions = new CustomFlyViewOptions(this, this);
    }

    return _flyViewOptions;
}

CustomPlugin::CustomPlugin(QGCApplication *app, QGCToolbox *toolbox)
    : QGCCorePlugin(app, toolbox)
    , _options(new CustomOptions(this, this))
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
}

QGCOptions *CustomPlugin::options()
{
    return _options;
}

bool CustomPlugin::overrideSettingsGroupVisibility(QString name)
{
    if (name == BrandImageSettings::name) {
        return false;
    }

    return true;
}

bool CustomPlugin::adjustSettingMetaData(const QString &settingsGroup, FactMetaData &metaData)
{
    const bool parentResult = QGCCorePlugin::adjustSettingMetaData(settingsGroup, metaData);

    if (settingsGroup == AppSettings::settingsGroup) {

    }

    return parentResult;
}

bool CustomPlugin::mavlinkMessage(Vehicle *vehicle, LinkInterface *link, mavlink_message_t message)
{
    _droneFactGroup->handleMessage(vehicle, message);
    _gpuFactGroup->handleMessage(vehicle, message);
    _nextVisionFactGroup->handleMessage(vehicle, message);
    _viewproFactGroup->handleMessage(vehicle, message);

    return true;
}

const QVariantList &CustomPlugin::toolBarIndicators()
{
    if (_toolBarIndicatorList.isEmpty()) {
        _toolBarIndicatorList = QVariantList({
            // QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/DroneIndicator.qml")),
            // QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/GPUIndicator.qml")),
        });
    }

    return _toolBarIndicatorList;
}
