#!/usr/bin/env bash
# Snapshot TestNet tip + append only *new* debug.log error lines (byte-offset based).
set -euo pipefail

ROOT=/home/chief/telestai
export LD_LIBRARY_PATH="$ROOT/lib-3.0.0${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
CLI="$ROOT/build-out/3.0.0/bin/telestai-cli"
CONF="$ROOT/testnet-3.0.0/telestai.conf"
WATCH="$ROOT/testnet-3.0.0/watch"
OUT="$WATCH/status.json"
ERR="$WATCH/errors.log"
LOG="$ROOT/testnet-3.0.0/data/testnet3/debug.log"
OFFSETF="$WATCH/debug.log.offset"
PAT='\[error\]|ERROR|bad-cb|mix_hash|Reject|Invalid Telestai|Assert|exception'

mkdir -p "$WATCH"
ts=$(date -u +%Y-%m-%dT%H:%M:%SZ)

if ! info=$("$CLI" -testnet -conf="$CONF" getblockchaininfo 2>/dev/null); then
  echo "{\"ts\":\"$ts\",\"ok\":false,\"error\":\"rpc_down\"}" > "$OUT"
  echo "$ts rpc_down" >> "$ERR"
  exit 0
fi
mining=$("$CLI" -testnet -conf="$CONF" getmininginfo 2>/dev/null || echo '{}')
peers=$("$CLI" -testnet -conf="$CONF" getconnectioncount 2>/dev/null || echo 0)

python3 - <<PY
import json
from pathlib import Path
info=json.loads('''$info''')
mining=json.loads('''$mining''')
out={
  "ts":"$ts",
  "ok":True,
  "blocks":info.get("blocks"),
  "headers":info.get("headers"),
  "bestblockhash":info.get("bestblockhash"),
  "difficulty":info.get("difficulty"),
  "peers": int("$peers"),
  "networkhashps": mining.get("networkhashps"),
}
Path("$OUT").write_text(json.dumps(out, indent=2)+"\n")
print(json.dumps(out))
PY

# --- New-only error scrape via byte offset ---
if [[ -f "$LOG" ]]; then
  size=$(stat -c%s "$LOG")
  offset=0
  if [[ -f "$OFFSETF" ]]; then
    offset=$(cat "$OFFSETF" 2>/dev/null || echo 0)
  fi
  # Log rotated/truncated
  if ! [[ "$offset" =~ ^[0-9]+$ ]] || (( offset > size )); then
    offset=0
  fi

  if (( size > offset )); then
    # Read only the new suffix
    dd if="$LOG" bs=1 skip="$offset" count=$((size - offset)) 2>/dev/null \
      | grep -E "$PAT" \
      | while IFS= read -r line; do
          echo "$ts $line" >> "$ERR"
        done || true
  fi
  echo "$size" >"$OFFSETF"

  # Rolling snapshot of recent matches (for humans), not appended forever
  grep -E "$PAT" "$LOG" | tail -30 > "$WATCH/debug_errors_tail.txt" || true
fi
