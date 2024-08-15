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
{
    _options = new CustomOptions(this, this);
    _showAdvancedUI = false;
}

CustomPlugin::~CustomPlugin()
{

}

void CustomPlugin::setToolbox(QGCToolbox* toolbox)
{
    QGCCorePlugin::setToolbox(toolbox);
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
    bool parentResult = QGCCorePlugin::adjustSettingMetaData(settingsGroup, metaData);

    if (settingsGroup == AppSettings::settingsGroup) {

    }

    return parentResult;
}
