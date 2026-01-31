#include "InitialConnectStateMachine.h"
#include "Vehicle.h"
#include "QGCCorePlugin.h"
#include "QGCOptions.h"
#include "FirmwarePlugin.h"
#include "ParameterManager.h"
#include "ComponentInformationManager.h"
#include "MissionManager.h"
#include "StandardModes.h"
#include "GeoFenceManager.h"
#include "RallyPointManager.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(InitialConnectStateMachineLog, "Vehicle.InitialConnectStateMachine")

// ============================================================================
// ProgressTracker Implementation
// ============================================================================

void InitialConnectStateMachine::ProgressTracker::initialize(const QList<int>& weights)
{
    _weights = weights;
    _totalWeight = 0;
    for (int w : weights) {
        _totalWeight += w;
    }
    _currentIndex = 0;
    _subProgress = 0.0f;
}

void InitialConnectStateMachine::ProgressTracker::setCurrentState(int index)
{
    _currentIndex = index;
    _subProgress = 0.0f;
}

void InitialConnectStateMachine::ProgressTracker::setSubProgress(float subProgress)
{
    _subProgress = qBound(0.0f, subProgress, 1.0f);
}

float InitialConnectStateMachine::ProgressTracker::progress() const
{
    if (_totalWeight <= 0) {
        return 0.0f;
    }

    int completedWeight = 0;
    for (int i = 0; i < _currentIndex && i < _weights.size(); ++i) {
        completedWeight += _weights[i];
    }

    int currentWeight = (_currentIndex >= 0 && _currentIndex < _weights.size())
                            ? _weights[_currentIndex]
                            : 1;

    float calculatedProgress = (completedWeight + currentWeight * _subProgress) / static_cast<float>(_totalWeight);

    // Progress should only increase, never decrease (handles race conditions in signal delivery)
    if (calculatedProgress > _lastEmittedProgress) {
        _lastEmittedProgress = calculatedProgress;
    }
    return _lastEmittedProgress;
}

void InitialConnectStateMachine::ProgressTracker::reset()
{
    _currentIndex = 0;
    _subProgress = 0.0f;
    _lastEmittedProgress = 0.0f;
}

// ============================================================================
// InitialConnectStateMachine Implementation
// ============================================================================

InitialConnectStateMachine::InitialConnectStateMachine(Vehicle* vehicle, QObject* parent)
    : QGCStateMachine(QStringLiteral("InitialConnectStateMachine"), vehicle, parent)
    , _vehicle(vehicle)
{
    // Initialize progress tracking with weights
    QList<int> weights;
    for (int i = 0; i < _stateCount; ++i) {
        weights.append(_progressWeights[i]);
    }
    _progressTracker.initialize(weights);

    _createStates();
    _wireTransitions();
    _wireProgressTracking();

    setInitialState(_stateAutopilotVersion);
}

InitialConnectStateMachine::~InitialConnectStateMachine()
{
}

void InitialConnectStateMachine::start()
{
    _progressTracker.reset();
    QGCStateMachine::start();
}

