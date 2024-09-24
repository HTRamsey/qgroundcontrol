#pragma once

#include <QtCore/QLoggingCategory>

#include "QGCCorePlugin.h"
#include "DroneFactGroup.h"
#include "GPUFactGroup.h"
#include "NextVisionFactGroup.h"
#include "ViewproFactGroup.h"
#include "GCSFactGroup.h"
#include "Drone.h"

class CustomOptions;
class QGCOptions;
class Vehicle;

Q_DECLARE_LOGGING_CATEGORY(CustomLog)

class CustomPlugin : public QGCCorePlugin
{
    Q_OBJECT

    Q_PROPERTY(FactGroup* drone READ droneFactGroup CONSTANT)
    Q_PROPERTY(FactGroup* gpu READ gpuFactGroup CONSTANT)
    Q_PROPERTY(FactGroup* nextVision READ nextVisionFactGroup CONSTANT)
    Q_PROPERTY(FactGroup* viewpro READ viewproFactGroup CONSTANT)
    Q_PROPERTY(FactGroup* gcs READ gcsFactGroup CONSTANT)
    Q_PROPERTY(Drone* droneControl READ droneControl CONSTANT)

public:
    CustomPlugin(QGCApplication *app, QGCToolbox *toolbox);
    ~CustomPlugin();

    QVariantList &analyzePages() final;
    QGCOptions *options() final;
    bool overrideSettingsGroupVisibility(QString name) final;
    bool adjustSettingMetaData(const QString &settingsGroup, FactMetaData &metaData) final;
    QString brandImageIndoor() const final;
    QString brandImageOutdoor() const final;
    void factValueGridCreateDefaultSettings(const QString &defaultSettingsGroup) final;
    bool mavlinkMessage(Vehicle *vehicle, LinkInterface *link, mavlink_message_t message) final;
    QList<int> firstRunPromptStdIds() final;
    const QVariantList &toolBarIndicators();

    void setToolbox(QGCToolbox *toolbox) final;

    FactGroup *droneFactGroup() { return _droneFactGroup; }
    FactGroup *gpuFactGroup() { return _gpuFactGroup; }
    FactGroup *nextVisionFactGroup() { return _nextVisionFactGroup; }
    FactGroup *viewproFactGroup() { return _viewproFactGroup; }
    FactGroup *gcsFactGroup() { return _gcsFactGroup; }

    Drone *droneControl() { return _droneControl; }

private:
    // void _requestHomePosition(Vehicle *vehicle);
    // void _requestHomePositionCommandResultHandler(void *resultHandlerData, int compId, const mavlink_command_ack_t &ack, MavCmdResultFailureCode_t failureCode);

    CustomOptions *_options = nullptr;

    DroneFactGroup *_droneFactGroup = nullptr;
    GPUFactGroup *_gpuFactGroup = nullptr;
    NextVisionFactGroup *_nextVisionFactGroup = nullptr;
    ViewproFactGroup *_viewproFactGroup = nullptr;
    GCSFactGroup *_gcsFactGroup = nullptr;

    Drone *_droneControl = nullptr;

    const QString _droneFactGroupName = QStringLiteral("drone");
    const QString _gpuFactGroupName = QStringLiteral("gpu");
    const QString _nextVisionFactGroupName = QStringLiteral("nextvision");
    const QString _gcsFactGroupName = QStringLiteral("gcs");

    static constexpr mavlink_system_t _mavsys = {3, MAV_COMP_ID_MISSIONPLANNER};
};
