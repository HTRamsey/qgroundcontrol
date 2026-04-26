#pragma once

#include <QtCore/QLoggingCategory>

class ParameterManager;

Q_DECLARE_LOGGING_CATEGORY(OfflineParameterLoaderLog)

/// Loads custom offline-editing parameters from the firmware-plugin-supplied
/// .params text file into the placeholder offline-editing vehicle. Used only
/// when @ref Vehicle::isOfflineEditingVehicle() is true.
class OfflineParameterLoader
{
public:
    /// Open the offline params file from the FirmwarePlugin and bolt the
    /// resulting Facts onto @p manager.
    static void load(ParameterManager* manager);
};
