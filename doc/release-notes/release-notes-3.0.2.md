# Telestai Core 3.0.2

Patch release on the Core 3.0 line. Mainnet activation remains **block
1,150,000**. Proof of work remains **Meraki**. No new fork height.

**Tag tip (consensus fix):** `9d59ee526`

## Why this release

Stock **3.0.1** set `MIN_PEER_PROTO_VERSION` to **70030**. That made a 3.0.1
node disconnect every current **2.1.x** peer (protocol **70028**), so a lone
3.0.1 upgrade could not stay connected to today’s seed/pool network.

**3.0.2** keeps speaking protocol **70031** itself, but again accepts peers
down to **70028** so operators can run Core 3.0 while seeds and pools finish
upgrading.

### Fresh mainnet IBD (`bad-diffbits` at block 1)

Early builds of 3.0.x used a Meraki `powLimit` whose compact form was
`0x2000ffff`. Historical mainnet block 1
(`000001ca2aa3249de0587423ac38f82846bb8597baa770187f99e997859c8170`) was
mined under 2.1.x with `bits=1e0fffff`. A **fresh** 3.0 node (empty datadir)
therefore rejected block 1 in `ContextualCheckBlockHeader` (`bad-diffbits`)
and marked every 2.1.x peer misbehaving.

**3.0.2** restores the 2.1.x Meraki pow limit
(`00000ffff…` → compact `0x1e0fffff`) on mainnet and testnet, so
empty-datadir sync from genesis works. Testnet datadirs mined by an
earlier 3.0.x build (compact `0x2000ffff`) must be wiped and resynced.

DGW activation follows `nDGWActivationBlock` (regtest stays at 200).
Mainnet `nMinimumChainWork` and `defaultAssumeValid` are the 2.1.x
height **16943** anchor, so a node will not leave IBD or serve headers
on a low-work chain.

### Binaries

Linux packages on the GitHub release were rebuilt from `release/3.0.2` with
that fix:

- `telestai-3.0.2-linux-x86_64.tar.gz`
- `telestai-3.0.2-linux-arm64.tar.gz`

## Compatibility (important)

| Side | Protocol | Typical user agent |
|------|----------|--------------------|
| This node (3.0.2) | **70031** | `/Telestai:3.0.2/` |
| Minimum accepted peer | **70028** | `/Telestai:2.1.x/` |
| Stock 3.0.1 (do not use alone) | 70031, min peer **70030** | disconnects 2.1.x |

- Prefer **3.0.2** over 3.0.1 for mainnet until most public seeds advertise
  `/Telestai:3.0.x/` with protocol ≥ 70030.
- When the network is fully on 3.0+, a later release may raise the peer floor
  again.

## Unchanged from 3.0.1

- Mining RPCs: `getkawpowhash`, `getmerakihash`, Meraki GBT / `pprpcsb`
- Binary names: `telestaid`, `telestai-cli`, `telestai-qt`
- Asset fee address after activation: `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe`

## Upgrade

If you are on **2.1.x** or isolated **3.0.1**, move to **3.0.2**.

Supported paths:

1. **Preferred for new nodes:** install **3.0.2** and sync from an empty
   mainnet datadir.
2. **Also fine:** sync fully on **2.1.8 / 2.1.9**, stop, replace the binary
   with **3.0.2**, keep the same datadir (in-place upgrade).

After replacing the binary, if startup fails with a corrupt `peers.dat`
message, move `peers.dat` aside (a new one will be created) and restart.
