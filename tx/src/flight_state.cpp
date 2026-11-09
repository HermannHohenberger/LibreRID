#include "flight_state.h"

namespace {
constexpr uint32_t kDataTimeoutMs = 2000;
}

FlightStateStatus FlightStateManager::update(const FlightData& flight,
                                             ODID_UAS_Data& uas,
                                             uint32_t now) {
  const bool fc_fresh = flight.fc_detected &&
                        now - flight.last_fc_data_ms <= kDataTimeoutMs;
  const bool gps_fresh = flight.gps_valid &&
                         now - flight.last_gps_ms <= kDataTimeoutMs;
  const bool altitude_fresh = flight.altitude_valid &&
                              now - flight.last_altitude_ms <= kDataTimeoutMs;
  const bool arm_fresh = flight.arm_state_valid &&
                         now - flight.last_arm_state_ms <= kDataTimeoutMs;

  if (arm_fresh && flight.armed && !was_armed_) {
    if (gps_fresh) {
      takeoff_latitude_ = flight.latitude_e7 / 10000000.0;
      takeoff_longitude_ = flight.longitude_e7 / 10000000.0;
      takeoff_altitude_m_ = flight.gnss_altitude_cm / 100.0F;
      takeoff_valid_ = true;
      armed_without_fix_ = false;
    } else {
      armed_without_fix_ = true;
    }
  }
  if (arm_fresh) was_armed_ = flight.armed;

  if (gps_fresh) {
    uas.Location.Latitude = flight.latitude_e7 / 10000000.0;
    uas.Location.Longitude = flight.longitude_e7 / 10000000.0;
    uas.Location.AltitudeGeo = flight.gnss_altitude_cm / 100.0F;
    uas.Location.Direction = flight.ground_course_decideg / 10.0F;
    uas.Location.SpeedHorizontal = flight.ground_speed_cms / 100.0F;
    uas.Location.SpeedVertical =
        altitude_fresh ? flight.vertical_speed_cms / 100.0F : INV_SPEED_V;
    uas.Location.HeightType = ODID_HEIGHT_REF_OVER_TAKEOFF;
    uas.Location.Height =
        altitude_fresh ? flight.relative_altitude_cm / 100.0F : INV_ALT;
    uas.Location.HorizAccuracy = ODID_HOR_ACC_UNKNOWN;
    uas.Location.VertAccuracy = ODID_VER_ACC_UNKNOWN;
    uas.Location.BaroAccuracy = ODID_VER_ACC_UNKNOWN;
    uas.Location.SpeedAccuracy = ODID_SPEED_ACC_UNKNOWN;
    uas.Location.TSAccuracy = ODID_TIME_ACC_UNKNOWN;
    uas.Location.TimeStamp = INV_TIMESTAMP;
    uas.LocationValid = 1;
  }

  if (takeoff_valid_) {
    uas.System.OperatorLatitude = takeoff_latitude_;
    uas.System.OperatorLongitude = takeoff_longitude_;
    uas.System.OperatorAltitudeGeo = takeoff_altitude_m_;
    uas.SystemValid = 1;
  } else if (gps_fresh && !flight.armed) {
    uas.System.OperatorLatitude = flight.latitude_e7 / 10000000.0;
    uas.System.OperatorLongitude = flight.longitude_e7 / 10000000.0;
    uas.System.OperatorAltitudeGeo = flight.gnss_altitude_cm / 100.0F;
    uas.SystemValid = 1;
  }

  if (!fc_fresh) {
    uas.Location.Status = ODID_STATUS_REMOTE_ID_SYSTEM_FAILURE;
    return flight.armed ? FlightStateStatus::DataError
                        : FlightStateStatus::WaitingForFc;
  }
  if (!gps_fresh || !arm_fresh) {
    uas.Location.Status = ODID_STATUS_REMOTE_ID_SYSTEM_FAILURE;
    return flight.armed ? FlightStateStatus::DataError
                        : FlightStateStatus::WaitingForGnss;
  }
  if (flight.armed && (armed_without_fix_ || !takeoff_valid_)) {
    uas.Location.Status = ODID_STATUS_REMOTE_ID_SYSTEM_FAILURE;
    return FlightStateStatus::DataError;
  }
  if (flight.armed) {
    uas.Location.Status = ODID_STATUS_AIRBORNE;
    return FlightStateStatus::Active;
  }
  uas.Location.Status = ODID_STATUS_GROUND;
  return FlightStateStatus::ReadyDisarmed;
}

bool FlightStateManager::takeoffPointValid() const { return takeoff_valid_; }
