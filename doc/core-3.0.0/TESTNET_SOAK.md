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

`telemerakiminer` uses Telestai’s Meraki GBT extensions (`pprpcheader` / `pprpcepoch`) and submits via `pprpcsb` (not BIP22 `submitblock`). Set `-miningaddress=` (legacy P2PKH) so templates include a coinbase the miner can seal.

```bash
# Example — see tele-meraki-miner release notes for exact flags
# Requires libnvrtc (CUDA 12) on PATH / LD_LIBRARY_PATH for the GPU build
telemerakiminer -U -P http://telestai:CHANGE_ME_STRONG@127.0.0.1:18766/
```

Optimus private soak paths (leave mainnet `:8766` alone):

- Binaries: `/home/chief/telestai/build-out/3.0.0/bin/`
- Conf/datadir: `/home/chief/telestai/testnet-3.0.0/`
- Miner: `/home/chief/telestai/miner/telemerakiminer` with `LD_LIBRARY_PATH=.../miner/cuda-libs`

Or generate locally (CPU, slow; fine for functional check on regtest more than TestNet):

```bash
# Regtest is faster for smoke; TestNet needs real Meraki work
./build-3.0.0/bin/telestai-cli -datadir="$DATADIR" generatetoaddress 1 "$ADDR"
```

Soak checklist after N blocks (e.g. 10–100):

- [ ] `getblockcount` increases
- [ ] Coinbase has miner payout + **25%** to TestNet development address `mgaw88zztsHWN8SyL9vXwiCed7aRiPwL26`
- [ ] Asset issue fee (if tested) lands on that same development address (amounts unchanged)
- [ ] `getauxblock` / `createauxblock` / `submitauxblock` are **not** registered (AuxPoW out)
- [ ] Node stays stable; no Meraki epoch OOM from malformed headers
- [ ] `telestaid -version` shows Telestai copyright / GitHub URL (not Telestai)
- [ ] After an Accept, the next GBT `pprpcheader` is **new** (no 30s reuse across tip changes)
- [ ] Stale `pprpcsb` attempts error out (not string `"inconclusive"` that MiniZ/telemerakiminer may treat as Accepted)
- [ ] Use **telemerakiminer** only (MiniZ is not supported for this GBT/`pprpcsb` path)

Ops scripts (Optimus soak): `contrib/testnet-3.0.0/` — proxy rewrites bad `pprpcsb` results to JSON-RPC errors; watchdog restarts the miner on tip advance.

See also [RELEASE_READINESS.md](RELEASE_READINESS.md).

## 5. Stop

```bash
./build-3.0.0/bin/telestai-cli -datadir="$DATADIR" stop
```

## Pass / fail gate

**Pass (private):** node builds; TestNet mines Meraki blocks without stale-job babysitting; subsidy + asset-fee destination correct; AuxPoW RPCs absent; version branding clean; CreateNewBlock harden live.  
**Still required before merge:** at least one **external peer** on the same TestNet tip (deferred separately).  
**Then:** open PR → `master` (still not force-merge).
