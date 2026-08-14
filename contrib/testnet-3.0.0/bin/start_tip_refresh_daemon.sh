#!/usr/bin/env bash
# Start / restart the tip_refresh_daemon (fast miner job refresh).
set -euo pipefail
ROOT=/home/chief/telestai
WATCH="$ROOT/testnet-3.0.0/watch"
BIN="$ROOT/testnet-3.0.0/bin"
PIDF="$WATCH/tip_refresh.pid"
mkdir -p "$WATCH"
if [[ -f "$PIDF" ]]; then
  old=$(cat "$PIDF" || true)
  if [[ -n "${old}" ]] && kill -0 "$old" 2>/dev/null; then
    kill "$old" 2>/dev/null || true
    sleep 1
  fi
fi
# Match only the daemon script path (not start_tip_refresh_daemon.sh).
pkill -f '/tip_refresh_daemon\.sh' 2>/dev/null || true
sleep 1
nohup "$BIN/tip_refresh_daemon.sh" >>"$WATCH/tip_refresh.log" 2>&1 &
disown || true
echo $! >"$PIDF"
echo "tip_refresh_daemon pid=$(cat "$PIDF")"
