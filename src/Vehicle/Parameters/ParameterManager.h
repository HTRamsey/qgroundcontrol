#pragma once

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtQmlIntegration/QtQmlIntegration>

#include <functional>
#include <limits>
#include <memory>

#include "Fact.h"
#include "MAVLinkEnums.h"
#include "QGCMAVLinkTypes.h"

class ParameterCacheDiagnostics;

class QTextStream;

class HashCheckController;
class InitialParameterDownloader;
class ParameterEditorController;
class Vehicle;

/// All public methods must be called on the owning Vehicle's thread (the main thread in QGC).
/// MAVLink messages reach @ref mavlinkMessageReceived via Vehicle's queued signal connection;
/// do not call it directly from a link thread.
class ParameterManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    // Sibling classes inside the Parameters subsystem reach into private helpers below.
    friend class InitialParameterDownloader;
    friend class OfflineParameterLoader;
    friend class ParameterSetStateMachine;
    friend class ParameterRequestReadStateMachine;

    Q_PROPERTY(bool parametersReady READ parametersReady NOTIFY
                   parametersReadyChanged)    ///< true: Parameters are ready for use
    Q_PROPERTY(bool missingParameters READ missingParameters NOTIFY
                   missingParametersChanged)  ///< true: Parameters are missing from firmware response, false: all
                                              ///< parameters received from firmware
    Q_PROPERTY(double loadProgress READ loadProgress NOTIFY loadProgressChanged)
    Q_PROPERTY(bool pendingWrites READ pendingWrites NOTIFY
                   pendingWritesChanged)  ///< true: There are still pending write updates against the vehicle
    Q_PROPERTY(int pendingWritesCount READ pendingWritesCount NOTIFY
                   pendingWritesCountChanged)  ///< Number of in-flight PARAM_SETs awaiting ack
    Q_PROPERTY(
        bool parameterDownloadSkipped READ parameterDownloadSkipped NOTIFY
            parameterDownloadSkippedChanged)  ///< true: Parameter download was intentionally skipped (e.g. flying)
