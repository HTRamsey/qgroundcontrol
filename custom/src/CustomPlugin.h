#pragma once

#include <QtCore/QTranslator>
#include <QtCore/QLoggingCategory>

#include "QGCCorePlugin.h"
#include "QGCOptions.h"
#include "DroneFactGroup.h"
#include "GPUFactGroup.h"
#include "NextVisionFactGroup.h"
#include "ViewproFactGroup.h"

class CustomOptions;
class CustomPlugin;

Q_DECLARE_LOGGING_CATEGORY(CustomLog)

class CustomFlyViewOptions : public QGCFlyViewOptions
{
public:
    explicit CustomFlyViewOptions(CustomOptions *options, QObject *parent = nullptr);

    bool showMultiVehicleList() const final { return false; }
};

class CustomOptions : public QGCOptions
{
public:
    explicit CustomOptions(CustomPlugin *plugin, QObject *parent = nullptr);

    bool showFirmwareUpgrade() const final { return false; };
    QGCFlyViewOptions *flyViewOptions() final;

private:
    CustomFlyViewOptions *_flyViewOptions = nullptr;
};

class CustomPlugin : public QGCCorePlugin
{
    Q_OBJECT

    Q_PROPERTY(FactGroup* drone READ droneFactGroup CONSTANT)
    Q_PROPERTY(FactGroup* gpu READ gpuFactGroup CONSTANT)
    Q_PROPERTY(FactGroup* nextVision READ nextVisionFactGroup CONSTANT)
    Q_PROPERTY(FactGroup* viewpro READ viewproFactGroup CONSTANT)

public:
    CustomPlugin(QGCApplication *app, QGCToolbox *toolbox);
    ~CustomPlugin();

    QGCOptions *options() final;
    bool overrideSettingsGroupVisibility(QString name) final;
    bool adjustSettingMetaData(const QString &settingsGroup, FactMetaData &metaData) final;
    bool mavlinkMessage(Vehicle *vehicle, LinkInterface *link, mavlink_message_t message) final;
    const QVariantList &toolBarIndicators();

    void setToolbox(QGCToolbox *toolbox) final;

    FactGroup *droneFactGroup() { return _droneFactGroup; }
    FactGroup *gpuFactGroup() { return _gpuFactGroup; }
    FactGroup *nextVisionFactGroup() { return _nextVisionFactGroup; }
    FactGroup *viewproFactGroup() { return _viewproFactGroup; }

private:
    CustomOptions *_options = nullptr;
    DroneFactGroup *_droneFactGroup = nullptr;
    GPUFactGroup *_gpuFactGroup = nullptr;
    NextVisionFactGroup *_nextVisionFactGroup = nullptr;
    ViewproFactGroup *_viewproFactGroup = nullptr;

    const QString _droneFactGroupName = QStringLiteral("drone");
    const QString _gpuFactGroupName = QStringLiteral("gpu");
    const QString _nextVisionFactGroupName = QStringLiteral("nextvision");
    const QString _viewproFactGroupName = QStringLiteral("viewpro");
};
