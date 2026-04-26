#include "ParameterNameRemapper.h"

#include "FirmwarePlugin.h"
#include "QGCLoggingCategory.h"
#include "Vehicle.h"

QGC_LOGGING_CATEGORY(ParameterNameRemapperLog, "Vehicle.Parameters.ParameterNameRemapper")

namespace ParameterNameRemapper {

QString remap(Vehicle* vehicle, const QString& paramName)
{
    static const QString noRemapPrefix = QStringLiteral("noremap.");
    if (paramName.startsWith(noRemapPrefix)) {
        return paramName.mid(noRemapPrefix.length());
    }

    const int majorVersion = vehicle->firmwareMajorVersion();
    const int minorVersion = vehicle->firmwareMinorVersion();

    qCDebug(ParameterNameRemapperLog) << "remap" << paramName << majorVersion << minorVersion;

    if (majorVersion == Vehicle::versionNotSetValue) {
        return paramName;
    }

    const FirmwarePlugin::remapParamNameMajorVersionMap_t& majorVersionRemap =
        vehicle->firmwarePlugin()->paramNameRemapMajorVersionMap();
    if (!majorVersionRemap.contains(majorVersion)) {
        qCDebug(ParameterNameRemapperLog) << "remap: no major version mapping";
        return paramName;
    }

    const FirmwarePlugin::remapParamNameMinorVersionRemapMap_t& remapMinorVersion = majorVersionRemap[majorVersion];
    QString mappedParamName = paramName;
    for (int currentMinorVersion = vehicle->firmwarePlugin()->remapParamNameHigestMinorVersionNumber(majorVersion);
         currentMinorVersion > minorVersion; currentMinorVersion--) {
        if (remapMinorVersion.contains(currentMinorVersion)) {
            const FirmwarePlugin::remapParamNameMap_t& remap = remapMinorVersion[currentMinorVersion];
            if (remap.contains(mappedParamName)) {
                const QString toParamName = remap[mappedParamName];
                qCDebug(ParameterNameRemapperLog)
                    << "remap: remapped currentMinor:from:to" << currentMinorVersion << mappedParamName << toParamName;
                mappedParamName = toParamName;
            }
        }
    }

    return mappedParamName;
}

}  // namespace ParameterNameRemapper
