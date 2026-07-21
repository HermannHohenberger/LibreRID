#pragma once

#include <stdint.h>

struct FlightData {
  bool fc_detected = false;
  bool inav_detected = false;
  uint8_t api_major = 0;
  uint8_t api_minor = 0;

  uint8_t fix_type = 0;
  uint8_t satellites = 0;
  int32_t latitude_e7 = 0;
  int32_t longitude_e7 = 0;
  int32_t gnss_altitude_cm = 0;
  uint16_t ground_speed_cms = 0;
  uint16_t ground_course_decideg = 0;
  uint16_t hdop = 0;
  bool gps_valid = false;

  int32_t relative_altitude_cm = 0;
  int16_t vertical_speed_cms = 0;
  bool altitude_valid = false;

  bool armed = false;
  bool arm_state_valid = false;

  uint32_t last_fc_data_ms = 0;
  uint32_t last_gps_ms = 0;
  uint32_t last_altitude_ms = 0;
  uint32_t last_arm_state_ms = 0;
};
