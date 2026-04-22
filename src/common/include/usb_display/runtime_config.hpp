#pragma once

#include <cstdint>
#include <string>

namespace usb_display {

struct RuntimeConfig {
  std::string host = "127.0.0.1";
  std::uint16_t port = 45888;
  std::uint16_t width = 1280;
  std::uint16_t height = 720;
  std::uint16_t fps_cap = 30;
  std::uint8_t jpeg_quality = 70;
};

}  // namespace usb_display
