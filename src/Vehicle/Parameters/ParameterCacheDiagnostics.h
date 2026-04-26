#pragma once

#include <QtCore/QHash>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include "ParameterCache.h"

Q_DECLARE_LOGGING_CATEGORY(ParameterCacheDiagnosticsLog)

/// Per-parameter diagnostic logging used when a cache CRC mismatch is detected and the
/// `Vehicle.Parameters.ParameterCacheDiagnostics` debug log category is enabled.
///
/// Tracks the cache contents at mismatch time and records, on each subsequent PARAM_VALUE
/// arrival, whether the live value differs from the cached one. Cache entries never seen on
/// the wire ("unseen") are logged via @ref logUnseen at end-of-load.
class ParameterCacheDiagnostics
{
public:
    /// True if the diagnostics log category is enabled — gate the bookkeeping on this so the
    /// main param-value path pays nothing when diagnostics are off.
    [[nodiscard]] static bool isEnabled();

    /// Snapshot the cache contents for @p componentId so subsequent PARAM_VALUE arrivals can
    /// be compared. Pops a user-facing notice that the CRC failed.
    void noteCrcMismatch(int componentId, const ParameterCache::CacheMap& cacheParams);

    /// Compare an incoming PARAM_VALUE against the cached snapshot; logs differences.
    /// No-op if the component is not under diagnostic tracking.
    void noteParamValue(int componentId, const QString& parameterName, const QVariant& vehicleValue);

    /// Drop diagnostic state — called when load completes or the offline cache is wiped.
    void clear();

    /// Log every cache parameter that did not arrive on the wire.
    void logUnseen() const;

private:
    QSet<int> _crcMismatchCompIds;
    QHash<int, ParameterCache::CacheMap> _cacheParamsByComp;
    QHash<int, QSet<QString>> _unseenByComp;
};
