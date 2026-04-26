#include "ParameterManager.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QDir>
#include <QtCore/QEasingCurve>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtCore/QVariantAnimation>
#include <QtCore/QtMath>

#include "AppMessages.h"
#include "AutoPilotPlugin.h"
#include "CompInfoParam.h"
#include "ComponentInformationManager.h"
#include "FTPManager.h"
#include "FactGroup.h"
#include "FirmwarePlugin.h"
#include "HashCheckController.h"
#include "InitialParameterDownloader.h"
#include "MultiVehicleManager.h"
#include "OfflineParameterLoader.h"
#include "ParameterCache.h"
#include "ParameterCacheDiagnostics.h"
#include "ParameterFileTextIO.h"
#include "ParameterMavlinkCodec.h"
#include "ParameterNameRemapper.h"
#include "ParameterPackFile.h"
#include "ParameterRequestReadStateMachine.h"
#include "ParameterSetStateMachine.h"
#include "QGCApplication.h"
#include "QGCLoggingCategory.h"
#include "QGCStateMachine.h"
#include "QmlObjectListModel.h"
#include "Vehicle.h"
#include "VehicleLinkManager.h"

QGC_LOGGING_CATEGORY(ParameterManagerLog, "Vehicle.Parameters.ParameterManager")
QGC_LOGGING_CATEGORY(ParameterManagerVerbose1Log, "Vehicle.Parameters.ParameterManager:verbose1")

ParameterManager::ParameterManager(Vehicle* vehicle)
    : QObject(vehicle),
      _vehicle(vehicle),
      _logReplay(!vehicle->vehicleLinkManager()->primaryLink().expired() &&
                 vehicle->vehicleLinkManager()->primaryLink().lock()->isLogReplay()),
      _cacheDiag(std::make_unique<ParameterCacheDiagnostics>())
{
    qCDebug(ParameterManagerLog) << this;

    if (_vehicle->isOfflineEditingVehicle()) {
        OfflineParameterLoader::load(this);
        return;
    }

    if (_logReplay) {
        qCDebug(ParameterManagerLog) << this << "In log replay mode";
    }

    _hashCheck = new HashCheckController(_vehicle, this);
    (void)connect(_hashCheck, &HashCheckController::cacheOnlyTimedOut, this, &ParameterManager::cacheCheckOnlyFailed);

    _downloader = new InitialParameterDownloader(this);
    (void)connect(_hashCheck, &HashCheckController::fullDownloadTimedOut, this,
                  [this]() { _downloader->start(MAV_COMP_ID_ALL); });

    // Surface silent writes to the missing-parameter sentinel returned by requireParameter().
    // First write per session warns; subsequent writes log at debug level to avoid log flooding from
    // QML bindings that re-evaluate on every tick.
    (void)connect(&_defaultFact, &Fact::rawValueChanged, this, [this](const QVariant& value) {
        if (!_defaultFactWriteWarned) {
            _defaultFactWriteWarned = true;
            qCWarning(ParameterManagerLog) << "Write to missing-parameter sentinel ignored; value:" << value;
        } else {
            qCDebug(ParameterManagerLog) << "Write to missing-parameter sentinel ignored; value:" << value;
        }
    });

    ParameterCache::ensureCacheDirExists();
    // Prune stale/excess cache files off-thread; cheap O(N) walk of the cache dir.
    // Skip during unit tests so parallel tests' cache files don't get deleted underneath them.
    if (!QGC::runningUnitTests()) {
        (void)QtConcurrent::run([] { ParameterCache::prune(); });
    }
}

ParameterManager::~ParameterManager()
{
    qCDebug(ParameterManagerLog) << this;
}

void ParameterManager::resetCacheDiagnostics()
{
    _cacheDiag->clear();
}

