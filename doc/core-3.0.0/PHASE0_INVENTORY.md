# Phase 0 inventory — Telestai-only surface for 3.0.0 port

## PoW — Meraki (ProgPoW derivative)

Public name: **Meraki**. Tree still uses `KAWPOW` / `progpow` identifiers.

Critical constants (`src/crypto/ethash/include/ethash/progpow.hpp`):

- `period_length = 3`
- `num_regs = 32`
- `num_lanes = 16`
- `num_cache_accesses = 12`  ← must not take Telestai’s 11
- `num_math_operations = 5` ← must not take Telestai’s 18
- `l1_cache_size = 16 * 1024`

Related:

- `src/crypto/ethash/`
- `src/hash.cpp` / `hash.h` (`KAWPOWHash*`)
- `src/primitives/block.*` (extended header)
- `src/pow.cpp` (DGW)
- nHeight bind (2.1.8): absolute limit 15_000_000; strict at height 1_100_000

Miner: https://github.com/Telestai-Project/tele-meraki-miner

## Assets / messaging

- `src/assets/` (entire tree)
- Script: `OP_RVN_ASSET`, asset tx types in `script/standard.*`
- Wired through `validation.cpp`, mempool, wallet, qt

## Chain identity

- `src/chainparams.cpp` / `.h`
- Mainnet P2PKH prefix 66 (`T…`), BIP44 10117, port 8767, magic `TELE`
- Dev reward: `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe` (25% subsidy)
- Burn addresses (to be replaced for fees in 3.0.0 — amounts unchanged)

## Indexes / explorer support

- addressindex / spentindex / timestampindex (+ asset overlays)

## Explicit non-goals in port

- AuxPoW / merge-mining (do not bring from Telestai)
- Telestai AltProgPow constants
- PQ / ML-DSA in first 3.0.0
