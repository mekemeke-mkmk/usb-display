#pragma once

#include <cstdint>

namespace usb_display {

struct TelemetrySnapshot {
  std::uint64_t frames_captured = 0;
  std::uint64_t frames_sent = 0;
  std::uint64_t frames_received = 0;
  std::uint64_t frames_rendered = 0;
  std::uint64_t frames_dropped = 0;
  std::uint64_t bytes_sent = 0;
  std::uint64_t bytes_received = 0;
};

}  // namespace usb_display
