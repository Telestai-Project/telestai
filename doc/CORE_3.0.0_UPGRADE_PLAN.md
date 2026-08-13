# Telestai Core 3.0.0 — Full Upgrade Plan

Date: 2026-08-13  
Status: **In progress on `feature/core-3.0.0`** (do **not** merge to `main`/`master` until TestNet sign-off)  
Related: [ADR 0002](adr/0002-core-3.0.0-asset-fees-to-dev-reward.md)

## Goal

Ship **Telestai Core 3.0.0**: a generational upgrade from the current Ravencoin / Bitcoin ~0.15 lineage (v2.1.8) onto a **Bitcoin Core 30.x–class** codebase — modern security, P2P, wallets, and address formats — while preserving Telestai consensus:

- **Meraki** PoW (ProgPoW derivative; code historically labeled “KAWPOW”)
- Assets / messaging / restricted / IPFS
- 25% development subsidy → `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe`

Primary engineering reference: Meowcoin Core Apex (BTC 30.2 rebase + asset / ProgPoW-family port).  
**Critical:** keep Telestai Meraki ProgPoW constants (not Meowcoin’s), especially:

| Param | Telestai Meraki | (Meowcoin differs) |
|-------|-----------------|--------------------|
| `period_length` | 3 | 3 |
| `num_cache_accesses` | **12** | 11 |
| `num_math_operations` | **5** | 18 |

## Explicitly out of scope

- AuxPoW / merge-mining
- USDT/oracle-priced asset fees (keep fixed TLS amounts)
- Post-quantum (ML-DSA / witness v2) in 3.0.0 — optional later
- Rewriting Zeroa as part of Core (Zeroa follows after TestNet)

## Branching / release rule

```
master/main  ── unchanged until TestNet validated ──► merge 3.0.0 later
                  ▲
feature/core-3.0.0 ── all work happens here ──► TestNet soak ──► PR
```

- Develop only on `feature/core-3.0.0` (or child branches merging into it).
- **No merge to `master`/`main` until TestNet testing passes.**
- Version string: **3.0.0** (Telestai product version; upstream BTC base remains 30.x-class internally).

---

## Current baseline (2.1.8)

| Area | State |
|------|--------|
| Base | ~Bitcoin Core 0.15 + Ravencoin assets |
| Build | Autotools, C++17 |
| PoW | **Meraki** (ProgPoW derivative; identifiers still say KAWPOW in tree) |
| Assets | Full Raven-style layer |
| Asset fees | Fixed TLS → unspendable burn addresses |
| Block subsidy | 25% → `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe` |
| SegWit | Partially present; no native bech32 UX |
| Taproot / bech32m | Absent |
| Wallet | Berkeley DB + BIP39 in Core |

---

## Target (Core 3.0.0)

| Area | Target |
|------|--------|
| Product version | **3.0.0** |
| Upstream base | Bitcoin Core **30.x** (track Meowcoin Apex deltas) |
| Build | CMake (+ Guix when ready) |
| PoW | **Meraki** preserved (ProgPoW params above + nHeight bind from 2.1.8) |
| Assets | Ported onto 30.x validation/mempool/wallet/indexes |
| Asset fees | **Same TLS amounts**; destination → `TesBmcg…` ([ADR 0002](adr/0002-core-3.0.0-asset-fees-to-dev-reward.md)) |
| Block subsidy | Unchanged 25% development reward |
| Addresses | SegWit + Taproot / bech32m in protocol; UI gated until network adoption |
| Wallet | Descriptor / modern wallet; migration path from legacy |
| Assets on new formats | Start **P2PKH-only** for asset ops; expand later |
| First validation | **TestNet** only |

---

## Workstreams

### 1. Rebase foundation
- Start from Bitcoin Core 30.x / Meowcoin Apex as port map
- CMake, CI, depends
- Telestai chainparams, magic, ports, prefixes (`T` / BIP44 10117)

### 2. Consensus port
- Meraki headers (`nHeight`, `nNonce64`, `mix_hash`), DGW, subsidy/halving
- Preserve 2.1.8 height-bind rules (absolute epoch limit + strict bind at 1,100,000) under Meraki naming
- Asset + messaging + restricted + IPFS validation
- Modern indexes
- Development coinbase rules (25% → TesBmcg…)

### 3. Asset fee destination (activate with 3.0.0 TestNet, then mainnet later)
- Burn-address checks → development reward address
- Amounts unchanged
- Activation height on TestNet first

### 4. Wallet & addresses
- Descriptor wallets; legacy migration
- SegWit + Taproot via RPC; Qt gated
- PSBT/PSMT groundwork for Zeroa later

### 5. Mining & network
- GBT / Meraki templates on TestNet
- Pool / tele-meraki-miner compatibility check

### 6. Ecosystem (after TestNet green)
- Zeroa, explorers, docs, Discord — post TestNet

---

## Phases

```
Phase 0 — Prep                          ← current
  • Branch feature/core-3.0.0
  • Lock Meraki params + Meowcoin reference
  • Inventory Telestai-only modules

Phase 1 — Bring-up
  • CMake tree builds telestaid / cli / qt
  • Sync TestNet; mine Meraki

Phase 2 — Feature-complete TestNet
  • Full asset port + indexes
  • Fee destination → TesBmcg… at TestNet activation height
  • Descriptor wallets; SegWit/Taproot RPC

Phase 3 — TestNet soak / RC
  • Multi-arch builds as needed
  • tele-meraki-miner + explorer against TestNet
  • Security review of consensus diffs

Phase 4 — Mainnet 3.0.0 (only after TestNet sign-off)
  • PR merge to master
  • Tag 3.0.0; publish binaries
  • Mandatory upgrade window before mainnet fee-destination activation

Phase 5 — Post-ship (optional)
  • Taproot UI, assets on non-P2PKH, PQ, USDT fees if revisited
```

---

## Locked decisions

| Decision | Choice |
|----------|--------|
| Product version | **3.0.0** |
| PoW name | **Meraki** (ProgPoW derivative) |
| Merge-mining / AuxPoW | **No** |
| Asset fee destination | **`TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe`** |
| Asset fee amounts | **Unchanged** |
| USDT/oracle pricing | **Not now** |
| PQ addresses | **Not in 3.0.0** |
| Merge to main | **Only after TestNet validation** |

---

## Success criteria (TestNet gate)

- TestNet tip-compatible with Meraki mining + 25% dev subsidy
- Assets issue/transfer/reissue; fees land on TesBmcg… after activation
- Subversion reports Telestai Core **3.0.0**
- No merge to main until this gate passes
