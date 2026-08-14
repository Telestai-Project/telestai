#!/usr/bin/env bash
# Fast tip watcher: restart miner within ~10–15s of a tip advance (cron is too slow).
# Complements miner_watchdog.sh; keeps telemerakiminer on fresh GBT jobs.
set -euo pipefail

ROOT=/home/chief/telestai
WATCH="$ROOT/testnet-3.0.0/watch"
BIN="$ROOT/testnet-3.0.0/bin"
START="$BIN/start_testnet_miner.sh"
TIPF="$WATCH/last_tip.height"
LOG="$WATCH/tip_refresh.log"
PIDF="$WATCH/tip_refresh.pid"
COOLDOWNF="$WATCH/tip_refresh_cooldown"
INTERVAL_S="${TLS_TIP_REFRESH_INTERVAL_S:-10}"
COOLDOWN_S="${TLS_TIP_REFRESH_COOLDOWN_S:-20}"

mkdir -p "$WATCH"
echo $$ >"$PIDF"
ts() { date -u +%Y-%m-%dT%H:%M:%SZ; }
log() { echo "$(ts) $*" | tee -a "$LOG"; }

current_tip() {
  export LD_LIBRARY_PATH="$ROOT/lib-3.0.0${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
  "$ROOT/build-out/3.0.0/bin/telestai-cli" -testnet -conf="$ROOT/testnet-3.0.0/telestai.conf" getblockcount 2>/dev/null || echo ""
}

in_cooldown() {
  [[ -f "$COOLDOWNF" ]] || return 1
  local last now
  last=$(cat "$COOLDOWNF" 2>/dev/null || echo 0)
  now=$(date +%s)
  (( now - last < COOLDOWN_S ))
}

log "tip_refresh_daemon start interval=${INTERVAL_S}s"
prev=$(current_tip)
[[ -n "$prev" ]] && echo "$prev" >"$TIPF"

while true; do
  sleep "$INTERVAL_S"
  tip=$(current_tip)
  [[ -n "$tip" && "$tip" =~ ^[0-9]+$ ]] || continue
  if [[ -n "$prev" && "$prev" =~ ^[0-9]+$ ]] && (( tip > prev )); then
    echo "$tip" >"$TIPF"
    date +%s >"$WATCH/tip_advanced_at"
    if in_cooldown; then
      log "tip_advanced ${prev}->${tip} (skip restart cooldown)"
    else
      log "tip_advanced ${prev}->${tip} RESTART miner"
      date +%s >"$COOLDOWNF"
      "$START" >>"$LOG" 2>&1 || log "start_testnet_miner.sh failed"
    fi
  fi
  prev=$tip
  echo "$tip" >"$TIPF"
done
