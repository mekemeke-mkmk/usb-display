#pragma once

#include "usb_display/runtime_config.hpp"

namespace usb_display::sender {

class SenderApp {
 public:
  explicit SenderApp(RuntimeConfig config);
  int run();

 private:
  RuntimeConfig config_;
};

}  // namespace usb_display::sender
