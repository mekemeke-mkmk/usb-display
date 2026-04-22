#!/bin/sh

set -eu

check_cmd() {
  if command -v "$1" >/dev/null 2>&1; then
    printf "[ok] %s: %s\n" "$1" "$(command -v "$1")"
  else
    printf "[missing] %s\n" "$1"
  fi
}

printf "usb-display bootstrap environment check\n"
printf "workspace: %s\n" "$(pwd)"

check_cmd git
check_cmd node
check_cmd clang
check_cmd cmake
check_cmd cargo
check_cmd rustc
check_cmd powershell
check_cmd pwsh

printf "\nRecommended MVP baseline:\n"
printf "%s\n" "- Windows development machine for capture and render bring-up"
printf "%s\n" "- USB virtual LAN path prepared between sender and receiver"
printf "%s\n" "- Native toolchain chosen before Phase 1 implementation starts"