void ParameterManager::mavlinkMessageReceived(const mavlink_message_t& message)
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (Q_UNLIKELY(QThread::currentThread() != thread())) {
        qCCritical(ParameterManagerLog) << "mavlinkMessageReceived called from wrong thread";
        return;
    }

    if (_downloader->tryFtp() && (message.compid == MAV_COMP_ID_AUTOPILOT1) && !_downloader->initialLoadComplete()) {
        return;
    }

    if (message.msgid == MAVLINK_MSG_ID_PARAM_VALUE) {
        mavlink_param_value_t param_value{};
        mavlink_msg_param_value_decode(&message, &param_value);

        // This will null terminate the name string
        char parameterNameWithNull[MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN + 1] = {};
        (void)strncpy(parameterNameWithNull, param_value.param_id, MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN);
        const QString parameterName(parameterNameWithNull);

        // Reject empty / non-printable names. A buggy or malicious peer could otherwise create
        // un-addressable Facts that QML can't reference.
        if (parameterName.isEmpty() ||
            std::any_of(parameterName.cbegin(), parameterName.cend(),
                        [](QChar c) { return !c.isPrint() || c == QLatin1Char(' '); })) {
            qCWarning(ParameterManagerLog) << "Rejecting PARAM_VALUE with invalid name from compid" << message.compid;
            return;
        }

        mavlink_param_union_t paramUnion{};
        paramUnion.param_float = param_value.param_value;
        paramUnion.type = param_value.param_type;

        const auto parameterValue = ParameterMavlinkCodec::unionToVariant(paramUnion);
        if (!parameterValue) {
            return;
        }

        _handleParamValue(message.compid, parameterName, param_value.param_count, param_value.param_index,
                          static_cast<MAV_PARAM_TYPE>(param_value.param_type), *parameterValue);
    }
}

void ParameterManager::_handleParamValue(int componentId, const QString& parameterName, int parameterCount,
                                         int parameterIndex, MAV_PARAM_TYPE mavParamType,
                                         const QVariant& parameterValue)
{
    qCDebug(ParameterManagerVerbose1Log) << _logVehiclePrefix(componentId) << "_parameterUpdate"
                                         << "name:" << parameterName << "count:" << parameterCount
                                         << "index:" << parameterIndex << "mavType:" << mavParamType
                                         << "value:" << parameterValue << ")";

    // ArduPilot has this strange behavior of streaming parameters that we didn't ask for. This even happens before it
    // responds to the PARAM_REQUEST_LIST. We disregard any of this until the initial request is responded to.
    if ((parameterIndex == kUnsolicitedParamIndex) && (parameterName != QStringLiteral("_HASH_CHECK")) &&
        _downloader->waitingForListResponse()) {
        qCDebug(ParameterManagerLog) << "Disregarding unrequested param prior to initial list response"
                                     << parameterName;
        return;
    }

    if (_vehicle->px4Firmware() && (parameterName == "_HASH_CHECK")) {
        _hashCheck->noteResponseReceived();
        if (!_downloader->initialLoadComplete() && !_logReplay) {
            _tryCacheHashLoad(_vehicle->id(), componentId, parameterValue);
        }
        return;
    }

    if (!_downloader->initialLoadComplete() && !_logReplay) {
        _cacheDiag->noteParamValue(componentId, parameterName, parameterValue);
    }

    // Pre-size the per-component fact map on first sight so the inner QHash
    // does not rehash 5-10× during a 2000-param initial load.
    if (parameterCount > 0 && !_mapCompId2FactMap.contains(componentId)) {
        _mapCompId2FactMap[componentId].reserve(parameterCount);
    }

    const int prevWaitingCount = _downloader->waitingReadParamIndexCount();
    const int currWaitingCount =
        _downloader->noteParamReceived(componentId, parameterName, parameterCount, parameterIndex);

    bool isNew = false;
    Fact* const fact =
        ensureFact(componentId, parameterName, ParameterMavlinkCodec::mavTypeToFactType(mavParamType), isNew);
    if (isNew) {
        emit factAdded(componentId, fact);
    }

    fact->containerSetRawValue(parameterValue);

    // PX4 only: write cache when all index reads just completed (transition nonzero→zero).
    if (!_logReplay && _vehicle->px4Firmware() && prevWaitingCount != 0 && currWaitingCount == 0) {
        ParameterCache::write(_vehicle->id(), componentId, _mapCompId2FactMap[componentId]);
    }

    _downloader->checkInitialLoadComplete();

    qCDebug(ParameterManagerVerbose1Log) << _logVehiclePrefix(componentId) << "_parameterUpdate complete";
}

