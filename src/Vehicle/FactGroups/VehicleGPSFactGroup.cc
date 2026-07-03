#include "VehicleGPSFactGroup.h"
#include "Vehicle.h"
#include "QGCGeo.h"
#include "QGCLoggingCategory.h"
#include "development/mavlink_msg_gnss_integrity.h"

#include <QtCore/QtMath>
#include <QtPositioning/QGeoCoordinate>

VehicleGPSFactGroup::VehicleGPSFactGroup(QObject *parent)
    : FactGroup(1000, ":/json/Vehicle/GPSFact.json", parent)
{
    _addFact(&_latFact);
    _addFact(&_lonFact);
    _addFact(&_mgrsFact);
    _addFact(&_altitudeMSLFact);
    _addFact(&_altitudeEllipsoidFact);
    _addFact(&_hdopFact);
    _addFact(&_vdopFact);
    _addFact(&_hAccFact);
    _addFact(&_vAccFact);
    _addFact(&_courseOverGroundFact);
    _addFact(&_yawFact);
    _addFact(&_groundSpeedFact);
    _addFact(&_velAccFact);
    _addFact(&_hdgAccFact);
    _addFact(&_lockFact);
    _addFact(&_countFact);
    _addFact(&_systemErrorsFact);
    _addFact(&_spoofingStateFact);
    _addFact(&_jammingStateFact);
    _addFact(&_authenticationStateFact);
    _addFact(&_correctionsQualityFact);
    _addFact(&_systemQualityFact);
    _addFact(&_gnssSignalQualityFact);
    _addFact(&_postProcessingQualityFact);
    _addFact(&_rtkHealthFact);
    _addFact(&_rtkRateFact);
    _addFact(&_rtkNumSatsFact);
    _addFact(&_rtkBaselineFact);
    _addFact(&_rtkAccuracyFact);
    _addFact(&_rtkIARFact);

    _latFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _lonFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _mgrsFact.setRawValue("");
    _altitudeMSLFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _altitudeEllipsoidFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _hdopFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _vdopFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _hAccFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _vAccFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _courseOverGroundFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _yawFact.setRawValue(std::numeric_limits<int16_t>::quiet_NaN());
    _groundSpeedFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _velAccFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _hdgAccFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _rtkBaselineFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _rtkAccuracyFact.setRawValue(std::numeric_limits<float>::quiet_NaN());
    _spoofingStateFact.setRawValue(255);
    _jammingStateFact.setRawValue(255);
    _authenticationStateFact.setRawValue(255);
    _correctionsQualityFact.setRawValue(255);
    _systemQualityFact.setRawValue(255);
    _gnssSignalQualityFact.setRawValue(255);
    _postProcessingQualityFact.setRawValue(255);
}

VehicleGPSFactGroup::GPSQuality VehicleGPSFactGroup::quality() const
{
    const int fixType = _lockFact.rawValue().toInt();
    const int sats = _countFact.rawValue().toInt();
    const double hdopVal = _hdopFact.rawValue().toDouble();

    if (fixType < static_cast<int>(GPSFixType::Fix2D))
        return GPSQuality::QualityNone;

    if (fixType >= static_cast<int>(GPSFixType::FixRTKFixed))
        return GPSQuality::QualityExcellent;

    if (fixType >= static_cast<int>(GPSFixType::FixRTKFloat))
        return sats >= 10 ? GPSQuality::QualityGood : GPSQuality::QualityFair;

    // 3D/DGPS fix — score based on HDOP and satellite count
    if (fixType >= static_cast<int>(GPSFixType::Fix3D)) {
        const bool goodHdop = !qIsNaN(hdopVal) && hdopVal < 2.0;
        const bool goodSats = sats >= 12;
        if (goodHdop && goodSats) return GPSQuality::QualityGood;
        if (goodHdop || goodSats) return GPSQuality::QualityFair;
        return GPSQuality::QualityPoor;
    }

    return GPSQuality::QualityPoor;
}

