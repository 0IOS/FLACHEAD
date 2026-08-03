#!/bin/sh
# pi-audit.sh — generate a FLACHEAD resource report on the target Pi.
#
# Run ON the Raspberry Pi Zero W (not the desktop). It measures the four
# numbers that matter: startup, FPS/frame time, idle CPU, peak RAM, plus the
# binary size. Produces a timestamped report under reports/.
#
# Usage:
#   ./tools/pi-audit.sh [binary] [seconds]
#
# Requires: /usr/bin/time, pgrep. On Raspberry Pi OS Lite these are present.
set -eu

BINARY="${1:-./build/FLACHEAD}"
SECONDS="${2:-15}"
REPORT_DIR="reports"
mkdir -p "$REPORT_DIR"
REPORT="$REPORT_DIR/audit-$(date +%Y%m%d-%H%M%S).txt"

note() { printf '%s\n' "$@" | tee -a "$REPORT"; }

note "FLACHEAD audit — $(date)"
note "Device     : $(grep -i 'model name' /proc/cpuinfo | head -1)"
note "OS         : $(grep PRETTY_NAME /etc/os-release)"
note "Binary     : $BINARY"

if [ ! -x "$BINARY" ]; then
    note "ERROR: $BINARY not found or not executable"
    exit 1
fi

BINARY_SIZE=$(stat -c %s "$BINARY" 2>/dev/null || stat -f %z "$BINARY")
note "Binary size: $BINARY_SIZE bytes ($((BINARY_SIZE / 1024)) KB)"

note
note "--- Startup + frame timing ($SECONDS s forced render) ---"
"$BINARY" --benchmark="$SECONDS" 2>&1 | tee -a "$REPORT"

note
note "--- Peak RAM (/usr/bin/time) ---"
/usr/bin/time -v "$BINARY" --benchmark=5 2>&1 | grep -E "Maximum resident|Elapsed" | tee -a "$REPORT"

note
note "--- Idle CPU + RSS while sitting on the launcher (30 s) ---"
"$BINARY" > /dev/null 2>&1 &
APP_PID=$!
sleep 3
note "pid: $APP_PID  pcpu:rss:"
pidstat -p "$APP_PID" 1 30 2>/dev/null | tee -a "$REPORT" \
    || ps -o pid,pcpu,rss,cmd -p "$APP_PID" | tee -a "$REPORT"
kill "$APP_PID"

note
note "Report written to $REPORT"
