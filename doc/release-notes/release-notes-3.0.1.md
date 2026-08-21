# Telestai Core 3.0.1

Patch release on the Core 3.0 line. Consensus is unchanged from 3.0.0:
mainnet activation remains **block 1,150,000**. Proof of work remains **Meraki**
(ProgPoW / KawPoW-family). MeowPow hashing is not used.

## Why this release

3.0.0 shipped as a GitHub pre-release (so watchers did not get the usual
email), and pool operators reported missing `getkawpowhash` plus leftover
Telestai binary/RPC names in help text.

3.0.1 is the build pools should run **before** treating 3.0 as mandatory.

## Mining / pool RPC

- Restored **`getkawpowhash`** with the same result shape as 2.1.x
  (`result` / `meets_target` as the strings `"true"` / `"false"`).
- Added **`getmerakihash`** as an alias (same implementation).
- Hashing uses **`progpow::hash`** (Meraki), not MeowPow.
- `getmininginfo` reports `"algorithm": "meraki"`.
- `getnetworkhashps` / `getdifficulty` accept `meraki`, `kawpow`, `kaw`, `tls`
  (legacy `meowpow` still accepted).

GBT / `pprpcsb` behaviour is unchanged from 3.0.0.

## Binaries

Shipped names are `telestaid`, `telestai-cli`, `telestai-qt`. CMake still uses
internal Apex target names (`telestaid`, …) with `OUTPUT_NAME` remapped.

## Upgrade

Run 3.0.1 before height **1,150,000**. Do not stay on 2.1.9 past that height.
Asset fees after activation still go to `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe`.
