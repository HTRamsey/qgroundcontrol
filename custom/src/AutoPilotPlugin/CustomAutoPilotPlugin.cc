#include "CustomAutoPilotPlugin.h"
#include "Vehicle.h"
#include "QGCApplication.h"
#include "QGCCorePlugin.h"
#include "ParameterManager.h"
#include "VehicleComponent.h"
#include "APMAirframeComponent.h"
#include "APMFlightModesComponent.h"
#include "APMRadioComponent.h"
#include "APMSafetyComponent.h"
#include "APMTuningComponent.h"
#include "APMSensorsComponent.h"
#include "APMPowerComponent.h"
#include "APMMotorComponent.h"
#include "APMCameraComponent.h"
#include "APMLightsComponent.h"
#include "APMSubFrameComponent.h"
#include "APMFollowComponent.h"

CustomAutoPilotPlugin::CustomAutoPilotPlugin(Vehicle *vehicle, QObject *parent)
    : APMAutoPilotPlugin(vehicle, parent)
{
    // connect(qgcApp()->toolbox()->corePlugin(), &QGCCorePlugin::showAdvancedUIChanged, this, &CustomAutoPilotPlugin::_advancedChanged);
}

CustomAutoPilotPlugin::~CustomAutoPilotPlugin()
{

}

const QVariantList& CustomAutoPilotPlugin::vehicleComponents()
{
    if (_components.isEmpty() && !_incorrectParameterVersion) {
        if (_vehicle->parameterManager()->parametersReady()) {
            _sensorsComponent = new APMSensorsComponent(_vehicle, this);
            _sensorsComponent->setupTriggerSignals();
            _components.append(QVariant::fromValue((VehicleComponent*)_sensorsComponent));

            if (!_vehicle->sub() || (_vehicle->sub() && _vehicle->versionCompare(3, 5, 3) >= 0)) {
                _motorComponent = new APMMotorComponent(_vehicle, this);
                _motorComponent->setupTriggerSignals();
                _components.append(QVariant::fromValue((VehicleComponent*)_motorComponent));
            }

            _safetyComponent = new APMSafetyComponent(_vehicle, this);
            _safetyComponent->setupTriggerSignals();
            _components.append(QVariant::fromValue((VehicleComponent*)_safetyComponent));

            if (_vehicle->parameterManager()->parameterExists(-1, QStringLiteral("FOLL_ENABLE"))) {
                _followComponent = new APMFollowComponent(_vehicle, this);
                _followComponent->setupTriggerSignals();
                _components.append(QVariant::fromValue((VehicleComponent*)_followComponent));
            }

            if(_vehicle->parameterManager()->parameterExists(-1, "MNT1_TYPE")) {
                _cameraComponent = new APMCameraComponent(_vehicle, this);
                _cameraComponent->setupTriggerSignals();
                _components.append(QVariant::fromValue((VehicleComponent*)_cameraComponent));
            }

        } else {
            qWarning() << "Call to vehicleCompenents prior to parametersReady";
        }
    }

    return _components;
}

/*QString CustomAutoPilotPlugin::prerequisiteSetup(VehicleComponent *component) const
{
    bool requiresAirframeCheck = false;

    if (qobject_cast<const APMFlightModesComponent*>(component)) {
        if (_airframeComponent && !_airframeComponent->setupComplete()) {
            return _airframeComponent->name();
        }
        if (_radioComponent && !_radioComponent->setupComplete()) {
            return _radioComponent->name();
        }
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMRadioComponent*>(component)) {
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMCameraComponent*>(component)) {
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMPowerComponent*>(component)) {
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMSafetyComponent*>(component)) {
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMTuningComponent*>(component)) {
        requiresAirframeCheck = true;
    } else if (qobject_cast<const APMSensorsComponent*>(component)) {
        requiresAirframeCheck = true;
    }

    if (requiresAirframeCheck) {
        if (_airframeComponent && !_airframeComponent->setupComplete()) {
            return _airframeComponent->name();
        }
    }

    return QString();
}*/
