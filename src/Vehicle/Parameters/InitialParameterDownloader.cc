#include "InitialParameterDownloader.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStandardPaths>

#include "AppMessages.h"
#include "AutoPilotPlugin.h"
#include "FTPManager.h"
#include "HashCheckController.h"
#include "MAVLinkProtocol.h"
#include "ParameterCacheDiagnostics.h"
#include "ParameterManager.h"
#include "QGCLoggingCategory.h"
#include "Vehicle.h"
#include "VehicleLinkManager.h"

QGC_LOGGING_CATEGORY(InitialParameterDownloaderLog, "Vehicle.Parameters.InitialParameterDownloader")
QGC_LOGGING_CATEGORY(InitialParameterDownloaderVerbose1Log, "Vehicle.Parameters.InitialParameterDownloader:verbose1")

InitialParameterDownloader::InitialParameterDownloader(ParameterManager* manager)
    : QObject(manager),
      _manager(manager),
      _vehicle(manager->vehicle()),
      _logReplay(!_vehicle->vehicleLinkManager()->primaryLink().expired() &&
                 _vehicle->vehicleLinkManager()->primaryLink().lock()->isLogReplay()),
      _disableAllRetries(_logReplay),
      _tryftp(_vehicle->apmFirmware())
{
    _paramRequestListTimer.setSingleShot(true);
    _paramRequestListTimer.setInterval(QGC::runningUnitTests() ? kTestInitialRequestIntervalMs
                                                               : kParamRequestListTimeoutMs);
    (void)connect(&_paramRequestListTimer, &QTimer::timeout, this,
                  &InitialParameterDownloader::_paramRequestListTimeout);

    _waitingParamTimeoutTimer.setSingleShot(true);
    _waitingParamTimeoutTimer.setInterval(QGC::runningUnitTests() ? kTestWaitingParamTimeoutMs
                                                                  : kWaitingParamTimeoutMs);
    if (!_logReplay) {
        (void)connect(&_waitingParamTimeoutTimer, &QTimer::timeout, this,
                      &InitialParameterDownloader::_waitingParamTimeout);
    }
}

QString InitialParameterDownloader::_logVehiclePrefix(int componentId) const
{
    return _vehicle->logPrefix(componentId);
}

int InitialParameterDownloader::noteParamReceived(int componentId, const QString& parameterName, int parameterCount,
                                                  int parameterIndex)
{
    _paramRequestListTimer.stop();
    _waitingParamTimeoutTimer.stop();

    if (!_paramCountMap.contains(componentId)) {
        _paramCountMap[componentId] = parameterCount;
        _totalParamCount += parameterCount;
    }

    if (!_waitingReadParamIndexMap.contains(componentId)) {
        auto& indexMap = _waitingReadParamIndexMap[componentId];
        indexMap.reserve(parameterCount);
        for (int waitingIndex = 0; waitingIndex < parameterCount; waitingIndex++) {
            indexMap[waitingIndex] = 0;
        }
        qCDebug(InitialParameterDownloaderLog)
            << _logVehiclePrefix(componentId) << "Seeing component for first time - paramcount:" << parameterCount;
    }

    if (!_waitingReadParamIndexMap[componentId].contains(parameterIndex)) {
        qCDebug(InitialParameterDownloaderVerbose1Log)
            << _logVehiclePrefix(componentId) << "Unrequested param update" << parameterName;
    }

    if (_waitingReadParamIndexMap[componentId].contains(parameterIndex)) {
        _waitingReadParamIndexMap[componentId].remove(parameterIndex);
        (void)_indexBatchQueue.removeOne(parameterIndex);
        _fillIndexBatchQueue(false /* waitingParamTimeout */);
    }

    int waitingReadParamIndexCount = 0;
    for (const auto& [waitingCompId, indexMap] : _waitingReadParamIndexMap.asKeyValueRange()) {
        waitingReadParamIndexCount += indexMap.count();
    }
    if (waitingReadParamIndexCount) {
        qCDebug(InitialParameterDownloaderVerbose1Log)
            << _logVehiclePrefix(componentId) << "waitingReadParamIndexCount:" << waitingReadParamIndexCount;
    }

    if (waitingReadParamIndexCount) {
        Q_ASSERT(!_paramRequestListTimer.isActive());  // invariant: list and wait timers are mutually exclusive
        _waitingParamTimeoutTimer.start();
        qCDebug(InitialParameterDownloaderVerbose1Log)
            << _logVehiclePrefix(-1)
            << "Restarting _waitingParamTimeoutTimer: totalWaitingParamCount:" << waitingReadParamIndexCount;
    } else if (!_manager->hasParametersFor(_vehicle->primaryComponentId())) {
        Q_ASSERT(!_paramRequestListTimer.isActive());
        qCDebug(InitialParameterDownloaderLog)
            << _logVehiclePrefix(-1)
            << "Restarting _waitingParamTimeoutTimer (still waiting for default component params)";
        _waitingParamTimeoutTimer.start();
    } else {
        qCDebug(InitialParameterDownloaderVerbose1Log)
            << _logVehiclePrefix(-1) << "Not restarting _waitingParamTimeoutTimer (all requests satisfied)";
    }

    _updateProgressBar();

    _prevWaitingReadParamIndexCount = waitingReadParamIndexCount;
    return waitingReadParamIndexCount;
}

