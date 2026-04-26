#pragma once

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>

class ParameterManager;
class Vehicle;

Q_DECLARE_LOGGING_CATEGORY(InitialParameterDownloaderLog)
Q_DECLARE_LOGGING_CATEGORY(InitialParameterDownloaderVerbose1Log)

/// Drives the initial download of every parameter on the vehicle, including
/// the PARAM_REQUEST_LIST retry loop, the per-index re-request batch queue,
/// the per-parameter response-wait timer, and the FTP @PARAM/param.pck
/// fast-path on ArduPilot.
///
/// ParameterManager owns one downloader and routes incoming PARAM_VALUE
/// messages through @ref noteParamReceived after creating/updating the Fact.
/// The downloader emits its progress changes via PM-facing setters.
///
/// Timer state model — only one of the two QTimers is ever expected to be running:
///
///   AwaitListReply  → _paramRequestListTimer running. We sent PARAM_REQUEST_LIST and have not
///                     yet received any PARAM_VALUE for this attempt. Timeout retries the request
///                     up to kMaxInitialRequestListRetry times.
///   StreamReceive   → _waitingParamTimeoutTimer running. At least one PARAM_VALUE arrived; we are
///                     waiting on the remaining indices in _waitingReadParamIndexMap. Timeout
///                     batches up to maxBatchSize PARAM_REQUEST_READs (per-index retry) and
///                     restarts itself.
///   Done            → both timers stopped. _initialLoadComplete is true.
///
/// Transitions:
///   start()                → AwaitListReply (or Done for log-replay/high-latency)
///   noteParamReceived()    → StreamReceive (stops list timer, starts wait timer)
///   _waitingParamTimeout() → StreamReceive (re-issues batched PARAM_REQUEST_READs)
///   checkInitialLoadComplete() → Done
class InitialParameterDownloader : public QObject
{
    Q_OBJECT

public:
    InitialParameterDownloader(ParameterManager* manager);

    /// Bookkeeping for one PARAM_VALUE: updates per-component count, removes
    /// the parameter from the wait map, refills the batch queue, restarts the
    /// per-param timeout if needed, and updates the progress bar. Returns the
    /// post-update count of indices still waiting (used by the caller to
    /// decide whether to write the on-disk param cache).
    int noteParamReceived(int componentId, const QString& parameterName, int parameterCount, int parameterIndex);

    /// Final step of @ref noteParamReceived's caller: if every component has
    /// reported in (and the default component is present), mark
    /// _initialLoadComplete and emit the appropriate ParameterManager signals.
    void checkInitialLoadComplete();

    /// Begin a fresh download attempt for @p componentId. Picks between
    /// FTP fast-path (APM), PX4 _HASH_CHECK arming, or vanilla
    /// PARAM_REQUEST_LIST depending on firmware and prior state.
    void start(uint8_t componentId);

    /// Used by ParameterManager::tryHashCheckCacheLoad and similar non-download
    /// links (high-latency / log replay) to advance to the "load complete,
    /// nothing came back" terminal state.
    void markLogReplayOrHighLatencyComplete();

    /// Stop both timers and force the terminal state. Idempotent. Use for intentional
    /// mid-download aborts (e.g. user disconnected the vehicle). QObject parent-child
    /// already handles object-lifetime cleanup; this just guarantees no callbacks fire
    /// after the caller proceeds.
    void cancel();

    // Queries used by ParameterManager
    bool initialLoadComplete() const { return _initialLoadComplete; }

    bool tryFtp() const { return _tryftp; }

    bool waitingForListResponse() const { return _paramRequestListTimer.isActive(); }

    QList<int> componentIds() const { return _paramCountMap.keys(); }

    /// Count of indices still outstanding after the most recent noteParamReceived call.
    int waitingReadParamIndexCount() const { return _prevWaitingReadParamIndexCount; }

    /// Stop the per-parameter wait timer (called from the cache-load fast
    /// path before injecting the parameters from disk).
    void stopParamRequestListTimer() { _paramRequestListTimer.stop(); }

    /// Mark @p componentId as fully populated by the FTP @PARAM/param.pck
    /// fast-path (no per-index PARAM_REQUEST_READ traffic needed).
    void markComponentFullyDownloaded(int componentId, int paramCount);

    // Timeouts (public for unit tests).
    static constexpr int kMaxInitialRequestListRetry = 4;
    static constexpr int kMaxInitialLoadRetrySingleParam = 5;
    static constexpr int kParamRequestListTimeoutMs = 5000;
    static constexpr int kWaitingParamTimeoutMs = 3000;
    static constexpr int kTestInitialRequestIntervalMs = 500;
    static constexpr int kTestWaitingParamTimeoutMs = 500;
    static constexpr int kTestMaxInitialRequestTimeMs =
        (kMaxInitialRequestListRetry + 1) * kTestInitialRequestIntervalMs + 1000;

private slots:
    void _paramRequestListTimeout();
    void _waitingParamTimeout();
    void _ftpDownloadComplete(const QString& fileName, const QString& errorMsg);
    void _ftpDownloadProgress(float progress);

private:
    bool _fillIndexBatchQueue(bool waitingParamTimeout);
    void _updateProgressBar();
    QString _logVehiclePrefix(int componentId) const;

    ParameterManager* _manager = nullptr;
    Vehicle* _vehicle = nullptr;

    QTimer _paramRequestListTimer;
    QTimer _waitingParamTimeoutTimer;

    const bool _logReplay;
    const bool _disableAllRetries;
    bool _tryftp;  ///< Cleared on FTP fast-path failure to force MAVLink fallback.

    bool _initialLoadComplete = false;
    bool _waitingForDefaultComponent = false;
    bool _readParamIndexProgressActive = false;
    bool _indexBatchQueueActive = false;

    int _initialRequestRetryCount = 0;
    int _prevWaitingReadParamIndexCount = 0;
    int _totalParamCount = 0;
    double _lastReportedProgress = 0.0;

    QList<int> _indexBatchQueue;
    QHash<int, int> _paramCountMap;
    // Inner: paramIndex → retry count. Inserted up to parameterCount entries on first sight per
    // component; QHash avoids the QMap rb-tree rebalancing on bulk insert.
    QHash<int, QHash<int, int>> _waitingReadParamIndexMap;
    QHash<int, QList<int>> _failedReadParamIndexMap;
};
