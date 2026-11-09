#include "inav_msp.h"

namespace {
constexpr uint16_t kMspApiVersion = 1;
constexpr uint16_t kMspFcVariant = 2;
constexpr uint16_t kMspRawGps = 106;
constexpr uint16_t kMspAltitude = 109;
constexpr uint16_t kMspActiveBoxes = 113;
constexpr uint16_t kMspBoxIds = 119;

uint16_t readU16(const uint8_t* value) {
  return value[0] | (static_cast<uint16_t>(value[1]) << 8);
}

int16_t readI16(const uint8_t* value) {
  return static_cast<int16_t>(readU16(value));
}

uint32_t readU32(const uint8_t* value) {
  return value[0] | (static_cast<uint32_t>(value[1]) << 8) |
         (static_cast<uint32_t>(value[2]) << 16) |
         (static_cast<uint32_t>(value[3]) << 24);
}

int32_t readI32(const uint8_t* value) {
  return static_cast<int32_t>(readU32(value));
}
}  // namespace

InavMspClient::InavMspClient(HardwareSerial& serial, int8_t rx_pin,
                             int8_t tx_pin, uint32_t baud)
    : serial_(serial), rx_pin_(rx_pin), tx_pin_(tx_pin), baud_(baud) {}

void InavMspClient::begin() {
  serial_.begin(baud_, SERIAL_8N1, rx_pin_, tx_pin_);
  parser_.setHandler(frameHandler, this);
  begin_ms_ = millis();
}

void InavMspClient::update() {
  while (serial_.available() > 0) {
    received_bytes_++;
    parser_.consume(static_cast<uint8_t>(serial_.read()));
  }

  const uint32_t now = millis();
  if (!data_.fc_detected && version_ == MspVersion::V2Native &&
      now - begin_ms_ >= 2000) {
    version_ = MspVersion::V1;
    last_identity_request_ms_ = 0;
  }
  scheduleRequests(now);
}

const FlightData& InavMspClient::data() const { return data_; }
MspVersion InavMspClient::activeVersion() const { return version_; }
uint32_t InavMspClient::receivedBytes() const { return received_bytes_; }
uint32_t InavMspClient::receivedFrames() const { return parser_.validFrames(); }
uint32_t InavMspClient::checksumErrors() const {
  return parser_.checksumErrors();
}
uint32_t InavMspClient::errorFrames() const { return error_frames_; }
uint32_t InavMspClient::sentRequests() const { return sent_requests_; }

void InavMspClient::frameHandler(const MspFrame& frame, void* context) {
  static_cast<InavMspClient*>(context)->handleFrame(frame);
}

void InavMspClient::handleFrame(const MspFrame& frame) {
  if (frame.error) {
    error_frames_++;
    return;
  }
  const uint32_t now = millis();
  data_.fc_detected = true;
  data_.last_fc_data_ms = now;

  switch (frame.command) {
    case kMspApiVersion:
      if (frame.payload_length >= 3) {
        data_.api_major = frame.payload[1];
        data_.api_minor = frame.payload[2];
      }
      break;
    case kMspFcVariant:
      if (frame.payload_length >= 4) {
        data_.inav_detected = memcmp(frame.payload, "INAV", 4) == 0;
      }
      break;
    case kMspRawGps:
      if (frame.payload_length >= 18) {
        data_.fix_type = frame.payload[0];
        data_.satellites = frame.payload[1];
        data_.latitude_e7 = readI32(&frame.payload[2]);
        data_.longitude_e7 = readI32(&frame.payload[6]);
        data_.gnss_altitude_cm = static_cast<int32_t>(readU16(&frame.payload[10])) * 100;
        data_.ground_speed_cms = readU16(&frame.payload[12]);
        data_.ground_course_decideg = readU16(&frame.payload[14]);
        data_.hdop = readU16(&frame.payload[16]);
        data_.gps_valid = data_.fix_type >= 2 && data_.satellites > 0 &&
                          (data_.latitude_e7 != 0 || data_.longitude_e7 != 0);
        data_.last_gps_ms = now;
      }
      break;
    case kMspAltitude:
      if (frame.payload_length >= 6) {
        data_.relative_altitude_cm = readI32(&frame.payload[0]);
        data_.vertical_speed_cms = readI16(&frame.payload[4]);
        data_.altitude_valid = true;
        data_.last_altitude_ms = now;
      }
      break;
    case kMspBoxIds:
      decodeBoxIds(frame);
      break;
    case kMspActiveBoxes:
      decodeActiveBoxes(frame, now);
      break;
  }
}

void InavMspClient::sendRequest(uint16_t command) {
  if (writeMspRequest(serial_, version_, command) > 0) sent_requests_++;
}

void InavMspClient::scheduleRequests(uint32_t now) {
  if (now - last_identity_request_ms_ >= 1000 &&
      (!data_.fc_detected || !data_.inav_detected)) {
    last_identity_request_ms_ = now;
    sendRequest(data_.api_major == 0 ? kMspApiVersion : kMspFcVariant);
    return;
  }
  if (arm_box_index_ < 0 && now - last_boxes_request_ms_ >= 1000) {
    last_boxes_request_ms_ = now;
    sendRequest(kMspBoxIds);
    return;
  }
  if (now - last_gps_request_ms_ >= 200) {
    last_gps_request_ms_ = now;
    sendRequest(kMspRawGps);
    return;
  }
  if (now - last_active_boxes_request_ms_ >= 200) {
    last_active_boxes_request_ms_ = now;
    sendRequest(kMspActiveBoxes);
    return;
  }
  if (now - last_altitude_request_ms_ >= 500) {
    last_altitude_request_ms_ = now;
    sendRequest(kMspAltitude);
  }
}

void InavMspClient::decodeBoxIds(const MspFrame& frame) {
  for (uint16_t index = 0; index < frame.payload_length; ++index) {
    if (frame.payload[index] == 0) {
      arm_box_index_ = index;
      return;
    }
  }
}

void InavMspClient::decodeActiveBoxes(const MspFrame& frame, uint32_t now) {
  if (arm_box_index_ < 0) return;
  const uint16_t byte_index = static_cast<uint16_t>(arm_box_index_) / 8;
  const uint8_t bit_index = static_cast<uint8_t>(arm_box_index_) % 8;
  if (byte_index >= frame.payload_length) return;
  data_.armed = (frame.payload[byte_index] & (1U << bit_index)) != 0;
  data_.arm_state_valid = true;
  data_.last_arm_state_ms = now;
}
