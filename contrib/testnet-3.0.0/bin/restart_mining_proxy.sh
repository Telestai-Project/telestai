#!/usr/bin/env bash
# Restart the allowlist mining RPC proxy (picks up longpoll / HTTP/1.1 fixes).
set -euo pipefail

ROOT=/home/chief/telestai
CONF="$ROOT/testnet-3.0.0/telestai.conf"
BIN="$ROOT/testnet-3.0.0/bin"
WATCH="$ROOT/testnet-3.0.0/watch"
PROXY_LOG="$WATCH/proxy.log"
PIDF="$WATCH/proxy.pid"

mkdir -p "$WATCH"

rpcuser=$(grep '^rpcuser=' "$CONF" | cut -d= -f2)
rpcpassword=$(grep '^rpcpassword=' "$CONF" | cut -d= -f2)

pkill -f 'mining_rpc_proxy.py' 2>/dev/null || true
sleep 1

export TLS_UPSTREAM=http://127.0.0.1:18766
export TLS_PROXY_LISTEN=0.0.0.0
export TLS_PROXY_PORT=18768
export TLS_RPC_USER="$rpcuser"
export TLS_RPC_PASS="$rpcpassword"
export TLS_PROXY_TIMEOUT=60
export TLS_PROXY_LONGPOLL_TIMEOUT=300

nohup python3 "$BIN/mining_rpc_proxy.py" >>"$PROXY_LOG" 2>&1 &
echo $! >"$PIDF"
echo "proxy=$(cat "$PIDF") listen=${TLS_PROXY_LISTEN}:${TLS_PROXY_PORT}"