void VehicleGPSFactGroup::handleMessage(Vehicle *vehicle, const mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    switch (message.msgid) {
    case MAVLINK_MSG_ID_GPS_RAW_INT:
        _handleGpsRawInt(message);
        break;
    case MAVLINK_MSG_ID_HIGH_LATENCY:
        _handleHighLatency(message);
        break;
    case MAVLINK_MSG_ID_HIGH_LATENCY2:
        _handleHighLatency2(message);
        break;
    case MAVLINK_MSG_ID_GNSS_INTEGRITY:
        _handleGnssIntegrity(message);
        break;
    case MAVLINK_MSG_ID_GPS_RTK:
        _handleGpsRtk(message);
        break;
    default:
        break;
    }
}

void VehicleGPSFactGroup::_handleGpsRawInt(const mavlink_message_t &message)
{
    mavlink_gps_raw_int_t gpsRawInt{};
    mavlink_msg_gps_raw_int_decode(&message, &gpsRawInt);

    lat()->setRawValue(gpsRawInt.lat * 1e-7);
    lon()->setRawValue(gpsRawInt.lon * 1e-7);
    mgrs()->setRawValue(QGCGeo::convertGeoToMGRS(QGeoCoordinate(gpsRawInt.lat * 1e-7, gpsRawInt.lon * 1e-7)));
    altitudeMSL()->setRawValue(gpsRawInt.alt / 1000.0);
    altitudeEllipsoid()->setRawValue(gpsRawInt.alt_ellipsoid / 1000.0);
    count()->setRawValue((gpsRawInt.satellites_visible == 255) ? 0 : gpsRawInt.satellites_visible);
    hdop()->setRawValue((gpsRawInt.eph == UINT16_MAX) ? qQNaN() : (gpsRawInt.eph / 100.0));
    vdop()->setRawValue((gpsRawInt.epv == UINT16_MAX) ? qQNaN() : (gpsRawInt.epv / 100.0));
    hAcc()->setRawValue((gpsRawInt.h_acc == UINT32_MAX) ? qQNaN() : (gpsRawInt.h_acc / 1000.0));
    vAcc()->setRawValue((gpsRawInt.v_acc == UINT32_MAX) ? qQNaN() : (gpsRawInt.v_acc / 1000.0));
    courseOverGround()->setRawValue((gpsRawInt.cog == UINT16_MAX) ? qQNaN() : (gpsRawInt.cog / 100.0));
    yaw()->setRawValue((gpsRawInt.yaw == UINT16_MAX) ? qQNaN() : (gpsRawInt.yaw / 100.0));
    groundSpeed()->setRawValue((gpsRawInt.vel == UINT16_MAX) ? qQNaN() : (gpsRawInt.vel / 100.0));
    velAcc()->setRawValue((gpsRawInt.vel_acc == 0) ? qQNaN() : (gpsRawInt.vel_acc / 1000.0));
    hdgAcc()->setRawValue((gpsRawInt.hdg_acc == 0) ? qQNaN() : (gpsRawInt.hdg_acc / 1e5));
    lock()->setRawValue(gpsRawInt.fix_type);

    _setTelemetryAvailable(true);

    _emitQualityIfChanged();
}

void VehicleGPSFactGroup::_handleHighLatency(const mavlink_message_t &message)
{
    mavlink_high_latency_t highLatency{};
    mavlink_msg_high_latency_decode(&message, &highLatency);

    lat()->setRawValue(highLatency.latitude * 1e-7);
    lon()->setRawValue(highLatency.longitude * 1e-7);
    mgrs()->setRawValue(QGCGeo::convertGeoToMGRS(QGeoCoordinate(highLatency.latitude * 1e-7, highLatency.longitude * 1e-7, highLatency.altitude_amsl)));
    lock()->setRawValue(highLatency.gps_fix_type);
    count()->setRawValue((highLatency.gps_nsat == UINT8_MAX) ? 0 : highLatency.gps_nsat);
    // HIGH_LATENCY carries no DOP/accuracy — clear any stale values from a prior source
    // so quality() and the UI don't report leftover figures.
    hdop()->setRawValue(qQNaN());
    vdop()->setRawValue(qQNaN());
    hAcc()->setRawValue(qQNaN());
    vAcc()->setRawValue(qQNaN());

    _setTelemetryAvailable(true);

    _emitQualityIfChanged();
}

