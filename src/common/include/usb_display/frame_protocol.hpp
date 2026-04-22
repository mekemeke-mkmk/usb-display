#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace usb_display {

inline constexpr std::array<char, 4> kMagic = {'U', 'S', 'B', 'D'};
inline constexpr std::uint16_t kProtocolVersion = 1;

enum class Codec : std::uint16_t {
  kJpeg = 1,
};

struct FrameHeader {
  std::uint64_t frame_id = 0;
  std::uint64_t timestamp_us = 0;
  std::uint16_t width = 1280;
  std::uint16_t height = 720;
  Codec codec = Codec::kJpeg;
  std::uint16_t flags = 0;
  std::uint32_t payload_len = 0;
};

struct FramePacket {
  FrameHeader header;
  std::vector<std::uint8_t> payload;
};

struct ParseResult {
  bool ok = false;
  const char* error = "";
};

// Phase 1 intentionally keeps protocol code simple and allocation-friendly.
// Performance work can replace this with pooled buffers later.
ParseResult validate_packet(const FramePacket& packet);

}  // namespace usb_display
