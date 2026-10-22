#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
mode="${1:-strict}"
cc="${CC:-cc}"
sources="core-src/src/memory.c core-src/src/bus.c core-src/src/cpu.c core-src/src/state.c core-src/src/covers.c core-src/tests/test_core_host.c"
case "$mode" in
  strict)
    "$cc" -std=c99 -Wall -Wextra -Werror -O2 -Icore-src/include $sources -o /tmp/cubex360-core-host-test
    /tmp/cubex360-core-host-test
    ;;
  sanitize)
    "$cc" -std=c99 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Icore-src/include $sources -o /tmp/cubex360-core-sanitizers
    /tmp/cubex360-core-sanitizers
    ;;
  *) echo "Usage: $0 [strict|sanitize]" >&2; exit 2 ;;
esac