void VehicleGPSFactGroup::_handleHighLatency2(const mavlink_message_t &message)
{
    mavlink_high_latency2_t highLatency2{};
    mavlink_msg_high_latency2_decode(&message, &highLatency2);

    lat()->setRawValue(highLatency2.latitude * 1e-7);
    lon()->setRawValue(highLatency2.longitude * 1e-7);
    mgrs()->setRawValue(QGCGeo::convertGeoToMGRS(QGeoCoordinate(highLatency2.latitude * 1e-7, highLatency2.longitude * 1e-7, highLatency2.altitude)));
    count()->setRawValue(0);
    hdop()->setRawValue((highLatency2.eph == UINT8_MAX) ? qQNaN() : (highLatency2.eph / 10.0));
    vdop()->setRawValue((highLatency2.epv == UINT8_MAX) ? qQNaN() : (highLatency2.epv / 10.0));
    // HIGH_LATENCY2 carries no accuracy fields — clear stale values from a prior source.
    hAcc()->setRawValue(qQNaN());
    vAcc()->setRawValue(qQNaN());

    _setTelemetryAvailable(true);

    _emitQualityIfChanged();
}

void VehicleGPSFactGroup::_handleGnssIntegrity(const mavlink_message_t& message)
{
    mavlink_gnss_integrity_t gnssIntegrity;
    mavlink_msg_gnss_integrity_decode(&message, &gnssIntegrity);

    if (_gnssIntegrityId < 0) {
        _gnssIntegrityId = gnssIntegrity.id;  // latch onto the first reporting receiver
    } else if (gnssIntegrity.id != _gnssIntegrityId) {
        return;
    }

    systemErrors()->setRawValue         (gnssIntegrity.system_errors);
    spoofingState()->setRawValue        (gnssIntegrity.spoofing_state);
    jammingState()->setRawValue         (gnssIntegrity.jamming_state);
    authenticationState()->setRawValue  (gnssIntegrity.authentication_state);
    correctionsQuality()->setRawValue   (gnssIntegrity.corrections_quality);
    systemQuality()->setRawValue        (gnssIntegrity.system_status_summary);
    gnssSignalQuality()->setRawValue    (gnssIntegrity.gnss_signal_quality);
    postProcessingQuality()->setRawValue(gnssIntegrity.post_processing_quality);

    emit gnssIntegrityReceived();
}

void VehicleGPSFactGroup::_handleGpsRtk(const mavlink_message_t &message)
{
    mavlink_gps_rtk_t gpsRtk{};
    mavlink_msg_gps_rtk_decode(&message, &gpsRtk);

    if (_rtkReceiverId < 0) {
        _rtkReceiverId = gpsRtk.rtk_receiver_id;  // latch onto the first reporting receiver
    } else if (gpsRtk.rtk_receiver_id != _rtkReceiverId) {
        return;
    }

    rtkHealth()->setRawValue(gpsRtk.rtk_health);
    rtkRate()->setRawValue(gpsRtk.rtk_rate);
    rtkNumSats()->setRawValue(gpsRtk.nsats);
    rtkIAR()->setRawValue(gpsRtk.iar_num_hypotheses);
    rtkAccuracy()->setRawValue(gpsRtk.accuracy / 1000.0);

    const double a = gpsRtk.baseline_a_mm / 1000.0;
    const double b = gpsRtk.baseline_b_mm / 1000.0;
    const double c = gpsRtk.baseline_c_mm / 1000.0;
    rtkBaseline()->setRawValue(qSqrt((a * a) + (b * b) + (c * c)));
}

void VehicleGPSFactGroup::_emitQualityIfChanged()
{
    const GPSQuality newQuality = quality();
    if (newQuality != _lastQuality) {
        _lastQuality = newQuality;
        emit qualityChanged();
    }
}
