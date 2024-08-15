#include "CustomFirmwarePlugin.h"
#include "CustomAutoPilotPlugin.h"
#include "Vehicle.h"

CustomFirmwarePlugin::CustomFirmwarePlugin()
{

}

//-----------------------------------------------------------------------------
AutoPilotPlugin* CustomFirmwarePlugin::autopilotPlugin(Vehicle* vehicle)
{
    return new CustomAutoPilotPlugin(vehicle, vehicle);
}