QString ParameterManager::vehicleAndComponentString(int componentId) const
{
    QStringList parts;
    if (MultiVehicleManager::instance()->vehicles()->count() > 1) {
        parts << QStringLiteral("veh: %1").arg(_vehicle->id());
    }
    if (_mapCompId2FactMap.size() > 1) {
        parts << QStringLiteral("comp: %1").arg(componentId);
    }
    return parts.join(QLatin1Char(' '));
}

void ParameterManager::mavlinkParamSet(int componentId, const QString& paramName, FactMetaData::ValueType_t valueType,
                                        const QVariant& rawValue)
{
    Q_ASSERT(QThread::currentThread() == thread());

    const QPair<int, QString> key{componentId, paramName};
    if (_inFlightSets.contains(key)) {
        // A write is already pending; just remember the latest target. The completion handler
        // below will dispatch one final write if the latest stash differs from the acked value.
        _pendingLatestSets.insert(key, PendingSet{valueType, rawValue});
        return;
    }
    _inFlightSets.insert(key);

    auto* sm = new ParameterSetStateMachine(this, componentId, paramName, valueType, rawValue, this);
    (void)connect(sm, &ParameterSetStateMachine::setSucceeded, this, &ParameterManager::_paramSetSuccess);
    (void)connect(sm, &ParameterSetStateMachine::setFailed, this, &ParameterManager::_paramSetFailure);
    // Drop the on-disk cache the moment a write succeeds. Otherwise reconnect → CRC matches the
    // pre-write file → user sees the stale value flash before the vehicle re-streams.
    (void)connect(sm, &ParameterSetStateMachine::setSucceeded, this, [this](int compId, const QString&) {
        ParameterCache::invalidate(_vehicle->id(), compId);
    });

    // After succeed/fail, mark this (comp,name) free and dispatch any stashed follow-up.
    auto onTerminal = [this, key, rawValue]() {
        _inFlightSets.remove(key);
        const auto pendingIt = _pendingLatestSets.constFind(key);
        if (pendingIt == _pendingLatestSets.cend()) {
            return;
        }
        const PendingSet pending = pendingIt.value();
        _pendingLatestSets.erase(pendingIt);
        // If the stash matches the value the vehicle already acked, no work to do.
        if (pending.rawValue == rawValue) {
            return;
        }
        mavlinkParamSet(key.first, key.second, pending.valueType, pending.rawValue);
    };
    (void)connect(sm, &ParameterSetStateMachine::setSucceeded, this, onTerminal);
    (void)connect(sm, &ParameterSetStateMachine::setFailed, this, onTerminal);

    sm->start();
}

void ParameterManager::_factRawValueUpdated(const QVariant& rawValue)
{
    Fact* const fact = qobject_cast<Fact*>(sender());
    if (!fact) {
        qCWarning(ParameterManagerLog) << "Internal error";
        return;
    }

    mavlinkParamSet(fact->componentId(), fact->name(), fact->type(), rawValue);
}

void ParameterManager::refreshAllParameters(uint8_t componentId)
{
    Q_ASSERT(QThread::currentThread() == thread());
    _hashCheck->reset();
    setParameterDownloadSkipped(false);
    _downloader->start(componentId);
}