void InitialConnectStateMachine::_createStates()
{
    // State 0: Request autopilot version
    _stateAutopilotVersion = new AsyncFunctionState(
        QStringLiteral("RequestAutopilotVersion"),
        this,
        [this](AsyncFunctionState* state) { _requestAutopilotVersion(state); }
    );

    // State 1: Request standard modes
    _stateStandardModes = new AsyncFunctionState(
        QStringLiteral("RequestStandardModes"),
        this,
        [this](AsyncFunctionState* state) { _requestStandardModes(state); }
    );

    // State 2: Request component information
    _stateCompInfo = new AsyncFunctionState(
        QStringLiteral("RequestCompInfo"),
        this,
        [this](AsyncFunctionState* state) { _requestCompInfo(state); }
    );

    // State 3: Request parameters
    _stateParameters = new AsyncFunctionState(
        QStringLiteral("RequestParameters"),
        this,
        [this](AsyncFunctionState* state) { _requestParameters(state); }
    );

    // State 4: Request mission (skippable)
    _stateMission = new SkippableAsyncState(
        QStringLiteral("RequestMission"),
        this,
        [this]() { return _shouldSkipForLinkType() || !_hasPrimaryLink(); },
        [this](SkippableAsyncState* state) { _requestMission(state); },
        [this]() {
            qCDebug(InitialConnectStateMachineLog) << "Skipping mission load";
        }
    );

    // State 5: Request geofence (skippable)
    _stateGeoFence = new SkippableAsyncState(
        QStringLiteral("RequestGeoFence"),
        this,
        [this]() {
            return _shouldSkipForLinkType() || !_hasPrimaryLink() ||
                   !_vehicle->_geoFenceManager->supported();
        },
        [this](SkippableAsyncState* state) { _requestGeoFence(state); },
        [this]() {
            qCDebug(InitialConnectStateMachineLog) << "Skipping geofence load";
        }
    );

    // State 6: Request rally points (skippable)
    _stateRallyPoints = new SkippableAsyncState(
        QStringLiteral("RequestRallyPoints"),
        this,
        [this]() {
            return _shouldSkipForLinkType() || !_hasPrimaryLink() ||
                   !_vehicle->_rallyPointManager->supported();
        },
        [this](SkippableAsyncState* state) { _requestRallyPoints(state); },
        [this]() {
            qCDebug(InitialConnectStateMachineLog) << "Skipping rally points load";
            // Mark plan request complete when skipping
            _vehicle->_initialPlanRequestComplete = true;
            emit _vehicle->initialPlanRequestCompleteChanged(true);
        }
    );

    // State 7: Signal completion
    _stateComplete = new FunctionState(
        QStringLiteral("SignalComplete"),
        this,
        [this]() { _signalComplete(); }
    );

    // Final state
    _stateFinal = new QGCFinalState(QStringLiteral("Final"), this);
}

void InitialConnectStateMachine::_wireTransitions()
{
    // Linear progression for AsyncFunctionStates
    _stateAutopilotVersion->addTransition(_stateAutopilotVersion, &QGCState::advance, _stateStandardModes);
    _stateStandardModes->addTransition(_stateStandardModes, &QGCState::advance, _stateCompInfo);
    _stateCompInfo->addTransition(_stateCompInfo, &QGCState::advance, _stateParameters);
    _stateParameters->addTransition(_stateParameters, &QGCState::advance, _stateMission);

    // SkippableAsyncStates: both advance and skipped go to next state
    _stateMission->addTransition(_stateMission, &QGCState::advance, _stateGeoFence);
    _stateMission->addTransition(_stateMission, &SkippableAsyncState::skipped, _stateGeoFence);

    _stateGeoFence->addTransition(_stateGeoFence, &QGCState::advance, _stateRallyPoints);
    _stateGeoFence->addTransition(_stateGeoFence, &SkippableAsyncState::skipped, _stateRallyPoints);

    _stateRallyPoints->addTransition(_stateRallyPoints, &QGCState::advance, _stateComplete);
    _stateRallyPoints->addTransition(_stateRallyPoints, &SkippableAsyncState::skipped, _stateComplete);

    // Complete -> Final
    _stateComplete->addTransition(_stateComplete, &QGCState::advance, _stateFinal);
}

void InitialConnectStateMachine::_wireProgressTracking()
{
    // Track state index for progress calculation
    _stateAutopilotVersion->setOnEntry([this]() {
        _progressTracker.setCurrentState(0);
        _updateProgress();
    });

    _stateStandardModes->setOnEntry([this]() {
        _progressTracker.setCurrentState(1);
        _updateProgress();
    });

    _stateCompInfo->setOnEntry([this]() {
        _progressTracker.setCurrentState(2);
        _updateProgress();
    });

    _stateParameters->setOnEntry([this]() {
        _progressTracker.setCurrentState(3);
        _updateProgress();
    });

    _stateMission->setOnEntry([this]() {
        _progressTracker.setCurrentState(4);
        _updateProgress();
    });

    _stateGeoFence->setOnEntry([this]() {
        _progressTracker.setCurrentState(5);
        _updateProgress();
    });

    _stateRallyPoints->setOnEntry([this]() {
        _progressTracker.setCurrentState(6);
        _updateProgress();
    });

    _stateComplete->setOnEntry([this]() {
        _progressTracker.setCurrentState(7);
        _updateProgress();
    });
}

void InitialConnectStateMachine::_updateProgress(float subProgress)
{
    _progressTracker.setSubProgress(subProgress);
    emit progressUpdate(_progressTracker.progress());
}