void InitialParameterDownloader::_updateProgressBar()
{
    int waitingReadParamIndexCount = 0;
    for (const auto& [compId, indexMap] : _waitingReadParamIndexMap.asKeyValueRange()) {
        waitingReadParamIndexCount += indexMap.count();
    }

    if (waitingReadParamIndexCount == 0) {
        if (_readParamIndexProgressActive) {
            _readParamIndexProgressActive = false;
            _manager->setLoadProgress(0.0);
        }
        return;
    }

    _readParamIndexProgressActive = true;
    const double rawProgress =
        static_cast<double>(_totalParamCount - waitingReadParamIndexCount) / static_cast<double>(_totalParamCount);
    // Denominator grows as new components stream their first PARAM_VALUE, which would otherwise
    // make the bar retreat. Clamp to monotonic-increasing within an attempt; reset/start() flips
    // _readParamIndexProgressActive back to false and we re-baseline.
    if (rawProgress < _lastReportedProgress) {
        return;
    }
    _lastReportedProgress = rawProgress;
    _manager->setLoadProgress(rawProgress);
}

bool InitialParameterDownloader::_fillIndexBatchQueue(bool waitingParamTimeout)
{
    if (!_indexBatchQueueActive) {
        return false;
    }

    constexpr int maxBatchSize = 10;

    if (waitingParamTimeout) {
        qCDebug(InitialParameterDownloaderLog) << "Refilling index based batch queue due to timeout";
        _indexBatchQueue.clear();
    } else {
        qCDebug(InitialParameterDownloaderLog) << "Refilling index based batch queue due to received parameter";
    }

    for (auto&& [componentId, indexRetryMap] : _waitingReadParamIndexMap.asKeyValueRange()) {
        if (indexRetryMap.count()) {
            qCDebug(InitialParameterDownloaderLog)
                << _logVehiclePrefix(componentId) << "_waitingReadParamIndexMap count" << indexRetryMap.count();
            qCDebug(InitialParameterDownloaderVerbose1Log)
                << _logVehiclePrefix(componentId) << "_waitingReadParamIndexMap" << indexRetryMap;
        }

        for (auto it = indexRetryMap.begin(); it != indexRetryMap.end();) {
            const int paramIndex = it.key();

            if (_indexBatchQueue.contains(paramIndex)) {
                ++it;
                continue;
            }

            if (_indexBatchQueue.count() > maxBatchSize) {
                break;
            }

            it.value()++;
            if (_disableAllRetries || (it.value() > kMaxInitialLoadRetrySingleParam)) {
                _failedReadParamIndexMap[componentId] << paramIndex;
                qCDebug(InitialParameterDownloaderLog)
                    << _logVehiclePrefix(componentId) << "Giving up on (paramIndex:" << paramIndex
                    << "retryCount:" << it.value() << ")";
                it = indexRetryMap.erase(it);
            } else {
                _indexBatchQueue.append(paramIndex);
                _manager->mavlinkParamRequestRead(componentId, QString(), paramIndex, false /* notifyFailure */);
                qCDebug(InitialParameterDownloaderLog)
                    << _logVehiclePrefix(componentId) << "Read re-request for (paramIndex:" << paramIndex
                    << "retryCount:" << it.value() << ")";
                ++it;
            }
        }
    }

    return (!_indexBatchQueue.isEmpty());
}

