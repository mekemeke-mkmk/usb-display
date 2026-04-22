#pragma once

#include "usb_display/runtime_config.hpp"

namespace usb_display::receiver {

class ReceiverApp {
 public:
  explicit ReceiverApp(RuntimeConfig config);
  int run();

 private:
  RuntimeConfig config_;
};

}  // namespace usb_display::receiver
