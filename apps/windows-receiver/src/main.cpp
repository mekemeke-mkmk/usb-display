#include <chrono>
#include <iostream>
#include <thread>

#include "receiver_app.hpp"
#include "usb_display/telemetry.hpp"

namespace usb_display::receiver {

ReceiverApp::ReceiverApp(RuntimeConfig config) : config_(std::move(config)) {}

int ReceiverApp::run() {
  std::cout << "[receiver] bootstrap mode\n";
  std::cout << "[receiver] listen : " << config_.port << "\n";
  std::cout << "[receiver] expecting " << config_.width << "x" << config_.height << " JPEG\n";

  // Placeholder main loop for Phase 1 bring-up.
  TelemetrySnapshot telemetry{};
  constexpr int warmup_frames = 5;
  for (int i = 0; i < warmup_frames; ++i) {
    ++telemetry.frames_rendered;
    std::this_thread::sleep_for(std::chrono::milliseconds(33));
    std::cout << "[receiver] render placeholder frame " << (i + 1) << "/" << warmup_frames << "\n";
  }

  std::cout << "[receiver] next: wire TCP receive + JPEG decode + present\n";
  return 0;
}

}  // namespace usb_display::receiver

int main() {
  usb_display::RuntimeConfig config{};
  usb_display::receiver::ReceiverApp app(config);
  return app.run();
}
