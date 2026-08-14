#!/usr/bin/env bash
# Keep TestNet miner alive: restart on death, OOM-risk RSS, huge logs, tip advance, or stale job.
set -euo pipefail

ROOT=/home/chief/telestai
WATCH="$ROOT/testnet-3.0.0/watch"
BIN="$ROOT/testnet-3.0.0/bin"
PIDF="$WATCH/miner.pid"
LOG="$WATCH/miner.log"
WDLOG="$WATCH/watchdog.log"
TIPF="$WATCH/last_tip.height"
COOLDOWNF="$WATCH/watchdog_cooldown"
START="$BIN/start_testnet_miner.sh"

MAX_RSS_KB="${TLS_MINER_MAX_RSS_KB:-8388608}"       # 8 GiB
MAX_LOG_BYTES="${TLS_MINER_MAX_LOG_BYTES:-52428800}" # 50 MiB
# Short cooldown so tip-advance restarts can keep pace; long enough to avoid thrash.
RESTART_COOLDOWN_S="${TLS_MINER_RESTART_COOLDOWN_S:-45}"
STALE_GRACE_S="${TLS_MINER_STALE_GRACE_S:-60}"

mkdir -p "$WATCH"
ts() { date -u +%Y-%m-%dT%H:%M:%SZ; }
log() { echo "$(ts) $*" | tee -a "$WDLOG"; }

in_cooldown() {
  [[ -f "$COOLDOWNF" ]] || return 1
  local last now
  last=$(cat "$COOLDOWNF" 2>/dev/null || echo 0)
  now=$(date +%s)
  (( now - last < RESTART_COOLDOWN_S ))
}

mark_cooldown() { date +%s >"$COOLDOWNF"; }

miner_pid() {
  if [[ -f "$PIDF" ]]; then
    local p
    p=$(cat "$PIDF" 2>/dev/null || true)
    if [[ -n "$p" ]] && kill -0 "$p" 2>/dev/null; then
      echo "$p"
      return 0
    fi
  fi
  pgrep -n -f '/home/chief/telestai/miner/telemerakiminer' || true
}

rss_kb() {
  local p=$1
  awk '/^VmRSS:/ {print $2; exit}' "/proc/$p/status" 2>/dev/null || echo 0
}

current_tip() {
  export LD_LIBRARY_PATH="$ROOT/lib-3.0.0${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
  "$ROOT/build-out/3.0.0/bin/telestai-cli" -testnet -conf="$ROOT/testnet-3.0.0/telestai.conf" getblockcount 2>/dev/null || echo ""
}

# telemerakiminer prints "Job: … block N" where N is the height being mined (tip+1).
latest_job_height() {
  [[ -f "$LOG" ]] || { echo ""; return; }
  tail -c 204800 "$LOG" 2>/dev/null \
    | grep -aEo 'block [0-9]+' \
    | tail -1 \
    | awk '{print $2}'
}

rotate_log_if_needed() {
  [[ -f "$LOG" ]] || return 0
  local sz
  sz=$(stat -c%s "$LOG" 2>/dev/null || echo 0)
  if (( sz > MAX_LOG_BYTES )); then
    local dest="$LOG.$(ts).old"
    mv -f "$LOG" "$dest"
    : >"$LOG"
    ls -1t "$LOG".*.old 2>/dev/null | tail -n +4 | xargs -r rm -f
    log "rotated miner.log size=${sz} -> $(basename "$dest")"
  fi
}

restart_miner() {
  local reason=$1
  if in_cooldown; then
    log "skip restart ($reason) — cooldown"
    return 0
  fi
  log "RESTART reason=$reason"
  mark_cooldown
  "$START" >>"$WDLOG" 2>&1 || log "start_testnet_miner.sh failed"
}

rotate_log_if_needed

pid=$(miner_pid)
if [[ -z "${pid}" ]]; then
  if ! pgrep -f 'mining_rpc_proxy.py' >/dev/null 2>&1; then
    log "proxy down — restart_mining_proxy.sh"
    "$BIN/restart_mining_proxy.sh" >>"$WDLOG" 2>&1 || true
    sleep 2
  fi
  restart_miner "miner_dead"
  exit 0
fi

rss=$(rss_kb "$pid")
if (( rss > MAX_RSS_KB )); then
  restart_miner "rss_kb=${rss}>${MAX_RSS_KB}"
  exit 0
fi

tip=$(current_tip)
job_h=$(latest_job_height)

if [[ -n "$tip" && "$tip" =~ ^[0-9]+$ ]]; then
  prev=""
  [[ -f "$TIPF" ]] && prev=$(cat "$TIPF" 2>/dev/null || true)
  if [[ -n "$prev" && "$prev" =~ ^[0-9]+$ ]] && (( tip > prev )); then
    echo "$tip" >"$TIPF"
    date +%s >"$WATCH/tip_advanced_at"
    # telemerakiminer does not reliably longpoll; force a fresh job after each find.
    restart_miner "tip_advanced ${prev}->${tip}"
    exit 0
  fi
  echo "$tip" >"$TIPF"

  # Stale job: still hashing height <= tip (should be tip+1)
  if [[ -n "$job_h" && "$job_h" =~ ^[0-9]+$ ]] && (( job_h <= tip )); then
    now=$(date +%s)
    advanced_at=$(cat "$WATCH/tip_advanced_at" 2>/dev/null || echo 0)
    started_at=$(date -u -d "$(cat "$WATCH/miner_started_at" 2>/dev/null || echo 1970-01-01)" +%s 2>/dev/null || echo 0)
    if (( now - advanced_at >= STALE_GRACE_S && now - started_at >= STALE_GRACE_S )); then
      restart_miner "stale_job job=${job_h} tip=${tip}"
      exit 0
    fi
  fi
fi

avail_kb=$(df -Pk "$WATCH" | awk 'NR==2{print $4}')
if [[ -n "${avail_kb}" ]] && (( avail_kb < 5242880 )); then
  log "ALERT low_disk_avail_kb=${avail_kb}"
fi

log "ok pid=$pid rss_kb=$rss tip=${tip:-?} job=${job_h:-?} log_bytes=$(stat -c%s "$LOG" 2>/dev/null || echo 0)"
