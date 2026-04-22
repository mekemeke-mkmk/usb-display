import { readFile } from "node:fs/promises";

const files = [
  "README.md",
  "docs/architecture.md",
  "docs/mvp-spec.md",
  "docs/roadmap.md",
  "docs/windows-prototype-plan.md",
  "proto/frame-v1.md"
];

let failed = false;

for (const file of files) {
  const body = await readFile(new URL(`../${file}`, import.meta.url), "utf8");
  if (!body.trim()) {
    console.error(`[fail] ${file} is empty`);
    failed = true;
    continue;
  }
  console.log(`[ok] ${file}`);
}

if (failed) {
  process.exitCode = 1;
}