void InitialConnectStateMachine::_onSubProgressUpdate(double progressValue)
{
    _updateProgress(static_cast<float>(progressValue));
}

// ============================================================================
// Skip Predicates
// ============================================================================

bool InitialConnectStateMachine::_shouldSkipForLinkType() const
{
    SharedLinkInterfacePtr sharedLink = _vehicle->vehicleLinkManager()->primaryLink().lock();
    if (!sharedLink) {
        return true;
    }
    return sharedLink->linkConfiguration()->isHighLatency() || sharedLink->isLogReplay();
}

bool InitialConnectStateMachine::_hasPrimaryLink() const
{
    SharedLinkInterfacePtr sharedLink = _vehicle->vehicleLinkManager()->primaryLink().lock();
    return sharedLink != nullptr;
}

// ============================================================================
// State Callbacks
// ============================================================================

void InitialConnectStateMachine::_requestAutopilotVersion(AsyncFunctionState* state)
{
    SharedLinkInterfacePtr sharedLink = _vehicle->vehicleLinkManager()->primaryLink().lock();

    if (!sharedLink) {
        qCDebug(InitialConnectStateMachineLog) << "Skipping REQUEST_MESSAGE:AUTOPILOT_VERSION request due to no primary link";
        state->complete();
        return;
    }

    if (sharedLink->linkConfiguration()->isHighLatency() || sharedLink->isLogReplay()) {
        qCDebug(InitialConnectStateMachineLog) << "Skipping REQUEST_MESSAGE:AUTOPILOT_VERSION request due to link type";
        state->complete();
        return;
    }

    qCDebug(InitialConnectStateMachineLog) << "Sending REQUEST_MESSAGE:AUTOPILOT_VERSION";

    // Store state pointer for callback to use
    _vehicle->requestMessage(
        [](void* resultHandlerData, MAV_RESULT commandResult,
           Vehicle::RequestMessageResultHandlerFailureCode_t failureCode,
           const mavlink_message_t& message) {
            auto* self = static_cast<InitialConnectStateMachine*>(resultHandlerData);
            self->_handleAutopilotVersionResult(commandResult, failureCode, message);
        },
        this,
        MAV_COMP_ID_AUTOPILOT1,
        MAVLINK_MSG_ID_AUTOPILOT_VERSION
    );
}

