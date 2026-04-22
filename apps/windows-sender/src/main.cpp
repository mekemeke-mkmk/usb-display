#include <chrono>
#include <iostream>
#include <thread>

#include "sender_app.hpp"
#include "usb_display/telemetry.hpp"

namespace usb_display::sender {

SenderApp::SenderApp(RuntimeConfig config) : config_(std::move(config)) {}

int SenderApp::run() {
  std::cout << "[sender] bootstrap mode\n";
  std::cout << "[sender] target " << config_.host << ":" << config_.port << "\n";
  std::cout << "[sender] " << config_.width << "x" << config_.height
            << " @" << config_.fps_cap << "fps q=" << static_cast<int>(config_.jpeg_quality) << "\n";

  // Placeholder main loop for Phase 1 bring-up.
  TelemetrySnapshot telemetry{};
  constexpr int warmup_frames = 5;
  for (int i = 0; i < warmup_frames; ++i) {
    ++telemetry.frames_captured;
    std::this_thread::sleep_for(std::chrono::milliseconds(33));
    std::cout << "[sender] warmup frame " << (i + 1) << "/" << warmup_frames << "\n";
  }

  std::cout << "[sender] next: wire Desktop Duplication + JPEG encode + TCP send\n";
  return 0;
}

}  // namespace usb_display::sender

int main() {
  usb_display::RuntimeConfig config{};
  usb_display::sender::SenderApp app(config);
  return app.run();
}
