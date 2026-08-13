#!/usr/bin/env bash
# Start cooled TestNet Meraki miner + thermal governor.
# Filters Sol: spam from miner.log while keeping the real miner PID for SIGSTOP/CONT.
set -euo pipefail

ROOT=/home/chief/telestai
export LD_LIBRARY_PATH="$ROOT/lib-3.0.0:$ROOT/miner/cuda-libs${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

POOL="${TLS_MINER_POOL:-http://tls_ext:1EBOLmggLkXEUjbwUAw5AOBk@127.0.0.1:18768/}"
WATCH="$ROOT/testnet-3.0.0/watch"
LOG="$WATCH/miner.log"
PIDF="$WATCH/miner.pid"
GOVPIDF="$WATCH/thermal_governor.pid"
FIFO="$WATCH/miner.fifo"

mkdir -p "$WATCH"
"$ROOT/testnet-3.0.0/bin/apply_gpu_power.sh" || true

# Stop previous miner/governor/log filter
if [[ -f "$PIDF" ]]; then
  old=$(cat "$PIDF" || true)
  if [[ -n "${old}" ]]; then
    kill -CONT "$old" 2>/dev/null || true
    kill -TERM "$old" 2>/dev/null || true
  fi
fi
pkill -9 -f '/home/chief/telestai/miner/telemerakiminer' 2>/dev/null || true
pkill -9 -f 'gpu_thermal_governor.sh' 2>/dev/null || true
pkill -9 -f 'testnet-3.0.0/watch/miner.fifo' 2>/dev/null || true
pkill -9 -f 'grep -Ev .+Sol:' 2>/dev/null || true
sleep 2

# Rotate previous log if large
if [[ -f "$LOG" ]]; then
  sz=$(stat -c%s "$LOG" 2>/dev/null || echo 0)
  if (( sz > 10485760 )); then
    mv -f "$LOG" "$LOG.$(date -u +%Y%m%dT%H%M%SZ).old" 2>/dev/null || true
    # keep at most 3 rotated logs
    ls -1t "$LOG".*.old 2>/dev/null | tail -n +4 | xargs -r rm -f
  fi
fi
: >"$LOG"

rm -f "$FIFO"
mkfifo "$FIFO"

# Log filter: drop ethash Sol spam, keep Accepted/Rejected/Job/errors
nohup bash -c "grep -Eav --line-buffered 'Sol:|found in' <\"$FIFO\" >>\"$LOG\"" >/dev/null 2>&1 &

nohup "$ROOT/miner/telemerakiminer" -U \
  --cu-devices 1 \
  --cu-streams 1 \
  --cu-grid-size 48 \
  --cu-block-size 128 \
  --cu-parallel-hash 1 \
  --HWMON 2 \
  -P "$POOL" >"$FIFO" 2>&1 &
echo $! >"$PIDF"

nohup "$ROOT/testnet-3.0.0/bin/gpu_thermal_governor.sh" >>"$WATCH/thermal_governor.log" 2>&1 &
echo $! >"$GOVPIDF"

# Record tip at start so watchdog can detect advances for job-refresh restarts
export LD_LIBRARY_PATH="$ROOT/lib-3.0.0${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
CLI="$ROOT/build-out/3.0.0/bin/telestai-cli"
CONF="$ROOT/testnet-3.0.0/telestai.conf"
tip=$("$CLI" -testnet -conf="$CONF" getblockcount 2>/dev/null || echo 0)
echo "$tip" >"$WATCH/last_tip.height"
date -u +%Y-%m-%dT%H:%M:%SZ >"$WATCH/miner_started_at"

echo "miner=$(cat "$PIDF") governor=$(cat "$GOVPIDF") tip=$tip"