void InitialParameterDownloader::_waitingParamTimeout()
{
    if (_logReplay) {
        return;
    }

    qCDebug(InitialParameterDownloaderLog) << _logVehiclePrefix(-1) << "_waitingParamTimeout";

    _indexBatchQueueActive = true;

    bool paramsRequested = _fillIndexBatchQueue(true /* waitingParamTimeout */);
    if (!paramsRequested && !_waitingForDefaultComponent &&
        !_manager->hasParametersFor(_vehicle->primaryComponentId())) {
        qCDebug(InitialParameterDownloaderLog)
            << _logVehiclePrefix(-1)
            << "Restarting _waitingParamTimeoutTimer - still don't have default component params"
            << _vehicle->primaryComponentId();
        _waitingParamTimeoutTimer.start();
        _waitingForDefaultComponent = true;
        return;
    }
    _waitingForDefaultComponent = false;

    checkInitialLoadComplete();

    if (paramsRequested) {
        qCDebug(InitialParameterDownloaderLog)
            << _logVehiclePrefix(-1) << "Restarting _waitingParamTimeoutTimer - re-request";
        _waitingParamTimeoutTimer.start();
    }
}

void InitialParameterDownloader::checkInitialLoadComplete()
{
    if (_initialLoadComplete) {
        return;
    }

    for (const auto& [compId, indexMap] : _waitingReadParamIndexMap.asKeyValueRange()) {
        Q_UNUSED(compId);
        if (!indexMap.isEmpty()) {
            return;
        }
    }

    if (!_manager->hasParametersFor(_vehicle->primaryComponentId())) {
        return;
    }

    _initialLoadComplete = true;

    if (!_logReplay) {
        _manager->_cacheDiag->logUnseen();
    }
    _manager->_cacheDiag->clear();

    qCDebug(InitialParameterDownloaderLog) << _logVehiclePrefix(-1) << "Initial load complete";

    QString indexList;
    bool initialLoadFailures = false;
    for (const auto& [failCompId, failedIndices] : _failedReadParamIndexMap.asKeyValueRange()) {
        for (const int paramIndex : failedIndices) {
            if (initialLoadFailures) {
                indexList += ", ";
            }
            indexList += QStringLiteral("%1:%2").arg(failCompId).arg(paramIndex);
            initialLoadFailures = true;
            qCDebug(InitialParameterDownloaderLog)
                << _logVehiclePrefix(failCompId)
                << "Gave up on initial load after max retries (paramIndex:" << paramIndex << ")";
        }
    }

    if (initialLoadFailures) {
        const QString errorMsg =
            tr("%1 was unable to retrieve the full set of parameters from vehicle %2. "
               "This will cause %1 to be unable to display its full user interface. "
               "If you are using modified firmware, you may need to resolve any vehicle startup errors to resolve the "
               "issue. "
               "If you are using standard firmware, you may need to upgrade to a newer version to resolve the issue.")
                .arg(QCoreApplication::applicationName())
                .arg(_vehicle->id());
        qCDebug(InitialParameterDownloaderLog) << errorMsg;
        QGC::showAppMessage(errorMsg);
        if (!QGC::runningUnitTests()) {
            qCWarning(InitialParameterDownloaderLog)
                << _logVehiclePrefix(-1)
                << "The following parameter indices could not be loaded after the maximum number of retries:"
                << indexList;
        }
    }

    _manager->finalizeInitialLoad(initialLoadFailures,
                                  [this] { _vehicle->autopilotPlugin()->parametersReadyPreChecks(); });
}

