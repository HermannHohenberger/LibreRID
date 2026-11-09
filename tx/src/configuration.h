#pragma once

#include <Arduino.h>

struct LibreRidConfig {
  String operator_id;
  String serial_number;
  bool test_profile = false;
};
class ConfigurationManager {
 public:
  bool begin();
  const LibreRidConfig& current() const;
  LibreRidConfig& pending();
  bool isValid(const LibreRidConfig& config, String* reason = nullptr) const;
  bool save(String* reason = nullptr);
  void reset();
  String maskedOperatorId() const;

  static bool validateEuOperatorId(const String& value,
                                   String* reason = nullptr);
  static bool validateSerialNumber(const String& value,
                                   String* reason = nullptr);

 private:
  LibreRidConfig config_;
};