void InitialConnectStateMachine::_handleAutopilotVersionResult(
    MAV_RESULT commandResult,
    Vehicle::RequestMessageResultHandlerFailureCode_t failureCode,
    const mavlink_message_t& message)
{
    switch (failureCode) {
    case Vehicle::RequestMessageNoFailure:
    {
        qCDebug(InitialConnectStateMachineLog) << "AUTOPILOT_VERSION received";

        mavlink_autopilot_version_t autopilotVersion;
        mavlink_msg_autopilot_version_decode(&message, &autopilotVersion);

        _vehicle->_uid = (quint64)autopilotVersion.uid;
        _vehicle->_firmwareBoardVendorId = autopilotVersion.vendor_id;
        _vehicle->_firmwareBoardProductId = autopilotVersion.product_id;
        emit _vehicle->vehicleUIDChanged();

        if (autopilotVersion.flight_sw_version != 0) {
            int majorVersion, minorVersion, patchVersion;
            FIRMWARE_VERSION_TYPE versionType;

            majorVersion = (autopilotVersion.flight_sw_version >> (8*3)) & 0xFF;
            minorVersion = (autopilotVersion.flight_sw_version >> (8*2)) & 0xFF;
            patchVersion = (autopilotVersion.flight_sw_version >> (8*1)) & 0xFF;
            versionType = (FIRMWARE_VERSION_TYPE)((autopilotVersion.flight_sw_version >> (8*0)) & 0xFF);
            _vehicle->setFirmwareVersion(majorVersion, minorVersion, patchVersion, versionType);
        }

        if (_vehicle->px4Firmware()) {
            // Lower 3 bytes is custom version
            int majorVersion, minorVersion, patchVersion;
            majorVersion = autopilotVersion.flight_custom_version[2];
            minorVersion = autopilotVersion.flight_custom_version[1];
            patchVersion = autopilotVersion.flight_custom_version[0];
            _vehicle->setFirmwareCustomVersion(majorVersion, minorVersion, patchVersion);

            // PX4 Firmware stores the first 16 characters of the git hash as binary, with the individual bytes in reverse order
            _vehicle->_gitHash = "";
            for (int i = 7; i >= 0; i--) {
                _vehicle->_gitHash.append(QString("%1").arg(autopilotVersion.flight_custom_version[i], 2, 16, QChar('0')));
            }
        } else {
            // APM Firmware stores the first 8 characters of the git hash as an ASCII character string
            char nullStr[9];
            strncpy(nullStr, (char*)autopilotVersion.flight_custom_version, 8);
            nullStr[8] = 0;
            _vehicle->_gitHash = nullStr;
        }
        if (QGCCorePlugin::instance()->options()->checkFirmwareVersion() && !_vehicle->_checkLatestStableFWDone) {
            _vehicle->_checkLatestStableFWDone = true;
            _vehicle->_firmwarePlugin->checkIfIsLatestStable(_vehicle);
        }
        emit _vehicle->gitHashChanged(_vehicle->_gitHash);

        _vehicle->_setCapabilities(autopilotVersion.capabilities);
    }
        break;
    case Vehicle::RequestMessageFailureCommandError:
        qCDebug(InitialConnectStateMachineLog) << QStringLiteral("REQUEST_MESSAGE:AUTOPILOT_VERSION command error(%1)").arg(commandResult);
        break;
    case Vehicle::RequestMessageFailureCommandNotAcked:
        qCDebug(InitialConnectStateMachineLog) << "REQUEST_MESSAGE:AUTOPILOT_VERSION command never acked";
        break;
    case Vehicle::RequestMessageFailureMessageNotReceived:
        qCDebug(InitialConnectStateMachineLog) << "REQUEST_MESSAGE:AUTOPILOT_VERSION command acked but message never received";
        break;
    case Vehicle::RequestMessageFailureDuplicateCommand:
        qCDebug(InitialConnectStateMachineLog) << "REQUEST_MESSAGE:AUTOPILOT_VERSION Internal Error: Duplicate command";
        break;
    }

    if (failureCode != Vehicle::RequestMessageNoFailure) {
        qCDebug(InitialConnectStateMachineLog) << "REQUEST_MESSAGE:AUTOPILOT_VERSION failed. Setting no capabilities";
        uint64_t assumedCapabilities = MAV_PROTOCOL_CAPABILITY_MAVLINK2;
        if (_vehicle->px4Firmware() || _vehicle->apmFirmware()) {
            // We make some assumptions for known firmware
            assumedCapabilities |= MAV_PROTOCOL_CAPABILITY_MISSION_INT | MAV_PROTOCOL_CAPABILITY_COMMAND_INT |
                                   MAV_PROTOCOL_CAPABILITY_MISSION_FENCE | MAV_PROTOCOL_CAPABILITY_MISSION_RALLY;
        }
        _vehicle->_setCapabilities(assumedCapabilities);
    }

    // Complete the state - need to find and complete it
    if (_stateAutopilotVersion) {
        _stateAutopilotVersion->complete();
    }
}

void InitialConnectStateMachine::_requestStandardModes(AsyncFunctionState* state)
{
    qCDebug(InitialConnectStateMachineLog) << "_stateRequestStandardModes";

    state->connectToCompletion(_vehicle->_standardModes, &StandardModes::requestCompleted);
    _vehicle->_standardModes->request();
}

void InitialConnectStateMachine::_requestCompInfo(AsyncFunctionState* state)
{
    qCDebug(InitialConnectStateMachineLog) << "_stateRequestCompInfo";

    connect(_vehicle->_componentInformationManager, &ComponentInformationManager::progressUpdate,
            this, &InitialConnectStateMachine::_onSubProgressUpdate);

    _vehicle->_componentInformationManager->requestAllComponentInformation(
        [](void* requestAllCompleteFnData) {
            auto* self = static_cast<InitialConnectStateMachine*>(requestAllCompleteFnData);
            disconnect(self->_vehicle->_componentInformationManager, &ComponentInformationManager::progressUpdate,
                       self, &InitialConnectStateMachine::_onSubProgressUpdate);
            if (self->_stateCompInfo) {
                self->_stateCompInfo->complete();
            }
        },
        this
    );
}

