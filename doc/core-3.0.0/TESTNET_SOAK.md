# TestNet soak — Telestai Core 3.0.0 (`feature/core-3.0.0`)

**Do not merge to `master` until this soak passes.**

## Prerequisites

- Built binaries from this branch, e.g.:
  - `build-3.0.0/bin/telestaid`
  - `build-3.0.0/bin/telestai-cli`
- GPU miner: [tele-meraki-miner](https://github.com/Telestai-Project/tele-meraki-miner) (or compatible Meraki/ProgPoW miner)
- Fresh machine/user recommended so you do not touch mainnet `~/.telestai`

## 1. Dedicated TestNet datadir

```bash
DATADIR="$HOME/telestai-3.0.0-testnet"
mkdir -p "$DATADIR"

cat > "$DATADIR/telestai.conf" <<'EOF'
testnet=1
server=1
listen=1
txindex=1
rpcuser=telestai
rpcpassword=CHANGE_ME_STRONG
rpcallowip=127.0.0.1
rpcport=18766
port=18770
# optional while peers/seeds are thin:
# addnode=<known-3.0.0-testnet-peer>
EOF
```

Notes:

- Default TestNet P2P port is **18770**; RPC in conf above uses **18766** (set explicitly).
- 3.0.0 TestNet genesis/params were retargeted on this branch — **do not** reuse a 2.1.x TestNet datadir; start empty.
- DNS seeds are currently cleared; you may need `addnode` / `connect` to a peer running the same 3.0.0 TestNet build, or solo-mine a private TestNet.

## 2. Start the node

```bash
./build-3.0.0/bin/telestaid -datadir="$DATADIR" -daemon
./build-3.0.0/bin/telestai-cli -datadir="$DATADIR" -getinfo
./build-3.0.0/bin/telestai-cli -datadir="$DATADIR" getnetworkinfo
# expect subversion like /Telestai:3.0.0/
./build-3.0.0/bin/telestai-cli -datadir="$DATADIR" getblockchaininfo
```

Sync criteria (shared TestNet):

- `blocks` catches tip / headers
- peers > 0 (if a public TestNet peer exists)
- no continuous `invalid-meraki-epoch` / `bad-blk-height` spam

Private soak (no peers): skip sync; go straight to mining.

## 3. Wallet + mining address

```bash
./build-3.0.0/bin/telestai-cli -datadir="$DATADIR" createwallet soak
ADDR=$(./build-3.0.0/bin/telestai-cli -datadir="$DATADIR" getnewaddress)
echo "$ADDR"
```

Confirm development reward (25%) is configured:

```bash
./build-3.0.0/bin/telestai-cli -datadir="$DATADIR" getblocktemplate '{"rules":["segwit"]}' | head
# or inspect coinbase construction via getmininginfo / mined blocks
```

## 4. Meraki mine soak (solo RPC)

Point tele-meraki-miner at the node RPC (adjust path/binary for your OS):

```bash
# Example — see tele-meraki-miner release notes for exact flags
telemerakiminer -U -P http://telestai:CHANGE_ME_STRONG@127.0.0.1:18766/
```

Or generate locally (CPU, slow; fine for functional check on regtest more than TestNet):

```bash
# Regtest is faster for smoke; TestNet needs real Meraki work
./build-3.0.0/bin/telestai-cli -datadir="$DATADIR" generatetoaddress 1 "$ADDR"
```

Soak checklist after N blocks (e.g. 10–100):

- [ ] `getblockcount` increases
- [ ] Coinbase has miner payout + **25%** to TestNet development address `nVG96MbaKEDFzzj9NzbAuxkDt86KAm2Qj5`
- [ ] Asset issue fee (if tested) lands on that same development address (amounts unchanged)
- [ ] `getauxblock` / `createauxblock` / `submitauxblock` are **not** registered (AuxPoW out)
- [ ] Node stays stable; no Meraki epoch OOM from malformed headers
- [ ] `telestaid -version` shows Telestai copyright / GitHub URL (not Meowcoin)

## 5. Stop

```bash
./build-3.0.0/bin/telestai-cli -datadir="$DATADIR" stop
```

## Pass / fail gate

**Pass:** node builds, TestNet (or private TestNet) mines Meraki blocks, subsidy + asset-fee destination correct, AuxPoW RPCs absent, version branding clean.  
**Then:** open PR → `master` (still not force-merge).