public:
    ParameterManager(Vehicle* vehicle);
    ~ParameterManager() override;

    bool parametersReady() const { return _parametersReady; }

    bool missingParameters() const { return _missingParameters; }

    double loadProgress() const { return _loadProgress; }

    bool parameterDownloadSkipped() const { return _parameterDownloadSkipped; }

    /// Set externally by InitialConnectStateMachine when arming/flying blocks the initial download.
    /// Cleared inside @ref refreshAllParameters; QML uses the flag to render "Parameters not available"
    /// until the user (or auto-refresh on disarm) requests another download.
    void setParameterDownloadSkipped(bool skipped);

    void mavlinkMessageReceived(const mavlink_message_t& message);

    QList<int> componentIds() const;

    /// Re-request the full set of parameters from the autopilot.
    void refreshAllParameters(uint8_t componentID);

    Q_INVOKABLE void refreshAllParameters() { refreshAllParameters(MAV_COMP_ID_ALL); }

    /// Attempt a PX4 hash-check cache load only. If the cache misses or the
    /// vehicle is not PX4, cacheCheckOnlyFailed() is emitted.
    void tryHashCheckCacheLoad();

    /// Request a refresh on the specific parameter
    void refreshParameter(int componentId, const QString& paramName);

    /// Request a refresh on all parameters that begin with the specified prefix.
    /// Failures are logged but do not raise per-param dialogs (would spam on flaky links).
    void refreshParametersPrefix(int componentId, const QString& namePrefix);

    void resetAllParametersToDefaults();
    void resetAllToVehicleConfiguration();

    /// Returns true if the specifed parameter exists
    ///     @param componentId: Component id or ParameterManager::anyComponentId
    ///     @param name: Parameter name
    [[nodiscard]] bool parameterExists(int componentId, const QString& paramName) const;

    /// Returns all parameter names
    [[nodiscard]] QStringList parameterNames(int componentId) const;

    /// Looks up a parameter by component+name. Returns nullptr if the parameter is unknown.
    /// Pair with @ref parameterExists when the caller wants to branch on presence; use
    /// @ref requireParameter when missing-is-a-bug.
    ///     @param componentId: Component id or ParameterManager::anyComponentId
    ///     @param name: Parameter name
    [[nodiscard]] Fact* getParameter(int componentId, const QString& paramName);

    /// Returns the parameter as a reference, signalling the caller asserts it exists.
    /// On miss, pops a user-visible "missing parameter" message and returns a silent stand-in
    /// fact whose value is zero — preserving the legacy never-null behaviour for callers that
    /// were relying on it.
    [[nodiscard]] Fact& requireParameter(int componentId, const QString& paramName);

    /// Returns error messages from loading
    [[nodiscard]] QString readParametersFromStream(QTextStream& stream);

    void writeParametersToStream(QTextStream& stream);

    [[nodiscard]] bool pendingWrites() const;

    [[nodiscard]] int pendingWritesCount() const { return _pendingWritesCount; }

    Vehicle* vehicle();

    /// Send a PARAM_SET via the state machine; tracks pending-write count.
    /// Public because ParameterEditorController calls this when applying a `.params` diff for
    /// parameters that don't yet exist locally on the vehicle (so getParameter()->setRawValue
    /// won't reach the wire). Most callers should set the value via the corresponding Fact.
    void mavlinkParamSet(int componentId, const QString& name, FactMetaData::ValueType_t valueType,
                         const QVariant& rawValue);

    static constexpr int anyComponentId = -1;

    /// QML accessor for @ref anyComponentId (QML cannot reference C++ static constexpr members directly).
    Q_PROPERTY(int anyComponentIdConstant READ anyComponentIdConstant CONSTANT)
    int anyComponentIdConstant() const { return anyComponentId; }

    // These are public for creating unit tests
    static constexpr int kParamSetRetryCount = 2;          ///< Number of retries for PARAM_SET
    static constexpr int kParamRequestReadRetryCount = 2;  ///< Number of retries for PARAM_REQUEST_READ
    static constexpr int kWaitForParamValueAckMs = 1000;   ///< Time to wait for param value ack after set param
    /// MAVLink "no index" sentinel in PARAM_VALUE.param_index — ArduPilot streams unsolicited values with this set.
    static constexpr int kUnsolicitedParamIndex = std::numeric_limits<uint16_t>::max();
    /// Kept for test compatibility; mirrors InitialParameterDownloader::kTestMaxInitialRequestTimeMs.
    static constexpr int kTestMaxInitialRequestTimeMs = ((4 + 1) * 500) + 1000;

signals:
    void parametersReadyChanged(bool parametersReady);
    void missingParametersChanged(bool missingParameters);
    void loadProgressChanged(float value);
    void cacheCheckOnlyFailed();
    void pendingWritesChanged(bool pendingWrites);
    void pendingWritesCountChanged(int count);
    void parameterDownloadSkippedChanged();
    void factAdded(int componentId, Fact* fact);

    // These signals are used to verify unit tests
    void _paramSetSuccess(int componentId, const QString& paramName);
    void _paramSetFailure(int componentId, const QString& paramName);
    void _paramRequestReadSuccess(int componentId, const QString& paramName, int paramIndex);
    void _paramRequestReadFailure(int componentId, const QString& paramName, int paramIndex);

private slots:
    void _factRawValueUpdated(const QVariant& rawValue);

