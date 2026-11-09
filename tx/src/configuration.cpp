#include "configuration.h"

#include <Preferences.h>

extern "C" {
#include <opendroneid.h>
}

namespace {
constexpr char kNamespace[] = "librerid";
constexpr char kAlphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
constexpr char kLegacyDevelopmentSerial[] = "LIBR0DEV000000000001";
constexpr char kDevelopmentSerial[] = "LIBRERIDHH0000000001";

int codePoint(char value) {
  const char* found = strchr(kAlphabet, value);
  return found == nullptr ? -1 : static_cast<int>(found - kAlphabet);
}

char luhnMod36CheckCharacter(const String& payload) {
  int factor = 2;
  int sum = 0;
  for (int index = payload.length() - 1; index >= 0; --index) {
    const int code_point = codePoint(payload[index]);
    if (code_point < 0) {
      return '\0';
    }
    const int addend = factor * code_point;
    sum += (addend / 36) + (addend % 36);
    factor = factor == 2 ? 1 : 2;
  }
  return kAlphabet[(36 - (sum % 36)) % 36];
}

void setReason(String* reason, const __FlashStringHelper* value) {
  if (reason != nullptr) {
    *reason = value;
  }
}
}  // namespace

bool ConfigurationManager::begin() {
  Preferences preferences;
  if (!preferences.begin(kNamespace, true)) {
    return false;
  }
  config_.operator_id = preferences.getString("operator", "");
  config_.serial_number = preferences.getString("serial", "");
  config_.test_profile = preferences.getBool("test", false);
  preferences.end();
  return true;
}

const LibreRidConfig& ConfigurationManager::current() const { return config_; }

LibreRidConfig& ConfigurationManager::pending() { return config_; }

bool ConfigurationManager::isValid(const LibreRidConfig& config,
                                   String* reason) const {
  if (config.test_profile) {
    if (config.serial_number != kLegacyDevelopmentSerial &&
        config.serial_number != kDevelopmentSerial) {
      setReason(reason, F("development profile serial does not match"));
      return false;
    }
    if (config.operator_id == "DE.TEST.LIBRERID") {
      return true;
    }
    return validateEuOperatorId(config.operator_id, reason);
  }
  if (!validateEuOperatorId(config.operator_id, reason)) {
    return false;
  }
  return validateSerialNumber(config.serial_number, reason);
}

bool ConfigurationManager::save(String* reason) {
  if (!isValid(config_, reason)) {
    return false;
  }
  Preferences preferences;
  if (!preferences.begin(kNamespace, false)) {
    setReason(reason, F("could not open NVS"));
    return false;
  }
  const bool ok = preferences.putString("operator", config_.operator_id) > 0 &&
                  preferences.putString("serial", config_.serial_number) > 0 &&
                  preferences.putBool("test", config_.test_profile) > 0;
  preferences.end();
  if (!ok) {
    setReason(reason, F("could not write NVS"));
  }
  return ok;
}

void ConfigurationManager::reset() {
  Preferences preferences;
  if (preferences.begin(kNamespace, false)) {
    preferences.clear();
    preferences.end();
  }
  config_ = {};
}

String ConfigurationManager::maskedOperatorId() const {
  if (config_.operator_id == "DE.TEST.LIBRERID") {
    return F("<public M1 test ID>");
  }
  if (config_.operator_id.length() != 20) {
    return F("<not configured>");
  }
  return config_.operator_id.substring(0, 3) + F("************") +
         config_.operator_id.substring(15, 17) + F("***");
}

bool ConfigurationManager::validateEuOperatorId(const String& value,
                                                String* reason) {
  if (value.length() != 20 || value[16] != '-') {
    setReason(reason, F("operator ID must be CCCxxxxxxxxxxxxc-sss"));
    return false;
  }
  for (int i = 0; i < 3; ++i) {
    if (value[i] < 'A' || value[i] > 'Z') {
      setReason(reason, F("country code must be three uppercase letters"));
      return false;
    }
  }
  for (int i = 3; i < 16; ++i) {
    if (codePoint(value[i]) < 0) {
      setReason(reason, F("registration body must use lowercase a-z or 0-9"));
      return false;
    }
  }
  for (int i = 17; i < 20; ++i) {
    if (codePoint(value[i]) < 0) {
      setReason(reason, F("secret digits must use lowercase a-z or 0-9"));
      return false;
    }
  }

  const String checksum_payload = value.substring(3, 15) + value.substring(17);
  if (luhnMod36CheckCharacter(checksum_payload) != value[15]) {
    setReason(reason, F("operator ID checksum mismatch"));
    return false;
  }
  return true;
}

bool ConfigurationManager::validateSerialNumber(const String& value,
                                                String* reason) {
  if (value.length() != ODID_ID_SIZE) {
    setReason(reason, F("serial number must contain exactly 20 characters"));
    return false;
  }
  for (const char character : value) {
    if (!isAlphaNumeric(character)) {
      setReason(reason, F("serial number must be alphanumeric"));
      return false;
    }
  }
  if (value == kLegacyDevelopmentSerial || value == kDevelopmentSerial) {
    setReason(reason, F("development serial only allowed by test profile"));
    return false;
  }
  return true;
}
