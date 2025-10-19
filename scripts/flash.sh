#!/usr/bin/env bash

# Simple helper to copy a UF2 artifact to a mounted Pico bootloader volume.

set -euo pipefail

UF2_PATH="${1:-build/my_project.uf2}"
TARGET_PATH="${2:-}"

if [[ ! -f "${UF2_PATH}" ]]; then
  echo "error: UF2 file '${UF2_PATH}' not found." >&2
  exit 1
fi

if [[ -z "${TARGET_PATH}" ]]; then
  if [[ -n "${PICO_MOUNT:-}" && -d "${PICO_MOUNT}" ]]; then
    TARGET_PATH="${PICO_MOUNT}"
  else
    declare -a candidates=(
      "/media/${USER}/RPI-RP2"
      "/run/media/${USER}/RPI-RP2"
      "/media/${USER}/RPI-RP2350"
      "/run/media/${USER}/RPI-RP2350"
      "/Volumes/RPI-RP2"
      "/mnt/RPI-RP2"
    )
    for cand in "${candidates[@]}"; do
      if [[ -d "${cand}" ]]; then
        TARGET_PATH="${cand}"
        break
      fi
    done
  fi
fi

if [[ -z "${TARGET_PATH}" ]]; then
  echo "error: no Pico mass-storage volume found. Pass the mount point as the second argument or set PICO_MOUNT." >&2
  exit 2
fi

if [[ ! -d "${TARGET_PATH}" ]]; then
  echo "error: target mount '${TARGET_PATH}' does not exist or is not a directory." >&2
  exit 3
fi

echo "Flashing '${UF2_PATH}' to '${TARGET_PATH}'..."
cp -v "${UF2_PATH}" "${TARGET_PATH}/"
sync
echo "Done."

