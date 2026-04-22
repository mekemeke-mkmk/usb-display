#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <span>
#include <vector>

#include "usb_display/frame_protocol.hpp"

namespace usb_display {

class FrameStreamParser {
 public:
  explicit FrameStreamParser(std::size_t max_frame_bytes = 4 * 1024 * 1024);

  ParseResult push_bytes(std::span<const std::uint8_t> bytes);
  bool pop_packet(FramePacket* out_packet);

 private:
  std::size_t max_frame_bytes_;
  std::vector<std::uint8_t> buffer_;
  std::deque<FramePacket> parsed_packets_;
};

}  // namespace usb_display
