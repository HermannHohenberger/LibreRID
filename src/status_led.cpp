#include "status_led.h"

StatusLed::StatusLed(uint8_t pin, bool rgb, bool active_low)
    : pin_(pin), rgb_(rgb), active_low_(active_low) {}

void StatusLed::set(DeviceStatus status) {
  status_ = status;
  last_change_ms_ = 0;
  on_ = false;
  write(false);
}

void StatusLed::update() {
  if (status_ == DeviceStatus::Broadcasting ||
      status_ == DeviceStatus::FatalError ||
      status_ == DeviceStatus::GnssMissing) {
    write(true);
    return;
  }
  const uint32_t now = millis();
  if (now - last_change_ms_ >= 500) {
    last_change_ms_ = now;
    on_ = !on_;
    write(on_);
  }
}

void StatusLed::write(bool on) {
  if (!initialised_) {
    if (!rgb_) pinMode(pin_, OUTPUT);
    initialised_ = true;
  }
  if (!rgb_) {
    digitalWrite(pin_, active_low_ ? !on : on);
    return;
  }
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  if (on) {
    switch (status_) {
      case DeviceStatus::Booting:
        blue = 32;
        break;
      case DeviceStatus::ConfigurationInvalid:
      case DeviceStatus::FatalError:
        red = 32;
        break;
      case DeviceStatus::FlightControllerMissing:
      case DeviceStatus::GnssMissing:
        red = 24;
        green = 12;
        break;
      case DeviceStatus::ReadyDisarmed:
      case DeviceStatus::Broadcasting:
        green = 32;
        break;
    }
  }
  neopixelWrite(pin_, red, green, blue);
}
