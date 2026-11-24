# Telestai Core 3.0.3

Patch release on the Core 3.0 line. Mainnet activation remains **block
1,150,000**. Proof of work remains **Meraki**. No new fork height.

This is **3.0.2** plus the merged asset-rule restoration
([PR 9](https://github.com/Telestai-Project/telestai/pull/9), merge
`f342b9076613145fee56b1b7264cfd8ac22044dc`) so pools can tell it from stock
3.0.2. The user agent is `/Telestai:3.0.3/`.

`nCore300ActivationHeight`, SegWit, Taproot, and the TesBmcg asset fee
switch are unchanged from 3.0.2.

## Compatibility

| Side | Protocol | Typical user agent |
|------|----------|--------------------|
| This node (3.0.3) | **70031** | `/Telestai:3.0.3/` |
| Minimum accepted peer | **70028** | `/Telestai:2.1.x/` |
| Stock 3.0.2 | 70031, min peer **70028** | `/Telestai:3.0.2/` |

Stock 3.0.2 speaks the same protocol but does not include the asset-rule
restoration. Prefer **3.0.3** when pools need that restoration.

## Unchanged from 3.0.2

- Peer floor: protocol **70028**
- Mining RPCs: `getkawpowhash`, `getmerakihash`, Meraki GBT / `pprpcsb`
- Binary names: `telestaid`, `telestai-cli`, `telestai-qt`
- Asset fee address after activation: `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe`
- Mainnet activation height: **1,150,000**

## Upgrade

If you are on **2.1.x**, stock **3.0.1**, or stock **3.0.2**, move to **3.0.3**.

Supported paths:

1. **Preferred for new nodes:** install **3.0.3** and sync from an empty
   mainnet datadir.
2. **Also fine:** sync fully on **2.1.8 / 2.1.9**, stop, replace the binary
   with **3.0.3**, keep the same datadir (in-place upgrade).

After replacing the binary, if startup fails with a corrupt `peers.dat`
message, move `peers.dat` aside (a new one will be created) and restart.