void InitialParameterDownloader::markComponentFullyDownloaded(int componentId, int paramCount)
{
    _paramCountMap[componentId] = paramCount;
    _totalParamCount += paramCount;
    _waitingReadParamIndexMap[componentId] = {};
    checkInitialLoadComplete();
}

void InitialParameterDownloader::start(uint8_t componentId)
{
    const SharedLinkInterfacePtr sharedLink = _vehicle->vehicleLinkManager()->primaryLink().lock();
    if (!sharedLink) {
        return;
    }

    // Note: post-_initialLoadComplete this acts as a silent refresh — checkInitialLoadComplete()
    // short-circuits on the flag, so neither parametersReady nor the failure toast re-fire. The
    // _failedReadParamIndexMap stays populated but is no longer read anywhere; that's harmless.

    if (sharedLink->linkConfiguration()->isHighLatency() || _logReplay) {
        markLogReplayOrHighLatencyComplete();
        return;
    }

    if (_tryftp && ((componentId == MAV_COMP_ID_ALL) || (componentId == MAV_COMP_ID_AUTOPILOT1))) {
        if (!_initialLoadComplete) {
            _paramRequestListTimer.start();
        }
        FTPManager* const ftpManager = _vehicle->ftpManager();
        (void)connect(ftpManager, &FTPManager::downloadComplete, this,
                      &InitialParameterDownloader::_ftpDownloadComplete);
        _waitingParamTimeoutTimer.stop();
        if (ftpManager->download(MAV_COMP_ID_AUTOPILOT1, QStringLiteral("@PARAM/param.pck?withdefaults=1"),
                                 QStandardPaths::writableLocation(QStandardPaths::TempLocation),
                                 QStringLiteral("param.pck"), false /* No filesize check */)) {
            (void)connect(ftpManager, &FTPManager::commandProgress, this,
                          &InitialParameterDownloader::_ftpDownloadProgress);
        } else {
            qCWarning(InitialParameterDownloaderLog) << "FTPManager::download returned failure";
            (void)disconnect(ftpManager, &FTPManager::downloadComplete, this,
                             &InitialParameterDownloader::_ftpDownloadComplete);
        }
    } else if (_vehicle->px4Firmware() && !_initialLoadComplete && !_manager->_hashCheck->isDone()) {
        qCDebug(InitialParameterDownloaderLog)
            << _logVehiclePrefix(-1) << "Requesting _HASH_CHECK before full parameter list";
        const uint8_t hashCheckCompId =
            (componentId == MAV_COMP_ID_ALL) ? static_cast<uint8_t>(MAV_COMP_ID_AUTOPILOT1) : componentId;
        _manager->_hashCheck->armForFullDownload(hashCheckCompId);
    } else {
        if (!_initialLoadComplete) {
            _paramRequestListTimer.start();
        }

        for (const auto& [cid, paramCount] : _paramCountMap.asKeyValueRange()) {
            if ((componentId != MAV_COMP_ID_ALL) && (componentId != cid)) {
                continue;
            }
            for (int waitingIndex = 0; waitingIndex < paramCount; waitingIndex++) {
                _waitingReadParamIndexMap[cid][waitingIndex] = 0;
            }
        }

        mavlink_message_t msg{};
        mavlink_msg_param_request_list_pack_chan(MAVLinkProtocol::instance()->getSystemId(),
                                                 MAVLinkProtocol::getComponentId(), sharedLink->mavlinkChannel(), &msg,
                                                 _vehicle->id(), componentId);
        (void)_vehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), msg);
    }

    const QString what = (componentId == MAV_COMP_ID_ALL) ? "MAV_COMP_ID_ALL" : QString::number(componentId);
    qCDebug(InitialParameterDownloaderLog)
        << _logVehiclePrefix(-1) << "Request to refresh all parameters for component ID:" << what;
}

