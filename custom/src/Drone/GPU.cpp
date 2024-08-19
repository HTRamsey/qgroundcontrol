#include "GPU.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QLoggingCategory>

QGC_LOGGING_CATEGORY(GPULog, "qgc.custom.drone.gpu")

GPU::GPU(QObject* parent): QObject(parent)
{
    // qCDebug(DroneLog) << Q_FUNC_INFO << this;
}

GPU::~GPU()
{
    // qCDebug(DroneLog) << Q_FUNC_INFO << this;
}

void GPU::setMotorDutyOffset(Vehicle *vehicle, int8_t value)
{
    /* new SetParamTask(
        static_cast<uint8_t>(GPUData::sysID()),
        static_cast<MAV_COMPONENT>(GPUData::compID()),
        static_cast<const QString>(GPUData::getParamNameTensionOffset()),
        static_cast<MAV_PARAM_TYPE>(GPUData::getParamTypeTensionOffset()),
        static_cast<QVariant>(QVariant(value))
    );*/
}

void GPU::requestArmAuthorization(Vehicle *vehicle)
{
    /* vehicle->sendMavCommand(vehicle->defaultComponentId(), MAV_CMD_ARM_AUTHORIZATION_REQUEST, true, ch, pwm);
    emit sendTask(
        new CommandLongTask(
            static_cast<uint8_t>(GPUData::sysID()),
            static_cast<MAV_COMPONENT>(GPUData::compID()),
            static_cast<MAV_CMD>(MAV_CMD_ARM_AUTHORIZATION_REQUEST),
            static_cast<uint8_t>(DroneData::sysID())
        )
    );*/
}