void ParameterManager::tryHashCheckCacheLoad()
{
    Q_ASSERT(QThread::currentThread() == thread());
    _hashCheck->reset();

    const SharedLinkInterfacePtr sharedLink = _vehicle->vehicleLinkManager()->primaryLink().lock();
    if (!sharedLink) {
        emit cacheCheckOnlyFailed();
        return;
    }

    if (sharedLink->linkConfiguration()->isHighLatency() || _logReplay) {
        qCDebug(ParameterManagerLog) << _logVehiclePrefix(-1)
                                     << "Cache-only hash check: high latency or log replay link, signalling failure";
        emit cacheCheckOnlyFailed();
        return;
    }

    if (_vehicle->px4Firmware() && !_downloader->initialLoadComplete()) {
        qCDebug(ParameterManagerLog) << _logVehiclePrefix(-1) << "Cache-only hash check: requesting _HASH_CHECK";
        _hashCheck->armForCacheOnly(MAV_COMP_ID_AUTOPILOT1);
    } else {
        qCDebug(ParameterManagerLog) << _logVehiclePrefix(-1)
                                     << "Cache-only hash check: not available, signalling failure";
        emit cacheCheckOnlyFailed();
    }
}

int ParameterManager::_actualComponentId(int componentId) const
{
    if (componentId == anyComponentId) {
        componentId = _vehicle->primaryComponentId();
        if (componentId == anyComponentId) {
            qCWarning(ParameterManagerLog) << _logVehiclePrefix(-1) << "Default component id not set";
        }
    }

    return componentId;
}

void ParameterManager::refreshParameter(int componentId, const QString& paramName)
{
    componentId = _actualComponentId(componentId);

    qCDebug(ParameterManagerLog) << _logVehiclePrefix(componentId) << "refreshParameter - name:" << paramName << ")";

    mavlinkParamRequestRead(componentId, paramName, -1, true /* notifyFailure */);
}

void ParameterManager::_refreshParameterQuiet(int componentId, const QString& paramName)
{
    mavlinkParamRequestRead(componentId, paramName, -1, false /* notifyFailure */);
}

void ParameterManager::refreshParametersPrefix(int componentId, const QString& namePrefix)
{
    componentId = _actualComponentId(componentId);
    qCDebug(ParameterManagerLog) << _logVehiclePrefix(componentId) << "refreshParametersPrefix - name:" << namePrefix
                                 << ")";

    // constFind avoids the non-const operator[] silently inserting an empty inner hash on miss.
    const auto it = _mapCompId2FactMap.constFind(componentId);
    if (it == _mapCompId2FactMap.cend()) {
        return;
    }
    // Sort once and use lower_bound so 1 prefix scan touches O(log N + k) names instead of O(N).
    // Setup pages refresh several prefixes back-to-back, so this matters even at 2000-param fleets.
    QStringList names = it.value().keys();
    std::sort(names.begin(), names.end());
    auto first = std::lower_bound(names.cbegin(), names.cend(), namePrefix);
    for (auto cur = first; cur != names.cend() && cur->startsWith(namePrefix); ++cur) {
        _refreshParameterQuiet(componentId, *cur);
    }
}

bool ParameterManager::parameterExists(int componentId, const QString& paramName) const
{
    bool ret = false;

    componentId = _actualComponentId(componentId);
    if (_mapCompId2FactMap.contains(componentId)) {
        ret = _mapCompId2FactMap[componentId].contains(ParameterNameRemapper::remap(_vehicle, paramName));
    }

    return ret;
}

Fact* ParameterManager::_lookup(int componentId, const QString& mappedParamName)
{
    const auto compIt = _mapCompId2FactMap.constFind(componentId);
    if (compIt == _mapCompId2FactMap.cend()) {
        return nullptr;
    }
    const auto factIt = compIt.value().constFind(mappedParamName);
    if (factIt == compIt.value().cend()) {
        return nullptr;
    }
    return factIt.value();
}

