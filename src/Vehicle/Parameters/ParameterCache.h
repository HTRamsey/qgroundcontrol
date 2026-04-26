#pragma once

#include <QtCore/QDataStream>
#include <QtCore/QHash>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include <cstdint>
#include <functional>

#include "FactMetaData.h"

class Fact;
class QDir;

Q_DECLARE_LOGGING_CATEGORY(ParameterCacheLog)

/// Handles reading, writing, and CRC validation of the on-disk parameter cache.
/// Pure I/O + CRC — no protocol or vehicle knowledge.
class ParameterCache
{
public:
    struct ParamTypeVal
    {
        FactMetaData::ValueType_t type = FactMetaData::valueTypeInt32;
        QVariant value;

        friend QDataStream& operator<<(QDataStream& out, const ParamTypeVal& ptv)
        {
            out << static_cast<int>(ptv.type) << ptv.value;
            return out;
        }

        friend QDataStream& operator>>(QDataStream& in, ParamTypeVal& ptv)
        {
            int t = 0;
            in >> t >> ptv.value;
            ptv.type = static_cast<FactMetaData::ValueType_t>(t);
            return in;
        }
    };

    // Sorted (QMap) to keep CRC deterministic across cache write/read and across processes.
    using CacheMap = QMap<QString /* parameter name */, ParamTypeVal>;

    /// Predicate used by @ref load to skip volatile params from the CRC. Type-erased so
    /// the implementation can live in the .cc; callers still pass a lambda as before.
    using VolatileCheck = std::function<bool(const QString& name, FactMetaData::ValueType_t type)>;

    enum class LoadStatus
    {
        CrcMatch,      ///< Cache loaded and CRC matches remote
        CrcMismatch,   ///< Cache loaded but CRC does not match
        FileNotFound,  ///< No cache file on disk
        OpenFailed,    ///< File exists but could not be opened
    };

    struct LoadResult
    {
        LoadStatus status = LoadStatus::FileNotFound;
        CacheMap params;
        uint32_t localCrc = 0;
    };

    /// @return Directory where parameter cache files are stored
    [[nodiscard]] static QDir cacheDir();

    /// @return Full path to the cache file for a given vehicle/component
    [[nodiscard]] static QString cacheFile(int vehicleId, int componentId);

    /// Delete the cache file for the given (vehicle, component). Used after a parameter write so
    /// the next session re-streams the canonical values rather than restoring the pre-write CRC.
    static void invalidate(int vehicleId, int componentId);

    /// Ensure the cache directory exists on disk
    static void ensureCacheDirExists();

    /// Wipe the cache directory and recreate it. Used by first-run / settings-version
    /// upgrade and by the @c --clear-cache CLI flag.
    static void clearAll();

    /// Delete cache files older than @ref kCacheMaxAgeDays and, if more than
    /// @ref kCacheMaxFiles remain, drop the oldest by mtime. Bounds disk use across many SYSIDs.
    /// Safe to call on a worker thread.
    static void prune();

    static constexpr int kCacheMaxAgeDays = 30;
    static constexpr int kCacheMaxFiles = 50;

    /// Layout version stamped into every cache file. Bumped any time the on-disk
    /// representation of @ref ParamTypeVal or @ref CacheMap changes.
    static constexpr uint32_t kCacheStreamVersion = 1;

    /// Write current parameter state to the cache file.
    /// @param facts  Map of parameter name → Fact* for the component being cached.
    static void write(int vehicleId, int componentId, const QHash<QString, Fact*>& facts);

    /// Load and validate the parameter cache against a remote CRC.
    /// @param isVolatile  Predicate returning true if the named param should be excluded from CRC.
    [[nodiscard]] static LoadResult load(int vehicleId, int componentId, uint32_t remoteCrc,
                                         const VolatileCheck& isVolatile);
};
