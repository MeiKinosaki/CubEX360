#!/bin/sh
set -eu
printf '%s\n' 'CubEX360 XEX preflight'
printf '%s\n' 'This script does not convert ELF to XEX.'
if [ -z "${XEX_CC:-}" ] || [ -z "${XEX_PACKAGER:-}" ]; then
  cat >&2 <<'MSG'
BLOCKED: set XEX_CC and XEX_PACKAGER only after selecting and validating
real tools compatible with the Xbox 360 XEX format/runtime.
The libXenon toolchain validated here produces ELF32, not default.xex.
Do not rename ELF or publish an unvalidated XEX.
MSG
  exit 2
fi
command -v "$XEX_CC" >/dev/null 2>&1 || { echo "ERROR: XEX compiler not found: $XEX_CC" >&2; exit 2; }
command -v "$XEX_PACKAGER" >/dev/null 2>&1 || { echo "ERROR: XEX packager/validator not found: $XEX_PACKAGER" >&2; exit 2; }
printf '%s\n' 'Tools are present; ABI, imports, output structure, and hardware behavior still require validation.'
