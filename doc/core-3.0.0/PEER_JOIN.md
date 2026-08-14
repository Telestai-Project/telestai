# Join Optimus Telestai 3.0.0 TestNet as an independent peer

**Goal:** Run your own `telestaid` (not only the shared mining proxy) so P2P sync + local mining prove the network.

Optimus seed (WAN): **`114.73.210.115`**
- P2P: **18770**
- Shared mining proxy (optional / easier): **18768** (needs credentials from Andrew — do not post publicly)

---

## 1) Get binaries

### Option A — release tarball (preferred)

Download the soak package from the prerelease:

https://github.com/Telestai-Project/telestai/releases/tag/v3.0.0-testnet-soak

Asset: `telestai-3.0.0-testnet-linux-x86_64-20260814.tar.gz`

(Also on Optimus: `/home/chief/telestai/releases/`.)

Verify checksum:

```bash
# expected: 84769d437adcd4ceb839d33c06004b8963a2a4df2945a71aafbcb77c6fd0f08f
sha256sum -c telestai-3.0.0-testnet-linux-x86_64-20260814.tar.gz.sha256
```

Unpack:

```bash
tar -xzf telestai-3.0.0-testnet-linux-x86_64-20260814.tar.gz
cd telestai-3.0.0-testnet-linux-x86_64
```
### Option B — build from source

```bash
git clone https://github.com/Telestai-Project/telestai.git
cd telestai
git checkout feature/core-3.0.0
# follow CMake build for meowcoind / meowcoin-cli targets → telestaid / telestai-cli
```

---

## 2) Fresh datadir (required)

Do **not** reuse 2.1.x TestNet data. 3.0.0 TestNet genesis differs.

```bash
DATADIR="$HOME/telestai-3.0.0-testnet"
mkdir -p "$DATADIR"

cat > "$DATADIR/telestai.conf" <<EOF
testnet=1
server=1
listen=1
txindex=1
rpcuser=telestai
rpcpassword=CHANGE_ME_STRONG
rpcallowip=127.0.0.1
rpcport=18766
port=18770
addnode=114.73.210.115:18770
EOF
```

---

## 3) Start node + sync

```bash
export LD_LIBRARY_PATH="$PWD/lib:${LD_LIBRARY_PATH:-}"   # if tarball includes lib/
./bin/telestaid -datadir="$DATADIR" -daemon
./bin/telestai-cli -datadir="$DATADIR" -getinfo
./bin/telestai-cli -datadir="$DATADIR" getblockchaininfo
./bin/telestai-cli -datadir="$DATADIR" getconnectioncount
```

Expect:
- `subversion` like `/Telestai:3.0.0/`
- `connections` ≥ 1 (to Optimus)
- `blocks` / `headers` catch Optimus tip

Check Optimus tip with Andrew if needed.

---

## 4) Mine to YOUR node (not the shared proxy)

```bash
./bin/telestai-cli -datadir="$DATADIR" createwallet soak
ADDR=$(./bin/telestai-cli -datadir="$DATADIR" getnewaddress "" legacy)
echo "$ADDR"

# restart daemon with mining address, or:
./bin/telestai-cli -datadir="$DATADIR" stop
./bin/telestai-cli -datadir="$DATADIR"  # wait
./bin/telestaid -datadir="$DATADIR" -miningaddress="$ADDR" -daemon
```

Miner ([tele-meraki-miner 1.5.0](https://github.com/Telestai-Project/tele-meraki-miner/releases/tag/1.5.0)):

```bash
./telemerakiminer -U -P "http://telestai:CHANGE_ME_STRONG@127.0.0.1:18766/"
```

**Do not use MiniZ** for this GBT / `pprpcsb` path.

---

## 5) Success criteria (send to Andrew)

- [ ] `getconnectioncount` ≥ 1  
- [ ] Tip matches Optimus (±1 while racing)  
- [ ] Coinbase 25% → `mgaw88zztsHWN8SyL9vXwiCed7aRiPwL26`  
- [ ] Local `**Accepted` lines and your wallet balance moves  
- [ ] Optimus shows you as a peer  

---

## Optional: shared proxy only (not a peer soak)

```text
telemerakiminer.exe -U -P http://USER:PASSWORD@114.73.210.115:18768/
```

Useful for miner smoke tests; **does not** satisfy independent-peer sign-off.
