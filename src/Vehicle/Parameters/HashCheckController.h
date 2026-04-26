#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QPointer>

#include <cstdint>

class HashCheckStateMachine;
class Vehicle;

Q_DECLARE_LOGGING_CATEGORY(HashCheckControllerLog)

/// Thin wrapper around PX4's `_HASH_CHECK` parameter cache-validation protocol.
/// Spawns a one-shot HashCheckStateMachine per arm() call and forwards its timeout
/// signal as cacheOnlyTimedOut / fullDownloadTimedOut depending on arming mode.
/// The cache load itself stays in ParameterManager.
class HashCheckController : public QObject
{
    Q_OBJECT
public:
    explicit HashCheckController(Vehicle* vehicle, QObject* parent = nullptr);

    static constexpr int kHashCheckTimeoutMs = 1000;

    /// Stop any in-flight state machine and clear the done flag — typically called
    /// when starting a fresh refreshAllParameters cycle.
    void reset();

    /// Begin a hash check that will fall back to a full parameter download on timeout.
    void armForFullDownload(uint8_t componentId);

    /// Begin a hash check that signals failure (no fallback) on timeout.
    void armForCacheOnly(uint8_t componentId);

    /// Notify the in-flight state machine that a `_HASH_CHECK` PARAM_VALUE arrived.
    void noteResponseReceived();

    /// Mark the hash-check phase complete. Subsequent arm…() calls reset it.
    void markDone();

    [[nodiscard]] bool isDone() const { return _hashCheckDone; }

    [[nodiscard]] bool isCacheOnly() const { return _cacheOnlyHashCheck; }

    /// Echo the local cache CRC back to the vehicle as a `_HASH_CHECK` PARAM_SET
    /// so the autopilot stops streaming on a cache hit.
    void sendHashAck(int componentId, uint32_t localCrc);

signals:
    /// Emitted when the timer fires while in cache-only mode.
    void cacheOnlyTimedOut();
    /// Emitted when the timer fires while in full-download mode.
    void fullDownloadTimedOut();

    /// Watched by the in-flight HashCheckStateMachine's wait state.
    /// Emitted from noteResponseReceived().
    void responseReceived();

private slots:
    void _onSmTimedOut();

private:
    void _arm(uint8_t componentId, bool cacheOnly);

    Vehicle* _vehicle = nullptr;
    QPointer<HashCheckStateMachine> _activeSm;
    bool _hashCheckDone = false;
    bool _cacheOnlyHashCheck = false;
};
