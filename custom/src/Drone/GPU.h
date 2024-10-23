#pragma once

#include <QtCore/QLoggingCategory>

#include "GPUFactGroup.h"

class Vehicle;

Q_DECLARE_LOGGING_CATEGORY(GPULog)

class GPU : public QObject
{
    Q_OBJECT
    // QML_ELEMENT
    // QML_SINGLETON

public:
    explicit GPU(GPUFactGroup *gpuFactGroup, QObject *parent = nullptr);
    ~GPU();

    Q_INVOKABLE void setMotorDutyOffset(Vehicle *vehicle, int8_t value);
    Q_INVOKABLE void requestArmAuthorization(Vehicle *vehicle);

private:
    GPUFactGroup *_gpuFactGroup = nullptr;
};
