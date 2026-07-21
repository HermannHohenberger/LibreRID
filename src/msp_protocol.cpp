#include "msp_protocol.h"

void MspParser::setHandler(MspFrameHandler handler, void* context) {
  handler_ = handler;
  context_ = context;
}
void MspParser::reset() {
  state_ = State::Dollar;
  offset_ = 0;
  checksum_ = 0;
  magic_ = 0;
}

void MspParser::consume(uint8_t value) {
  switch (state_) {
    case State::Dollar:
      if (value == '$') state_ = State::Magic;
      break;
    case State::Magic:
      if (value == 'M' || value == 'X') {
        magic_ = static_cast<char>(value);
        frame_ = {};
        frame_.version = value == 'M' ? MspVersion::V1 : MspVersion::V2Native;
        state_ = State::Direction;
      } else {
        reset();
      }
      break;
    case State::Direction:
      if (value == '>' || value == '!') {
        frame_.error = value == '!';
        checksum_ = 0;
        offset_ = 0;
        state_ = magic_ == 'M' ? State::V1Size : State::V2Flags;
      } else {
        reset();
      }
      break;
    case State::V1Size:
      frame_.payload_length = value;
      checksum_ ^= value;
      if (frame_.payload_length > sizeof(frame_.payload)) {
        failFrame();
      } else {
        state_ = State::V1Command;
      }
      break;
    case State::V1Command:
      frame_.command = value;
      checksum_ ^= value;
      state_ = frame_.payload_length == 0 ? State::V1Checksum
                                          : State::V1Payload;
      break;
    case State::V1Payload:
      frame_.payload[offset_++] = value;
      checksum_ ^= value;
      if (offset_ == frame_.payload_length) state_ = State::V1Checksum;
      break;
    case State::V1Checksum:
      value == checksum_ ? finishFrame() : failFrame();
      break;
    case State::V2Flags:
      frame_.flags = value;
      checksum_ = mspCrc8DvbS2(checksum_, value);
      state_ = State::V2CommandLow;
      break;
    case State::V2CommandLow:
      frame_.command = value;
      checksum_ = mspCrc8DvbS2(checksum_, value);
      state_ = State::V2CommandHigh;
      break;
    case State::V2CommandHigh:
      frame_.command |= static_cast<uint16_t>(value) << 8;
      checksum_ = mspCrc8DvbS2(checksum_, value);
      state_ = State::V2SizeLow;
      break;
    case State::V2SizeLow:
      frame_.payload_length = value;
      checksum_ = mspCrc8DvbS2(checksum_, value);
      state_ = State::V2SizeHigh;
      break;
    case State::V2SizeHigh:
      frame_.payload_length |= static_cast<uint16_t>(value) << 8;
      checksum_ = mspCrc8DvbS2(checksum_, value);
      if (frame_.payload_length > sizeof(frame_.payload)) {
        failFrame();
      } else {
        state_ = frame_.payload_length == 0 ? State::V2Checksum
                                            : State::V2Payload;
      }
      break;
    case State::V2Payload:
      frame_.payload[offset_++] = value;
      checksum_ = mspCrc8DvbS2(checksum_, value);
      if (offset_ == frame_.payload_length) state_ = State::V2Checksum;
      break;
    case State::V2Checksum:
      value == checksum_ ? finishFrame() : failFrame();
      break;
  }
}

uint32_t MspParser::validFrames() const { return valid_frames_; }
uint32_t MspParser::checksumErrors() const { return checksum_errors_; }

void MspParser::finishFrame() {
  valid_frames_++;
  if (handler_ != nullptr) handler_(frame_, context_);
  reset();
}

void MspParser::failFrame() {
  checksum_errors_++;
  reset();
}

uint8_t mspCrc8DvbS2(uint8_t crc, uint8_t value) {
  crc ^= value;
  for (uint8_t bit = 0; bit < 8; ++bit) {
    crc = crc & 0x80 ? static_cast<uint8_t>((crc << 1) ^ 0xD5)
                     : static_cast<uint8_t>(crc << 1);
  }
  return crc;
}

size_t writeMspRequest(Stream& stream, MspVersion version, uint16_t command) {
  if (version == MspVersion::V1) {
    if (command > 0xFF) return 0;
    const uint8_t frame[] = {'$', 'M', '<', 0, static_cast<uint8_t>(command),
                             static_cast<uint8_t>(command)};
    return stream.write(frame, sizeof(frame));
  }

  uint8_t frame[] = {'$', 'X', '<', 0,
                     static_cast<uint8_t>(command & 0xFF),
                     static_cast<uint8_t>(command >> 8), 0, 0, 0};
  uint8_t crc = 0;
  for (size_t index = 3; index < sizeof(frame) - 1; ++index) {
    crc = mspCrc8DvbS2(crc, frame[index]);
  }
  frame[sizeof(frame) - 1] = crc;
  return stream.write(frame, sizeof(frame));
}

bool runMspProtocolSelfTest() {
  struct Capture {
    uint8_t count = 0;
    uint16_t command = 0;
    uint8_t first_payload = 0;
  } capture;
  auto handler = [](const MspFrame& frame, void* context) {
    auto* result = static_cast<Capture*>(context);
    result->count++;
    result->command = frame.command;
    result->first_payload = frame.payload_length > 0 ? frame.payload[0] : 0;
  };

  MspParser parser;
  parser.setHandler(handler, &capture);
  const uint8_t v1[] = {'$', 'M', '>', 1, 106, 3, static_cast<uint8_t>(1 ^ 106 ^ 3)};
  for (uint8_t value : v1) parser.consume(value);
  if (capture.count != 1 || capture.command != 106 || capture.first_payload != 3)
    return false;

  uint8_t v2[] = {'$', 'X', '>', 0, 0x00, 0x20, 1, 0, 0xA5, 0};
  uint8_t crc = 0;
  for (size_t index = 3; index < sizeof(v2) - 1; ++index)
    crc = mspCrc8DvbS2(crc, v2[index]);
  v2[sizeof(v2) - 1] = crc;
  for (uint8_t value : v2) parser.consume(value);
  if (capture.count != 2 || capture.command != 0x2000 ||
      capture.first_payload != 0xA5)
    return false;

  v2[sizeof(v2) - 1] ^= 0x01;
  for (uint8_t value : v2) parser.consume(value);
  return capture.count == 2 && parser.validFrames() == 2 &&
         parser.checksumErrors() == 1;
}