void InitialParameterDownloader::markLogReplayOrHighLatencyComplete()
{
    _initialLoadComplete = true;
    _waitingForDefaultComponent = false;
    _manager->finalizeInitialLoad(true);
}

void InitialParameterDownloader::cancel()
{
    _paramRequestListTimer.stop();
    _waitingParamTimeoutTimer.stop();
    _initialLoadComplete = true;
}

void InitialParameterDownloader::_paramRequestListTimeout()
{
    if (_logReplay) {
        qCDebug(InitialParameterDownloaderLog)
            << _logVehiclePrefix(-1) << "_paramRequestListTimeout (log replay): Signalling load complete";
        _initialLoadComplete = true;
        _manager->finalizeInitialLoad(false,
                                      [this] { _vehicle->autopilotPlugin()->parametersReadyPreChecks(); });
        return;
    }

    if (!_disableAllRetries && (++_initialRequestRetryCount <= kMaxInitialRequestListRetry)) {
        qCDebug(InitialParameterDownloaderLog) << _logVehiclePrefix(-1) << "Retrying initial parameter request list";
        start(MAV_COMP_ID_ALL);
    } else if (!_vehicle->genericFirmware()) {
        const QString errorMsg = tr("Vehicle %1 did not respond to request for parameters. "
                                    "This will cause %2 to be unable to display its full user interface.")
                                     .arg(_vehicle->id())
                                     .arg(QCoreApplication::applicationName());
        qCDebug(InitialParameterDownloaderLog) << errorMsg;
        QGC::showAppMessage(errorMsg);
    }
}

void InitialParameterDownloader::_ftpDownloadComplete(const QString& fileName, const QString& errorMsg)
{
    bool continueWithDefaultParameterdownload = true;
    bool immediateRetry = false;

    (void)disconnect(_vehicle->ftpManager(), &FTPManager::downloadComplete, this,
                     &InitialParameterDownloader::_ftpDownloadComplete);
    (void)disconnect(_vehicle->ftpManager(), &FTPManager::commandProgress, this,
                     &InitialParameterDownloader::_ftpDownloadProgress);

    if (errorMsg.isEmpty()) {
        qCDebug(InitialParameterDownloaderLog) << "Parameter file received:" << fileName;
        if (_manager->parseParamPackFile(fileName)) {
            qCDebug(InitialParameterDownloaderLog) << "Parsed!";
            return;
        }
        qCDebug(InitialParameterDownloaderLog) << "Error in parameter file";
        /* This should not happen... */
    } else if (errorMsg.contains("File Not Found")) {
        qCDebug(InitialParameterDownloaderLog)
            << "ftp: No Parameterfile on vehicle - Start Conventional Parameter Download";
        if (_initialRequestRetryCount == 0) {
            immediateRetry = true;
        }
    } else if ((_manager->loadProgress() > 0.0001) && (_manager->loadProgress() < 0.01)) {
        qCDebug(InitialParameterDownloaderLog) << "ftp progress too slow - Start Conventional Parameter Download";
    } else if (_initialRequestRetryCount == 1) {
        qCDebug(InitialParameterDownloaderLog) << "ftp: Too many retries - Start Conventional Parameter Download";
    } else {
        qCDebug(InitialParameterDownloaderLog) << "ftp Retry:" << _initialRequestRetryCount;
        continueWithDefaultParameterdownload = false;
    }

    if (continueWithDefaultParameterdownload) {
        _tryftp = false;
        _initialRequestRetryCount = 0;
        if (immediateRetry) {
            _paramRequestListTimeout();
        } else {
            _paramRequestListTimer.start();
        }
    } else {
        _paramRequestListTimer.start();
    }
}

void InitialParameterDownloader::_ftpDownloadProgress(float progress)
{
    qCDebug(InitialParameterDownloaderVerbose1Log) << "ftpDownloadProgress:" << progress;
    _manager->setLoadProgress(static_cast<double>(progress));
    if (progress > 0.001) {
        _paramRequestListTimer.stop();
    }
}
