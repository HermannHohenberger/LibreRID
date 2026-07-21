#pragma once

#include <Arduino.h>

enum class MspVersion : uint8_t { V1, V2Native };

struct MspFrame {
  MspVersion version = MspVersion::V1;
  uint8_t flags = 0;
  uint16_t command = 0;
  uint16_t payload_length = 0;
  uint8_t payload[128] = {};
  bool error = false;
};

using MspFrameHandler = void (*)(const MspFrame& frame, void* context);

class MspParser {
 public:
  void setHandler(MspFrameHandler handler, void* context);
  void reset();
  void consume(uint8_t value);
  uint32_t validFrames() const;
  uint32_t checksumErrors() const;

 private:
  enum class State : uint8_t {
    Dollar,
    Magic,
    Direction,
    V1Size,
    V1Command,
    V1Payload,
    V1Checksum,
    V2Flags,
    V2CommandLow,
    V2CommandHigh,
    V2SizeLow,
    V2SizeHigh,
    V2Payload,
    V2Checksum,
  };

  void finishFrame();
  void failFrame();

  State state_ = State::Dollar;
  MspFrame frame_;
  uint16_t offset_ = 0;
  uint8_t checksum_ = 0;
  char magic_ = 0;
  MspFrameHandler handler_ = nullptr;
  void* context_ = nullptr;
  uint32_t valid_frames_ = 0;
  uint32_t checksum_errors_ = 0;
};

uint8_t mspCrc8DvbS2(uint8_t crc, uint8_t value);
size_t writeMspRequest(Stream& stream, MspVersion version, uint16_t command);
bool runMspProtocolSelfTest();
