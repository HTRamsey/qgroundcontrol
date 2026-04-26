#include "HashCheckController.h"

#include <cstring>

#include "HashCheckStateMachine.h"
#include "MAVLinkProtocol.h"
#include "QGCLoggingCategory.h"
#include "Vehicle.h"
#include "VehicleLinkManager.h"

QGC_LOGGING_CATEGORY(HashCheckControllerLog, "Vehicle.Parameters.HashCheckController")

HashCheckController::HashCheckController(Vehicle* vehicle, QObject* parent) : QObject(parent), _vehicle(vehicle)
{
}

void HashCheckController::reset()
{
    if (_activeSm) {
        _activeSm->stop();
        _activeSm->deleteLater();
        _activeSm.clear();
    }
    _hashCheckDone = false;
}

void HashCheckController::armForFullDownload(uint8_t componentId)
{
    _arm(componentId, /*cacheOnly=*/false);
}

void HashCheckController::armForCacheOnly(uint8_t componentId)
{
    _arm(componentId, /*cacheOnly=*/true);
}

void HashCheckController::_arm(uint8_t componentId, bool cacheOnly)
{
    _cacheOnlyHashCheck = cacheOnly;

    if (_activeSm) {
        _activeSm->stop();
        _activeSm->deleteLater();
    }

    _activeSm = new HashCheckStateMachine(this, _vehicle, componentId, this);
    connect(_activeSm.data(), &HashCheckStateMachine::timedOut, this, &HashCheckController::_onSmTimedOut);
    _activeSm->start();
}

void HashCheckController::noteResponseReceived()
{
    emit responseReceived();
}

void HashCheckController::markDone()
{
    _hashCheckDone = true;
}

void HashCheckController::_onSmTimedOut()
{
    _hashCheckDone = true;
    if (_cacheOnlyHashCheck) {
        emit cacheOnlyTimedOut();
    } else {
        emit fullDownloadTimedOut();
    }
}

void HashCheckController::sendHashAck(int componentId, uint32_t localCrc)
{
    const SharedLinkInterfacePtr sharedLink = _vehicle->vehicleLinkManager()->primaryLink().lock();
    if (!sharedLink) {
        return;
    }

    mavlink_param_set_t p{};
    mavlink_param_union_t union_value{};

    p.param_type = MAV_PARAM_TYPE_UINT32;
    std::strncpy(p.param_id, "_HASH_CHECK", sizeof(p.param_id));
    union_value.param_uint32 = localCrc;
    p.param_value = union_value.param_float;
    p.target_system = static_cast<uint8_t>(_vehicle->id());
    p.target_component = static_cast<uint8_t>(componentId);

    mavlink_message_t msg{};
    (void)mavlink_msg_param_set_encode_chan(MAVLinkProtocol::instance()->getSystemId(),
                                            MAVLinkProtocol::getComponentId(), sharedLink->mavlinkChannel(), &msg, &p);
    (void)_vehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), msg);
}