private:
    // ---- Internal API — only the friend-listed sibling classes inside the Parameters
    // subsystem may call these. Public-API consumers should use Fact-based access instead. ----

    /// Send a PARAM_REQUEST_READ via the state machine.
    void mavlinkParamRequestRead(int componentId, const QString& paramName, int paramIndex, bool notifyFailure);
    /// Parse the binary @PARAM/param.pck FTP fast-path file and inject params.
    /// See: https://github.com/ArduPilot/ardupilot/tree/master/libraries/AP_Filesystem
    bool parseParamPackFile(const QString& filename);
    /// Look up or construct the Fact for (componentId, name); wires metadata + value-changed signal on creation.
    /// Caller is responsible for emitting factAdded after any pre-emit setup it needs (e.g. setting defaults).
    Fact* ensureFact(int componentId, const QString& parameterName, FactMetaData::ValueType_t type, bool& isNew);
    /// Insert a caller-constructed Fact into the per-component map; does NOT wire the value-changed forwarder.
    /// Used by the offline-editing loader where there is no vehicle to send PARAM_SETs to.
    /// Returns true if the Fact was inserted; false (and ownership unchanged) if the slot was already occupied.
    [[nodiscard]] bool registerFact(int componentId, const QString& parameterName, Fact* fact);

    /// Clear cache-diagnostics state (used by paths that reload the fact map from disk).
    void resetCacheDiagnostics();

    void setLoadProgress(double progress);
    /// Stage ready/missing flags, run @p preEmitHook (e.g. AutoPilotPlugin::parametersReadyPreChecks)
    /// while listeners still see parametersReady==false, then emit parametersReadyChanged(true) followed
    /// by missingParametersChanged(@p anyMissing).
    void finalizeInitialLoad(bool anyMissing, const std::function<void()>& preEmitHook = {});
    /// Drop ready/missing back to the un-ready state (used by cache-only-failure paths).
    void setParametersReady(bool ready);

    void noteWriteStarted();
    void noteWriteFinished();

    [[nodiscard]] bool hasParametersFor(int componentId) const { return _mapCompId2FactMap.contains(componentId); }

    /// Render the "veh: N comp: M" disambiguation prefix used in log lines.
    [[nodiscard]] QString vehicleAndComponentString(int componentId) const;

    /// Called whenever a parameter is updated or first seen.
    void _handleParamValue(int componentId, const QString& parameterName, int parameterCount, int parameterIndex,
                           MAV_PARAM_TYPE mavParamType, const QVariant& parameterValue);
    /// Translates ParameterManager::anyComponentId to real component id if needed
    int _actualComponentId(int componentId) const;
    /// Single-remap lookup. Caller must pre-resolve componentId via @ref _actualComponentId
    /// and pre-remap @p mappedParamName via @ref ParameterNameRemapper::remap.
    Fact* _lookup(int componentId, const QString& mappedParamName);
    /// Internal sibling of @ref refreshParameter that suppresses the user-visible failure dialog.
    /// Used by @ref refreshParametersPrefix where 50 dialogs on a flaky link is bad UX.
    void _refreshParameterQuiet(int componentId, const QString& paramName);
    void _tryCacheHashLoad(int vehicleId, int componentId, const QVariant& hashValue);
    void _animateCacheLoadProgress();
    QString _logVehiclePrefix(int componentId) const;

    Vehicle* _vehicle = nullptr;

    QHash<int /* comp id */, QHash<QString /* parameter name */, Fact*>> _mapCompId2FactMap;

    double _loadProgress = 0;                ///< Parameter load progess, [0.0,1.0]
    bool _parametersReady = false;           ///< true: parameter load complete
    bool _parameterDownloadSkipped = false;  ///< true: parameter download was intentionally skipped
    bool _missingParameters = false;         ///< true: parameter missing from initial load
    bool _logReplay = false;                 ///< true: running with log replay link
    bool _defaultFactWriteWarned = false;    ///< true: already emitted the loud sentinel-write warning

    /// PIMPL — keeps ParameterCacheDiagnostics.h out of this header so changes to the
    /// diagnostics struct don't trigger recompiles of the 30+ TUs that include this file.
    std::unique_ptr<ParameterCacheDiagnostics> _cacheDiag;

    int _pendingWritesCount = 0;  ///< Number of parameters with pending writes

    HashCheckController* _hashCheck = nullptr;
    InitialParameterDownloader* _downloader = nullptr;

    Fact _defaultFact;  ///< Used to return default fact, when parameter not found

    /// Coalescing state for @ref mavlinkParamSet: per-(comp,name), zero or one in-flight write.
    /// While a write is in flight, additional setRawValue calls stash only the latest target;
    /// the in-flight completion handler then dispatches one follow-up write if the stash differs
    /// from the value that was just acked. Prevents N back-to-back QML toggles from spawning N
    /// state machines that race with each other.
    struct PendingSet
    {
        FactMetaData::ValueType_t valueType = FactMetaData::valueTypeInt32;
        QVariant rawValue;
    };
    QHash<QPair<int, QString>, PendingSet> _pendingLatestSets;
    QSet<QPair<int, QString>> _inFlightSets;

    /// Per-(comp,name) pairs already reported as missing. Prevents toast/dialog spam when a QML
    /// binding evaluates against a missing parameter on every property tick.
    QSet<QPair<int, QString>> _reportedMissingParams;
};