Fact* ParameterManager::getParameter(int componentId, const QString& paramName)
{
    return _lookup(_actualComponentId(componentId), ParameterNameRemapper::remap(_vehicle, paramName));
}

Fact& ParameterManager::requireParameter(int componentId, const QString& paramName)
{
    const int actualCompId = _actualComponentId(componentId);
    const QString mappedParamName = ParameterNameRemapper::remap(_vehicle, paramName);
    Fact* const fact = _lookup(actualCompId, mappedParamName);
    if (Q_UNLIKELY(!fact)) {
        const QPair<int, QString> key{actualCompId, mappedParamName};
        if (!_reportedMissingParams.contains(key)) {
            _reportedMissingParams.insert(key);
            qgcApp()->reportMissingParameter(actualCompId, mappedParamName);
        }
        return _defaultFact;
    }
    return *fact;
}

QStringList ParameterManager::parameterNames(int componentId) const
{
    QStringList names;

    const int compId = _actualComponentId(componentId);
    const auto& factMap = _mapCompId2FactMap[compId];
    names.reserve(factMap.size());
    for (auto it = factMap.cbegin(); it != factMap.cend(); ++it) {
        names << it.key();
    }

    return names;
}

void ParameterManager::mavlinkParamRequestRead(int componentId, const QString& paramName, int paramIndex,
                                                bool notifyFailure)
{
    auto* sm = new ParameterRequestReadStateMachine(this, componentId, paramName, paramIndex, notifyFailure, this);
    (void)connect(sm, &ParameterRequestReadStateMachine::readSucceeded, this,
                  &ParameterManager::_paramRequestReadSuccess);
    (void)connect(sm, &ParameterRequestReadStateMachine::readFailed, this, &ParameterManager::_paramRequestReadFailure);
    sm->start();
}

Fact* ParameterManager::ensureFact(int componentId, const QString& parameterName, FactMetaData::ValueType_t type,
                                    bool& isNew)
{
    auto& factMap = _mapCompId2FactMap[componentId];
    const auto it = factMap.find(parameterName);
    if (it != factMap.end()) {
        // Firmware reboot or upgrade can change a parameter's type. We keep the existing Fact (and
        // its type) — but a mismatch means containerSetRawValue will decode wrong. Surface it.
        if (Q_UNLIKELY(it.value()->type() != type)) {
            qCWarning(ParameterManagerLog) << _logVehiclePrefix(componentId) << "Type mismatch on" << parameterName
                                           << "existing:" << it.value()->type() << "incoming:" << type;
        }
        isNew = false;
        return it.value();
    }

    qCDebug(ParameterManagerVerbose1Log) << _logVehiclePrefix(componentId) << "Adding new fact" << parameterName;

    Fact* const fact = new Fact(componentId, parameterName, type, this);
    FactMetaData* const factMetaData =
        _vehicle->compInfoManager()->compInfoParam(componentId)->factMetaDataForName(parameterName, fact->type());
    fact->setMetaData(factMetaData);
    factMap.insert(parameterName, fact);
    (void)connect(fact, &Fact::containerRawValueChanged, this, &ParameterManager::_factRawValueUpdated);
    isNew = true;
    return fact;
}

bool ParameterManager::registerFact(int componentId, const QString& parameterName, Fact* fact)
{
    auto& factMap = _mapCompId2FactMap[componentId];
    if (factMap.contains(parameterName)) {
        return false;
    }
    factMap.insert(parameterName, fact);
    return true;
}

