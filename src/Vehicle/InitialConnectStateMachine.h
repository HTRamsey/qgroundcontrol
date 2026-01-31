#pragma once

#include "QGCStateMachine.h"
#include "MAVLinkLib.h"

#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(InitialConnectStateMachineLog)

class Vehicle;
class SkippableAsyncState;
class AsyncFunctionState;
class FunctionState;

/// State machine for initial vehicle connection sequence.
/// Handles requesting autopilot version, standard modes, component info,
/// parameters, missions, geofence, and rally points in sequence.
///
/// Uses weighted progress tracking where different states contribute
/// different amounts to the overall progress (e.g., parameter loading
/// takes longer than version request).
class InitialConnectStateMachine : public QGCStateMachine
{
    Q_OBJECT

public:
    explicit InitialConnectStateMachine(Vehicle* vehicle, QObject* parent = nullptr);
    ~InitialConnectStateMachine() override;

    void start();

signals:
    void progressUpdate(float progress);

private slots:
    void _onSubProgressUpdate(double progressValue);

private:
    // State creation
    void _createStates();
    void _wireTransitions();
    void _wireProgressTracking();

    // State callbacks
    void _requestAutopilotVersion(AsyncFunctionState* state);
    void _handleAutopilotVersionResult(MAV_RESULT commandResult,
                                       Vehicle::RequestMessageResultHandlerFailureCode_t failureCode,
                                       const mavlink_message_t& message);
    void _requestStandardModes(AsyncFunctionState* state);
    void _requestCompInfo(AsyncFunctionState* state);
    void _requestParameters(AsyncFunctionState* state);
    void _onParametersReady(bool ready);
    void _requestMission(SkippableAsyncState* state);
    void _requestGeoFence(SkippableAsyncState* state);
    void _requestRallyPoints(SkippableAsyncState* state);
    void _signalComplete();

    // Skip predicates
    bool _shouldSkipForLinkType() const;
    bool _hasPrimaryLink() const;

    // Progress tracking
    class ProgressTracker {
    public:
        void initialize(const QList<int>& weights);
        void setCurrentState(int index);
        void setSubProgress(float subProgress);
        float progress() const;
        void reset();  // Reset for new connection cycle
    private:
        QList<int> _weights;
        int _totalWeight = 0;
        int _currentIndex = 0;
        float _subProgress = 0.0f;
        mutable float _lastEmittedProgress = 0.0f;  // Track highest emitted progress
    };

    void _updateProgress(float subProgress = 0.0f);

    Vehicle* _vehicle = nullptr;
    ProgressTracker _progressTracker;

    // State pointers for wiring
    AsyncFunctionState* _stateAutopilotVersion = nullptr;
    AsyncFunctionState* _stateStandardModes = nullptr;
    AsyncFunctionState* _stateCompInfo = nullptr;
    AsyncFunctionState* _stateParameters = nullptr;
    SkippableAsyncState* _stateMission = nullptr;
    SkippableAsyncState* _stateGeoFence = nullptr;
    SkippableAsyncState* _stateRallyPoints = nullptr;
    FunctionState* _stateComplete = nullptr;
    QGCFinalState* _stateFinal = nullptr;

    // Progress weights matching old implementation
    static constexpr int _progressWeights[] = {
        1,  // _stateAutopilotVersion
        1,  // _stateStandardModes
        5,  // _stateCompInfo
        5,  // _stateParameters
        2,  // _stateMission
        1,  // _stateGeoFence
        1,  // _stateRallyPoints
        1   // _stateComplete
    };
    static constexpr int _stateCount = sizeof(_progressWeights) / sizeof(_progressWeights[0]);
};
