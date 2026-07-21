#include <Arduino.h>
#include <BLEAdvertising.h>
#include <BLEDevice.h>
#include <esp_system.h>

#include "configuration.h"
#include "flight_state.h"
#include "inav_msp.h"
#include "msp_protocol.h"
#include "status_led.h"

extern "C" {
#include <opendroneid.h>
}

namespace {
constexpr uint8_t kRgbLedPin = 48;
constexpr uint32_t kMessageIntervalMs = 200;
constexpr uint8_t kMessageTypeCount = 5;

BLEMultiAdvertising advertiser(1);
ODID_UAS_Data uas_data;
ConfigurationManager configuration;
StatusLed status_led(kRgbLedPin);
InavMspClient inav_msp(Serial1, 18, 17);
FlightStateManager flight_state;
uint8_t message_counters[ODID_MSG_COUNTER_AMOUNT] = {};
uint8_t legacy_payload[31] = {};
uint8_t phase = 0;
uint32_t last_message_ms = 0;
bool broadcasting = false;
bool msp_probe_active = false;
bool dynamic_broadcast = false;
FlightStateStatus last_flight_status = FlightStateStatus::WaitingForFc;
String console_line;

esp_ble_gap_ext_adv_params_t legacy_adv_params = {
    .type = ESP_BLE_GAP_SET_EXT_ADV_PROP_LEGACY_NONCONN,
    .interval_min = 160,  // 100 ms in 0.625 ms units
    .interval_max = 160,
    .channel_map = ADV_CHNL_ALL,
    .own_addr_type = BLE_ADDR_TYPE_RANDOM,
    .filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    .tx_power = ESP_PWR_LVL_P3,
    .primary_phy = ESP_BLE_GAP_PHY_1M,
    .max_skip = 0,
    .secondary_phy = ESP_BLE_GAP_PHY_1M,
    .sid = 0,
    .scan_req_notif = false,
};

void copyText(char* destination, size_t destination_size, const char* source) {
  strlcpy(destination, source, destination_size);
}

void initialiseStaticData(const LibreRidConfig& config) {
  odid_initUasData(&uas_data);

  uas_data.BasicID[0].IDType = ODID_IDTYPE_SERIAL_NUMBER;
  uas_data.BasicID[0].UAType = ODID_UATYPE_AEROPLANE;
  copyText(uas_data.BasicID[0].UASID, sizeof(uas_data.BasicID[0].UASID),
           config.serial_number.c_str());
  uas_data.BasicIDValid[0] = 1;

  uas_data.Location.Status = ODID_STATUS_GROUND;
  uas_data.Location.AltitudeBaro = INV_ALT;
  uas_data.Location.HeightType = ODID_HEIGHT_REF_OVER_TAKEOFF;
  uas_data.Location.Height = INV_ALT;
  uas_data.Location.HorizAccuracy = ODID_HOR_ACC_UNKNOWN;
  uas_data.Location.VertAccuracy = ODID_VER_ACC_UNKNOWN;
  uas_data.Location.BaroAccuracy = ODID_VER_ACC_UNKNOWN;
  uas_data.Location.SpeedAccuracy = ODID_SPEED_ACC_UNKNOWN;
  uas_data.Location.TSAccuracy = ODID_TIME_ACC_UNKNOWN;
  uas_data.Location.TimeStamp = INV_TIMESTAMP;
  uas_data.LocationValid = 0;

  uas_data.SelfID.DescType = ODID_DESC_TYPE_TEXT;
  copyText(uas_data.SelfID.Desc, sizeof(uas_data.SelfID.Desc),
           "LibreRID M1 TEST ONLY");
  uas_data.SelfIDValid = 1;

  uas_data.System.OperatorLocationType = ODID_OPERATOR_LOCATION_TYPE_TAKEOFF;
  uas_data.System.ClassificationType = ODID_CLASSIFICATION_TYPE_UNDECLARED;
  uas_data.System.AreaCount = 1;
  uas_data.System.AreaRadius = 0;
  uas_data.System.AreaCeiling = INV_ALT;
  uas_data.System.AreaFloor = INV_ALT;
  uas_data.System.Timestamp = 0;
  uas_data.SystemValid = 0;

  uas_data.OperatorID.OperatorIdType = ODID_OPERATOR_ID;
  const String public_operator_id = config.operator_id.substring(0, 16);
  copyText(uas_data.OperatorID.OperatorId,
           sizeof(uas_data.OperatorID.OperatorId), public_operator_id.c_str());
  uas_data.OperatorIDValid = 1;
}

bool encodeCurrentMessage(uint8_t* destination) {
  switch (phase) {
    case 0:
      message_counters[ODID_MSG_COUNTER_LOCATION]++;
      return encodeLocationMessage(
                 reinterpret_cast<ODID_Location_encoded*>(destination),
                 &uas_data.Location) == ODID_SUCCESS;
    case 1:
      message_counters[ODID_MSG_COUNTER_BASIC_ID]++;
      return encodeBasicIDMessage(
                 reinterpret_cast<ODID_BasicID_encoded*>(destination),
                 &uas_data.BasicID[0]) == ODID_SUCCESS;
    case 2:
      message_counters[ODID_MSG_COUNTER_SELF_ID]++;
      return encodeSelfIDMessage(
                 reinterpret_cast<ODID_SelfID_encoded*>(destination),
                 &uas_data.SelfID) == ODID_SUCCESS;
    case 3:
      message_counters[ODID_MSG_COUNTER_SYSTEM]++;
      return encodeSystemMessage(
                 reinterpret_cast<ODID_System_encoded*>(destination),
                 &uas_data.System) == ODID_SUCCESS;
    case 4:
      message_counters[ODID_MSG_COUNTER_OPERATOR_ID]++;
      return encodeOperatorIDMessage(
                 reinterpret_cast<ODID_OperatorID_encoded*>(destination),
                 &uas_data.OperatorID) == ODID_SUCCESS;
    default:
      return false;
  }
}

bool currentMessageValid() {
  switch (phase) {
    case 0:
      return uas_data.LocationValid != 0;
    case 1:
      return uas_data.BasicIDValid[0] != 0;
    case 2:
      return uas_data.SelfIDValid != 0;
    case 3:
      return uas_data.SystemValid != 0;
    case 4:
      return uas_data.OperatorIDValid != 0;
    default:
      return false;
  }
}

uint8_t currentCounter() {
  constexpr uint8_t counters[kMessageTypeCount] = {
      ODID_MSG_COUNTER_LOCATION, ODID_MSG_COUNTER_BASIC_ID,
      ODID_MSG_COUNTER_SELF_ID, ODID_MSG_COUNTER_SYSTEM,
      ODID_MSG_COUNTER_OPERATOR_ID};
  return message_counters[counters[phase]];
}

bool publishCurrentMessage() {
  constexpr uint8_t header[] = {0x1e, 0x16, 0xfa, 0xff, 0x0d};
  for (size_t attempts = 0;
       attempts < kMessageTypeCount && !currentMessageValid(); ++attempts) {
    phase = (phase + 1) % kMessageTypeCount;
  }
  memset(legacy_payload, 0, sizeof(legacy_payload));
  memcpy(legacy_payload, header, sizeof(header));

  if (!encodeCurrentMessage(&legacy_payload[6])) {
    return false;
  }
  legacy_payload[5] = currentCounter();

  const bool updated =
      advertiser.setAdvertisingData(0, sizeof(legacy_payload), legacy_payload);
  phase = (phase + 1) % kMessageTypeCount;
  return updated;
}

bool initialiseBluetooth() {
  BLEDevice::init("");

  uint8_t random_address[6];
  esp_fill_random(random_address, sizeof(random_address));
  random_address[0] |= 0xC0;

  advertiser.setAdvertisingParams(0, &legacy_adv_params);
  advertiser.setInstanceAddress(0, random_address);
  advertiser.setDuration(0);

  if (!publishCurrentMessage()) {
    return false;
  }
  return advertiser.start();
}

void printConfiguration() {
  String reason;
  Serial.printf("operator_id: %s\n", configuration.maskedOperatorId().c_str());
  Serial.printf("serial_number: %s\n",
                configuration.current().serial_number.length() == 0
                    ? "<not configured>"
                    : configuration.current().serial_number.c_str());
  Serial.printf("profile: %s\n",
                configuration.current().test_profile ? "TEST" : "NORMAL");
  Serial.printf("valid: %s\n",
                configuration.isValid(configuration.current(), &reason) ? "yes"
                                                                        : "no");
  if (reason.length() > 0) {
    Serial.printf("reason: %s\n", reason.c_str());
  }
}

void printHelp() {
  Serial.println("USB configuration commands:");
  Serial.println("  show");
  Serial.println("  set operator CCCxxxxxxxxxxxxc-sss");
  Serial.println("  set serial XXXXXXXXXXXXXXXXXXXX");
  Serial.println("  test       (stage public M1 test values)");
  Serial.println("  save       (validate, persist and restart)");
  Serial.println("  reset      (erase configuration and restart)");
  Serial.println("  msp start  (probe INAV on RX=GPIO18, TX=GPIO17)");
  Serial.println("  msp show");
  Serial.println("  msp broadcast (map live INAV data with TEST identity)");
  Serial.println("  help");
}

void printMspData() {
  const FlightData& data = inav_msp.data();
  const uint32_t now = millis();
  Serial.printf("MSP: protocol=%s fc=%s inav=%s API=%u.%u\n",
                inav_msp.activeVersion() == MspVersion::V2Native ? "v2" : "v1",
                data.fc_detected ? "yes" : "no",
                data.inav_detected ? "yes" : "no", data.api_major,
                data.api_minor);
  Serial.printf("MSP IO: tx_requests=%lu rx_bytes=%lu rx_frames=%lu checksum_errors=%lu error_frames=%lu\n",
                static_cast<unsigned long>(inav_msp.sentRequests()),
                static_cast<unsigned long>(inav_msp.receivedBytes()),
                static_cast<unsigned long>(inav_msp.receivedFrames()),
                static_cast<unsigned long>(inav_msp.checksumErrors()),
                static_cast<unsigned long>(inav_msp.errorFrames()));
  Serial.printf("GPS: valid=%s fix=%u sats=%u lat=%.7f lon=%.7f alt=%.2fm speed=%.2fm/s course=%.1fdeg hdop=%u\n",
                data.gps_valid ? "yes" : "no", data.fix_type,
                data.satellites, data.latitude_e7 / 10000000.0,
                data.longitude_e7 / 10000000.0,
                data.gnss_altitude_cm / 100.0,
                data.ground_speed_cms / 100.0,
                data.ground_course_decideg / 10.0, data.hdop);
  Serial.printf("ALT: valid=%s relative=%.2fm vertical=%.2fm/s ARM: valid=%s armed=%s\n",
                data.altitude_valid ? "yes" : "no",
                data.relative_altitude_cm / 100.0,
                data.vertical_speed_cms / 100.0,
                data.arm_state_valid ? "yes" : "no",
                data.armed ? "yes" : "no");
  Serial.printf("AGES: fc=%lums gps=%lums alt=%lums arm=%lums\n",
                static_cast<unsigned long>(data.last_fc_data_ms == 0 ? now : now - data.last_fc_data_ms),
                static_cast<unsigned long>(data.last_gps_ms == 0 ? now : now - data.last_gps_ms),
                static_cast<unsigned long>(data.last_altitude_ms == 0 ? now : now - data.last_altitude_ms),
                static_cast<unsigned long>(data.last_arm_state_ms == 0 ? now : now - data.last_arm_state_ms));
}

void handleCommand(String command) {
  command.trim();
  if (command.length() == 0) {
    return;
  }
  if (command == "show") {
    printConfiguration();
  } else if (command == "help") {
    printHelp();
  } else if (command.startsWith("set operator ")) {
    configuration.pending().operator_id = command.substring(13);
    configuration.pending().test_profile =
        configuration.pending().serial_number == "LIBR0DEV000000000001";
    Serial.println("Operator ID staged (masked; use 'save' to persist).");
  } else if (command.startsWith("set serial ")) {
    configuration.pending().serial_number = command.substring(11);
    configuration.pending().test_profile =
        configuration.pending().serial_number == "LIBRERIDHH0000000001" ||
        configuration.pending().serial_number == "LIBR0DEV000000000001";
    Serial.println("Serial number staged (use 'save' to persist).");
  } else if (command == "test") {
    configuration.pending().operator_id = "DE.TEST.LIBRERID";
    configuration.pending().serial_number = "LIBRERIDHH0000000001";
    configuration.pending().test_profile = true;
    Serial.println("Public M1 test profile staged; not for flight.");
  } else if (command == "save") {
    String reason;
    if (!configuration.save(&reason)) {
      Serial.printf("ERROR: configuration not saved: %s\n", reason.c_str());
      return;
    }
    Serial.println("Configuration saved. Restarting...");
    Serial.flush();
    delay(250);
    ESP.restart();
  } else if (command == "reset") {
    configuration.reset();
    Serial.println("Configuration erased. Restarting...");
    Serial.flush();
    delay(250);
    ESP.restart();
  } else if (command == "msp start") {
    if (!msp_probe_active) {
      inav_msp.begin();
      msp_probe_active = true;
      Serial.println("MSP probe started at 115200 baud (RX=18, TX=17).");
    } else {
      Serial.println("MSP probe already active.");
    }
  } else if (command == "msp show") {
    printMspData();
  } else if (command == "msp broadcast") {
    if (!configuration.current().test_profile) {
      Serial.println("ERROR: diagnostic live broadcast requires TEST profile");
      return;
    }
    if (!msp_probe_active) {
      inav_msp.begin();
      msp_probe_active = true;
    }
    dynamic_broadcast = true;
    Serial.println("Live INAV mapping enabled with public TEST identity.");
  } else {
    Serial.println("ERROR: unknown command; enter 'help'");
  }
}

void updateConsole() {
  while (Serial.available() > 0) {
    const char value = static_cast<char>(Serial.read());
    if (value == '\n' || value == '\r') {
      if (console_line.length() > 0) {
        handleCommand(console_line);
        console_line = "";
      }
    } else if (console_line.length() < 96) {
      console_line += value;
    }
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  status_led.set(DeviceStatus::Booting);
  delay(1200);

  Serial.println();
  Serial.println("LibreRID MSP diagnostic build");
  Serial.printf("Flash: %u MB, PSRAM: %u MB\n",
                ESP.getFlashChipSize() / (1024U * 1024U),
                ESP.getPsramSize() / (1024U * 1024U));

  if (!ConfigurationManager::validateEuOperatorId("FIN87astrdge12k8-xyz")) {
    Serial.println("FATAL: Luhn-mod-36 self-test failed");
    status_led.set(DeviceStatus::FatalError);
    return;
  }
  Serial.println("Operator ID validator self-test: PASS");
  if (!runMspProtocolSelfTest()) {
    Serial.println("FATAL: MSP v1/v2 parser self-test failed");
    status_led.set(DeviceStatus::FatalError);
    return;
  }
  Serial.println("MSP v1/v2 parser self-test: PASS");

  configuration.begin();
  printHelp();
  printConfiguration();

  String reason;
  if (!configuration.isValid(configuration.current(), &reason)) {
    Serial.printf("CONFIGURATION INVALID: %s\n", reason.c_str());
    status_led.set(DeviceStatus::ConfigurationInvalid);
    return;
  }

  if (configuration.current().test_profile) {
    Serial.println("WARNING: DEVELOPMENT SERIAL / TEST PROFILE - NOT FOR FLIGHT");
  }
  initialiseStaticData(configuration.current());
  if (!initialiseBluetooth()) {
    Serial.println("ERROR: BLE4 advertising failed");
    status_led.set(DeviceStatus::FatalError);
    return;
  }

  Serial.printf("BLE4 active: serial=%s operator=%s\n",
                configuration.current().serial_number.c_str(),
                configuration.maskedOperatorId().c_str());
  status_led.set(DeviceStatus::Broadcasting);
  broadcasting = true;
  last_message_ms = millis();

  inav_msp.begin();
  msp_probe_active = true;
  dynamic_broadcast = true;
  Serial.println("MSP auto-start: 115200 baud (RX=18, TX=17).");
  Serial.println("Live INAV mapping enabled automatically.");
}

void loop() {
  updateConsole();
  status_led.update();
  if (msp_probe_active) {
    inav_msp.update();
  }
  if (dynamic_broadcast) {
    const FlightStateStatus current =
        flight_state.update(inav_msp.data(), uas_data, millis());
    if (current != last_flight_status) {
      last_flight_status = current;
      switch (current) {
        case FlightStateStatus::WaitingForFc:
          status_led.set(DeviceStatus::FlightControllerMissing);
          Serial.println("STATE: waiting for flight controller");
          break;
        case FlightStateStatus::WaitingForGnss:
          status_led.set(DeviceStatus::GnssMissing);
          Serial.println("STATE: waiting for valid GNSS and arm data");
          break;
        case FlightStateStatus::ReadyDisarmed:
          status_led.set(DeviceStatus::ReadyDisarmed);
          Serial.println("STATE: live data valid, disarmed");
          break;
        case FlightStateStatus::Active:
          status_led.set(DeviceStatus::Broadcasting);
          Serial.println("STATE: armed, takeoff point locked");
          break;
        case FlightStateStatus::DataError:
          status_led.set(DeviceStatus::FatalError);
          Serial.println("STATE: live data error while armed");
          break;
      }
    }
  }

  if (!broadcasting) {
    delay(5);
    return;
  }
  const uint32_t now = millis();
  if (now - last_message_ms < kMessageIntervalMs) {
    delay(5);
    return;
  }
  last_message_ms = now;

  if (!publishCurrentMessage()) {
    Serial.println("ERROR: BLE4 payload update failed");
    status_led.set(DeviceStatus::FatalError);
    broadcasting = false;
  }
}
