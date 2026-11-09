#pragma once

#include <Arduino.h>

#include "flight_data.h"
#include "msp_protocol.h"

class InavMspClient {
 public:
  InavMspClient(HardwareSerial& serial, int8_t rx_pin, int8_t tx_pin,
                uint32_t baud = 115200);
  void begin();
  void update();
  const FlightData& data() const;
  MspVersion activeVersion() const;
  uint32_t receivedBytes() const;
  uint32_t receivedFrames() const;
  uint32_t checksumErrors() const;
  uint32_t errorFrames() const;
  uint32_t sentRequests() const;

 private:
  static void frameHandler(const MspFrame& frame, void* context);
  void handleFrame(const MspFrame& frame);
  void sendRequest(uint16_t command);
  void scheduleRequests(uint32_t now);
  void decodeBoxIds(const MspFrame& frame);
  void decodeActiveBoxes(const MspFrame& frame, uint32_t now);

  HardwareSerial& serial_;
  int8_t rx_pin_;
  int8_t tx_pin_;
  uint32_t baud_;
  MspParser parser_;
  FlightData data_;
  MspVersion version_ = MspVersion::V2Native;
  uint32_t begin_ms_ = 0;
  uint32_t last_identity_request_ms_ = 0;
  uint32_t last_gps_request_ms_ = 0;
  uint32_t last_altitude_request_ms_ = 0;
  uint32_t last_boxes_request_ms_ = 0;
  uint32_t last_active_boxes_request_ms_ = 0;
  uint32_t received_bytes_ = 0;
  uint32_t error_frames_ = 0;
  uint32_t sent_requests_ = 0;
  int16_t arm_box_index_ = -1;
};
