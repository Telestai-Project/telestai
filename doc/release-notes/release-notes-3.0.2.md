# Telestai Core 3.0.2

Patch release on the Core 3.0 line. Consensus is unchanged from 3.0.0 / 3.0.1:
mainnet activation remains **block 1,150,000**. Proof of work remains **Meraki**.

## Why this release

Stock **3.0.1** set `MIN_PEER_PROTO_VERSION` to **70030**. That made a 3.0.1
node disconnect every current **2.1.x** peer (protocol **70028**), so a lone
3.0.1 upgrade could not stay connected to today’s seed/pool network.

**3.0.2** keeps speaking protocol **70031** itself, but again accepts peers
down to **70028** so operators can run Core 3.0 while seeds and pools finish
upgrading.

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

After replacing the binary, if startup fails with a corrupt `peers.dat`
message, move `peers.dat` aside (a new one will be created) and restart.
