#pragma once

#include <Arduino.h>

enum class DeviceStatus {
  Booting,
  ConfigurationInvalid,
  FlightControllerMissing,
  GnssMissing,
  ReadyDisarmed,
  Broadcasting,
  FatalError,
};

class StatusLed {
 public:
  explicit StatusLed(uint8_t pin);
  void set(DeviceStatus status);
  void update();

 private:
  void write(bool on);

  uint8_t pin_;
  DeviceStatus status_ = DeviceStatus::Booting;
  uint32_t last_change_ms_ = 0;
  bool on_ = false;
};
