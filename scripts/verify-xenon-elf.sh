#!/bin/sh
set -eu
artifact=${1:-xenon-probe/cubex360_toolchain_probe.elf32}
if [ ! -s "$artifact" ]; then
  echo "ERROR: ELF missing or empty: $artifact" >&2
  exit 2
fi
command -v file >/dev/null 2>&1 || { echo "ERROR: file utility required" >&2; exit 2; }
info=$(file "$artifact")
printf '%s\n' "$info"
printf '%s\n' "$info" | grep -E 'ELF 32-bit.*(MSB|big-endian).*PowerPC' >/dev/null || {
  echo "ERROR: artifact is not identified as big-endian PowerPC ELF32" >&2
  exit 3
}
if command -v sha256sum >/dev/null 2>&1; then sha256sum "$artifact"
elif command -v shasum >/dev/null 2>&1; then shasum -a 256 "$artifact"
else echo "WARNING: no SHA-256 utility found" >&2; exit 2
fi
