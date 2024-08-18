#include "CustomPlugin.h"
#include "CustomOptions.h"
#include "QGCApplication.h"
#include "AppSettings.h"
#include "BrandImageSettings.h"
#include "FactMetaData.h"
#include "QmlComponentInfo.h"
#include "HorizontalFactValueGrid.h"
#include "InstrumentValueData.h"
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

    factValueGrid.appendRow();
    int rowIndex = 0;
    int columnIndex = 0;
    factValueGrid.appendColumn();
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
    if (_toolBarIndicatorList.isEmpty()) {
        _toolBarIndicatorList = QVariantList({
            // QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/DroneIndicator.qml")),
            // QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/GPUIndicator.qml")),
        });
    }

    return _toolBarIndicatorList;
}
