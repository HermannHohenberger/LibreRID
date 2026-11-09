#pragma once

#include "flight_data.h"

extern "C" {
#include <opendroneid.h>
}

enum class FlightStateStatus {
  WaitingForFc,
  WaitingForGnss,
  ReadyDisarmed,
  Active,
  DataError,
};
class FlightStateManager {
 public:
  FlightStateStatus update(const FlightData& flight, ODID_UAS_Data& uas,
                           uint32_t now);
  bool takeoffPointValid() const;

 private:
  bool was_armed_ = false;
  bool takeoff_valid_ = false;
  bool armed_without_fix_ = false;
  double takeoff_latitude_ = 0;
  double takeoff_longitude_ = 0;
  float takeoff_altitude_m_ = 0;
};
