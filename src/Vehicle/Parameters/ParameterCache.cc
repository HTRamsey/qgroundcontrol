#include "ParameterCache.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include <algorithm>

#include "AppMessages.h"
#include "Fact.h"
#include "QGCFileHelper.h"
#include "QGCLoggingCategory.h"
#include "QGCMath.h"

QGC_LOGGING_CATEGORY(ParameterCacheLog, "Vehicle.Parameters.ParameterCache")

QDir ParameterCache::cacheDir()
{
    // Use application-specific subdirectory to isolate parallel test runs
    const QFileInfo settingsFile(QSettings().fileName());
    const QString basePath = settingsFile.dir().absolutePath();
    const QString appName = settingsFile.completeBaseName();
    return QDir(basePath + QDir::separator() + appName + QDir::separator() + QStringLiteral("ParamCache"));
}

QString ParameterCache::cacheFile(int vehicleId, int componentId)
{
    // Limitation: keyed only by (sysid, compid) — two simultaneous vehicles with the same SYSID
    // (e.g. parallel SITL instances on one host) will overwrite each other's caches.
    return cacheDir().filePath(QStringLiteral("%1_%2.v2").arg(vehicleId).arg(componentId));
}

void ParameterCache::ensureCacheDirExists()
{
    (void)QGCFileHelper::ensureDirectoryExists(cacheDir().absolutePath());
}

void ParameterCache::invalidate(int vehicleId, int componentId)
{
    const QString path = cacheFile(vehicleId, componentId);
    if (QFile::exists(path) && !QFile::remove(path)) {
        qCWarning(ParameterCacheLog) << "Failed to invalidate cache file" << path;
    }
}

void ParameterCache::clearAll()
{
    QDir dir = cacheDir();
    dir.removeRecursively();
    (void)QGCFileHelper::ensureDirectoryExists(dir.absolutePath());
}

void ParameterCache::prune()
{
    const QDir dir = cacheDir();
    if (!dir.exists()) {
        return;
    }

    QFileInfoList entries = dir.entryInfoList(QStringList{QStringLiteral("*.v2")}, QDir::Files, QDir::Time);
    if (entries.isEmpty()) {
        return;
    }

    const QDateTime ageCutoff = QDateTime::currentDateTime().addDays(-kCacheMaxAgeDays);

    int removed = 0;
    QFileInfoList survivors;
    survivors.reserve(entries.size());
    for (const QFileInfo& info : entries) {
        if (info.lastModified() < ageCutoff) {
            if (QFile::remove(info.absoluteFilePath())) {
                ++removed;
            }
        } else {
            survivors.append(info);
        }
    }

    // Survivors are in QDir::Time order (newest first); drop the tail beyond kCacheMaxFiles.
    for (int i = kCacheMaxFiles; i < survivors.size(); ++i) {
        if (QFile::remove(survivors.at(i).absoluteFilePath())) {
            ++removed;
        }
    }

    if (removed > 0) {
        qCDebug(ParameterCacheLog) << "Pruned" << removed << "cache files; survivors:"
                                   << std::min<qsizetype>(survivors.size(), kCacheMaxFiles);
    }
}

void ParameterCache::write(int vehicleId, int componentId, const QHash<QString, Fact*>& facts)
{
    CacheMap cacheMap;
    for (const auto& [paramName, fact] : facts.asKeyValueRange()) {
        cacheMap[paramName] = ParamTypeVal{fact->type(), fact->rawValue()};
    }

    QByteArray buffer;
    QDataStream ds(&buffer, QIODevice::WriteOnly);
    // Stamp the layout version so a future field addition can be detected and old caches
    // discarded gracefully instead of misparsed.
    ds << kCacheStreamVersion;
    ds << cacheMap;
    if (ds.status() != QDataStream::Ok) {
        qCWarning(ParameterCacheLog) << "Failed to serialize cache map; status:" << ds.status();
        return;
    }

    const QString path = cacheFile(vehicleId, componentId);

    auto writeFn = [path, buffer = std::move(buffer)]() {
        if (!QGCFileHelper::atomicWrite(path, buffer)) {
            qCWarning(ParameterCacheLog) << "Failed to write cache file" << path;
        }
    };

    // Disk I/O off-thread to avoid stalling the GUI on slow storage (SD card / Android external).
    // Sync in unit tests so tests can observe the cache file without a separate wait.
    if (QGC::runningUnitTests()) {
        writeFn();
    } else {
        (void)QtConcurrent::run(std::move(writeFn));
    }
}

ParameterCache::LoadResult ParameterCache::load(int vehicleId, int componentId, uint32_t remoteCrc,
                                                const VolatileCheck& isVolatile)
{
    LoadResult result;

    QFile file(cacheFile(vehicleId, componentId));
    if (!file.exists()) {
        result.status = LoadStatus::FileNotFound;
        return result;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(ParameterCacheLog) << "Failed to open cache file for reading" << file.fileName();
        result.status = LoadStatus::OpenFailed;
        return result;
    }

    QDataStream ds(&file);
    uint32_t streamVersion = 0;
    ds >> streamVersion;
    if (ds.status() != QDataStream::Ok || streamVersion != kCacheStreamVersion) {
        qCWarning(ParameterCacheLog) << "Cache version mismatch (got" << streamVersion << "want"
                                     << kCacheStreamVersion << ") - discarding";
        result.status = LoadStatus::CrcMismatch;
        return result;
    }

    ds >> result.params;
    if (ds.status() != QDataStream::Ok) {
        qCWarning(ParameterCacheLog) << "Cache file truncated or corrupt:" << file.fileName()
                                     << "status=" << ds.status();
        result.status = LoadStatus::CrcMismatch;
        result.params.clear();
        return result;
    }

    uint32_t crc = 0;
    for (const auto& [name, ptv] : std::as_const(result.params).asKeyValueRange()) {
        if (isVolatile(name, ptv.type)) {
            continue;
        }
        // UTF-8 keeps CRC stable across user locales; qPrintable() goes through QLocal8Bit
        // which is locale-dependent on Windows.
        const QByteArray nameUtf8 = name.toUtf8();
        const void* const vdat = ptv.value.constData();
        crc = QGC::crc32(reinterpret_cast<const uint8_t*>(nameUtf8.constData()), nameUtf8.size(), crc);
        crc = QGC::crc32(static_cast<const uint8_t*>(vdat), FactMetaData::typeToSize(ptv.type), crc);
    }
    result.localCrc = crc;
    result.status = (crc == remoteCrc) ? LoadStatus::CrcMatch : LoadStatus::CrcMismatch;

    return result;
}