void ParameterManager::_animateCacheLoadProgress()
{
    auto* const ani = new QVariantAnimation(this);
    ani->setEasingCurve(QEasingCurve::OutCubic);
    ani->setStartValue(0.0);
    ani->setEndValue(1.0);
    ani->setDuration(750);
    (void)connect(ani, &QVariantAnimation::valueChanged, this,
                  [this](const QVariant& value) { setLoadProgress(value.toDouble()); });
    (void)connect(ani, &QVariantAnimation::finished, this,
                  [this] { QTimer::singleShot(500, this, [this] { setLoadProgress(0); }); });
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void ParameterManager::_tryCacheHashLoad(int vehicleId, int componentId, const QVariant& hashValue)
{
    qCDebug(ParameterManagerLog) << "Attempting load from cache";

    auto isVolatile = [this](const QString& name, FactMetaData::ValueType_t type) -> bool {
        const bool vol = _vehicle->compInfoManager()
                             ->compInfoParam(MAV_COMP_ID_AUTOPILOT1)
                             ->factMetaDataForName(name, type)
                             ->volatileValue();
        if (vol) {
            qCDebug(ParameterManagerLog) << "Volatile parameter" << name;
        }
        return vol;
    };

    const auto result = ParameterCache::load(vehicleId, componentId, hashValue.toUInt(), isVolatile);

    // Handle non-success statuses (FileNotFound, OpenFailed, CrcMismatch fallback)
    if (result.status == ParameterCache::LoadStatus::FileNotFound ||
        result.status == ParameterCache::LoadStatus::OpenFailed) {
        if (result.status == ParameterCache::LoadStatus::FileNotFound) {
            qCDebug(ParameterManagerLog) << "No parameter cache file";
        }
        if (!_hashCheck->isDone()) {
            _hashCheck->markDone();
            if (_hashCheck->isCacheOnly()) {
                qCDebug(ParameterManagerLog) << "Cache-only hash check: no cache, signalling failure";
                emit cacheCheckOnlyFailed();
                return;
            }
            _downloader->start(MAV_COMP_ID_ALL);
        }
        return;
    }

    if (result.status == ParameterCache::LoadStatus::CrcMatch) {
        _hashCheck->markDone();
        _downloader->stopParamRequestListTimer();
        qCDebug(ParameterManagerLog) << "Parameters loaded from cache"
                                     << ParameterCache::cacheFile(vehicleId, componentId);

        const int count = static_cast<int>(result.params.count());
        int index = 0;
        for (const auto& [name, ptv] : std::as_const(result.params).asKeyValueRange()) {
            const MAV_PARAM_TYPE mavParamType = ParameterMavlinkCodec::factTypeToMavType(ptv.type);
            _handleParamValue(componentId, name, count, index++, mavParamType, ptv.value);
        }

        // Notify vehicle we loaded from cache — send back the hash so it stops streaming
        _hashCheck->sendHashAck(componentId, result.localCrc);

        _animateCacheLoadProgress();
    } else {
        // CrcMismatch
        qCDebug(ParameterManagerLog) << "Parameters cache CRC mismatch"
                                     << ParameterCache::cacheFile(vehicleId, componentId);
        if (ParameterCacheDiagnostics::isEnabled()) {
            _cacheDiag->noteCrcMismatch(componentId, result.params);
        }
        if (!_hashCheck->isDone()) {
            _hashCheck->markDone();
            if (_hashCheck->isCacheOnly()) {
                qCDebug(ParameterManagerLog) << "Cache-only hash check: CRC mismatch, signalling failure";
                emit cacheCheckOnlyFailed();
                return;
            }
            _downloader->start(MAV_COMP_ID_ALL);
        }
    }
}

QString ParameterManager::readParametersFromStream(QTextStream& stream)
{
    return ParameterFileTextIO::read(stream, this);
}

void ParameterManager::writeParametersToStream(QTextStream& stream)
{
    ParameterFileTextIO::write(stream, this);
}

void ParameterManager::resetAllParametersToDefaults()
{
    _vehicle->sendMavCommand(MAV_COMP_ID_AUTOPILOT1, MAV_CMD_PREFLIGHT_STORAGE,
                             true,  // showError
                             2,     // Reset params to default
                             -1);   // Don't do anything with mission storage
}

void ParameterManager::resetAllToVehicleConfiguration()
{
    //-- https://github.com/PX4/Firmware/pull/11760
    Fact* const sysAutoConfigFact = getParameter(anyComponentId, "SYS_AUTOCONFIG");
    if (sysAutoConfigFact) {
        sysAutoConfigFact->setRawValue(2);
    }
}

QString ParameterManager::_logVehiclePrefix(int componentId) const
{
    return _vehicle->logPrefix(componentId);
}

void ParameterManager::setLoadProgress(double loadProgress)
{
    // Throttle to whole-percent boundaries (and always emit terminal 0.0/1.0 so QML resets cleanly).
    // 2000-param initial load fires ~2000 times; whole-percent rounding cuts that to ~100.
    const bool terminal = (loadProgress == 0.0) || (loadProgress == 1.0);
    if (!terminal && qFloor(_loadProgress * 100.0) == qFloor(loadProgress * 100.0)) {
        _loadProgress = loadProgress;
        return;
    }
    if (_loadProgress != loadProgress) {
        _loadProgress = loadProgress;
        emit loadProgressChanged(static_cast<float>(loadProgress));
    }
}

void ParameterManager::setParametersReady(bool ready)
{
    if (_parametersReady != ready) {
        _parametersReady = ready;
        emit parametersReadyChanged(ready);
    }
}

void ParameterManager::finalizeInitialLoad(bool anyMissing, const std::function<void()>& preEmitHook)
{
    _parametersReady = true;
    _missingParameters = anyMissing;
    if (preEmitHook) {
        preEmitHook();
    }
    // Listeners on missingParametersChanged frequently check parametersReady — emit ready first.
    emit parametersReadyChanged(true);
    emit missingParametersChanged(_missingParameters);
}

void ParameterManager::setParameterDownloadSkipped(bool skipped)
{
    if (_parameterDownloadSkipped != skipped) {
        _parameterDownloadSkipped = skipped;
        emit parameterDownloadSkippedChanged();
    }
}

QList<int> ParameterManager::componentIds() const
{
    return _downloader ? _downloader->componentIds() : QList<int>{};
}

bool ParameterManager::pendingWrites() const
{
    return _pendingWritesCount > 0;
}

Vehicle* ParameterManager::vehicle()
{
    return _vehicle;
}

bool ParameterManager::parseParamPackFile(const QString& filename)
{
    constexpr int componentId = MAV_COMP_ID_AUTOPILOT1;

    const ParameterPackFile::ParseResult result = ParameterPackFile::parse(filename);
    if (!result.ok) {
        return false;
    }

    for (const ParameterPackFile::ParsedParam& p : result.params) {
        bool isNew = false;
        Fact* const fact = ensureFact(componentId, p.name, p.type, isNew);
        // Set default before emitting factAdded so QML sees defaultValueAvailable from the start on new facts.
        if (p.defaultValue.isValid()) {
            fact->metaData()->setRawDefaultValue(p.defaultValue);
        }
        if (isNew) {
            emit factAdded(componentId, fact);
        }
        fact->containerSetRawValue(p.value);
    }

    const int paramCount = static_cast<int>(result.params.size());
    _downloader->markComponentFullyDownloaded(componentId, paramCount);
    setLoadProgress(0.0);
    return true;
}

void ParameterManager::noteWriteStarted()
{
    _pendingWritesCount++;
    emit pendingWritesCountChanged(_pendingWritesCount);
    if (_pendingWritesCount == 1) {
        emit pendingWritesChanged(true);
    }
}

void ParameterManager::noteWriteFinished()
{
    if (_pendingWritesCount == 0) {
        qCWarning(ParameterManagerLog) << "Internal Error: _pendingWriteCount == 0";
        return;
    }

    _pendingWritesCount--;
    emit pendingWritesCountChanged(_pendingWritesCount);
    if (_pendingWritesCount == 0) {
        emit pendingWritesChanged(false);
    }
}