void InitialConnectStateMachine::_requestParameters(AsyncFunctionState* state)
{
    qCDebug(InitialConnectStateMachineLog) << "_stateRequestParameters";

    connect(_vehicle->_parameterManager, &ParameterManager::loadProgressChanged,
            this, &InitialConnectStateMachine::_onSubProgressUpdate);

    state->connectToCompletion(_vehicle->_parameterManager, &ParameterManager::parametersReadyChanged,
        [this](bool parametersReady) {
            _onParametersReady(parametersReady);
        });

    _vehicle->_parameterManager->refreshAllParameters();
}

void InitialConnectStateMachine::_onParametersReady(bool parametersReady)
{
    qCDebug(InitialConnectStateMachineLog) << "_onParametersReady" << parametersReady;

    // Disconnect progress tracking from parameter manager
    disconnect(_vehicle->_parameterManager, &ParameterManager::loadProgressChanged,
               this, &InitialConnectStateMachine::_onSubProgressUpdate);

    if (parametersReady) {
        // Send time to vehicle (twice for reliability on noisy links)
        _vehicle->_sendQGCTimeToVehicle();
        _vehicle->_sendQGCTimeToVehicle();

        // Set up auto-disarm signalling
        _vehicle->_setupAutoDisarmSignalling();

        // Note: Speed limits are handled by Vehicle::_parametersReady

        if (_stateParameters) {
            _stateParameters->complete();
        }
    }
}

void InitialConnectStateMachine::_requestMission(SkippableAsyncState* state)
{
    qCDebug(InitialConnectStateMachineLog) << "_stateRequestMission";

    connect(_vehicle->_missionManager, &MissionManager::progressPctChanged,
            this, &InitialConnectStateMachine::_onSubProgressUpdate);

    state->connectToCompletion(_vehicle->_missionManager, &MissionManager::newMissionItemsAvailable);

    // Disconnect progress tracking on exit
    state->setOnExit([this]() {
        disconnect(_vehicle->_missionManager, &MissionManager::progressPctChanged,
                   this, &InitialConnectStateMachine::_onSubProgressUpdate);
    });

    _vehicle->_missionManager->loadFromVehicle();
}

void InitialConnectStateMachine::_requestGeoFence(SkippableAsyncState* state)
{
    qCDebug(InitialConnectStateMachineLog) << "_stateRequestGeoFence";

    connect(_vehicle->_geoFenceManager, &GeoFenceManager::progressPctChanged,
            this, &InitialConnectStateMachine::_onSubProgressUpdate);

    state->connectToCompletion(_vehicle->_geoFenceManager, &GeoFenceManager::loadComplete);

    // Disconnect progress tracking on exit
    state->setOnExit([this]() {
        disconnect(_vehicle->_geoFenceManager, &GeoFenceManager::progressPctChanged,
                   this, &InitialConnectStateMachine::_onSubProgressUpdate);
    });

    _vehicle->_geoFenceManager->loadFromVehicle();
}

void InitialConnectStateMachine::_requestRallyPoints(SkippableAsyncState* state)
{
    qCDebug(InitialConnectStateMachineLog) << "_stateRequestRallyPoints";

    connect(_vehicle->_rallyPointManager, &RallyPointManager::progressPctChanged,
            this, &InitialConnectStateMachine::_onSubProgressUpdate);

    state->connectToCompletion(_vehicle->_rallyPointManager, &RallyPointManager::loadComplete,
        [this]() {
            // Disconnect progress tracking
            disconnect(_vehicle->_rallyPointManager, &RallyPointManager::progressPctChanged,
                       this, &InitialConnectStateMachine::_onSubProgressUpdate);

            // Mark initial plan request complete
            _vehicle->_initialPlanRequestComplete = true;
            emit _vehicle->initialPlanRequestCompleteChanged(true);

            if (_stateRallyPoints) {
                _stateRallyPoints->complete();
            }
        });

    _vehicle->_rallyPointManager->loadFromVehicle();
}

void InitialConnectStateMachine::_signalComplete()
{
    qCDebug(InitialConnectStateMachineLog) << "Signalling initialConnectComplete";
    emit _vehicle->initialConnectComplete();
}
