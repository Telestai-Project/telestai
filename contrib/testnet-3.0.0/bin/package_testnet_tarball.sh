#!/usr/bin/env bash
# Build a Linux x86_64 TestNet soak tarball from Optimus build-out.
set -euo pipefail
ROOT="${ROOT:-/home/chief/telestai}"
OUT_DIR="${OUT_DIR:-$ROOT/releases}"
STAMP=$(date -u +%Y%m%d)
NAME="telestai-3.0.0-testnet-linux-x86_64"
STAGE="$OUT_DIR/$NAME"

rm -rf "$STAGE"
mkdir -p "$STAGE/bin" "$STAGE/lib" "$STAGE/docs"
cp -a "$ROOT/build-out/3.0.0/bin/telestaid" "$ROOT/build-out/3.0.0/bin/telestai-cli" "$STAGE/bin/"
cp -a "$ROOT/lib-3.0.0/." "$STAGE/lib/"

cat > "$STAGE/README.txt" <<'EOF'
Telestai Core 3.0.0 — TestNet soak binaries (Linux x86_64)

See docs/PEER_JOIN.md for independent peer setup.
See docs/POOL_OPERATORS.md for mining notes.

Quick start:
  export LD_LIBRARY_PATH="$PWD/lib:${LD_LIBRARY_PATH:-}"
  mkdir -p ~/telestai-3.0.0-testnet
  # write telestai.conf with testnet=1 and addnode=114.73.210.115:18770
  ./bin/telestaid -datadir=$HOME/telestai-3.0.0-testnet -daemon

Miner: tele-meraki-miner 1.5.0+ only (not MiniZ).
EOF

for d in PEER_JOIN.md POOL_OPERATORS.md EXTERNAL_TESTNET_MINING.md; do
  if [[ -f "$ROOT/src-3.0.0/doc/core-3.0.0/$d" ]]; then
    cp -a "$ROOT/src-3.0.0/doc/core-3.0.0/$d" "$STAGE/docs/"
  fi
done

export LD_LIBRARY_PATH="$STAGE/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
"$STAGE/bin/telestaid" -version | head -5 > "$STAGE/VERSION.txt"

(
  cd "$OUT_DIR"
  tar -czf "${NAME}-${STAMP}.tar.gz" "$NAME"
  sha256sum "${NAME}-${STAMP}.tar.gz" > "${NAME}-${STAMP}.tar.gz.sha256"
  ls -la "${NAME}-${STAMP}.tar.gz" "${NAME}-${STAMP}.tar.gz.sha256"
  cat "${NAME}-${STAMP}.tar.gz.sha256"
)
echo "PACKAGED=$OUT_DIR/${NAME}-${STAMP}.tar.gz"
