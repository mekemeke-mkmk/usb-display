const steps = [
  "Choose the Windows MVP implementation stack: C++ or Rust.",
  "Implement sender capture for a fixed 1280x720 region.",
  "Implement the Frame Protocol v1 TCP transport.",
  "Implement receiver decode and immediate presentation.",
  "Measure latency, payload size, and frame drops before optimization."
];

console.log("usb-display next plan");
for (const [index, step] of steps.entries()) {
  console.log(`${index + 1}. ${step}`);
}
