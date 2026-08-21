# Foundation import (Phase 1 start)

Date: 2026-08-13  
Branch: `feature/core-3.0.0` only — **do not merge to master**

## What landed

1. Working tree replaced with **Telestai Apex v30.2.6** (Bitcoin Core 30.x–class + assets).
2. Product version set to **Telestai Core 3.0.0** in `CMakeLists.txt`.
3. **Meraki** ethash/ProgPoW tree restored from Telestai 2.1.8 (`num_cache_accesses=12`, `num_math_operations=5`).

## Still required before TestNet

- [ ] Rebrand remaining Telestai strings / paths / datadir (`~/.telestai` → `~/.telestai`)
- [ ] Port Telestai `chainparams` (magic, ports, prefixes, genesis, burns, TesBmcg… subsidy)
- [ ] Remove AuxPoW / merge-mining surfaces from Telestai base
- [ ] Wire Meraki naming in RPC/miner paths; keep tele-meraki-miner compatible
- [ ] Preserve 2.1.8 Meraki nHeight bind semantics
- [ ] Asset fee destination → `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe` (amounts unchanged)
- [ ] Build + sync + mine on **TestNet**
- [ ] Only then: PR → master

Salvage of pre-import Telestai files: `/tmp/tls-3.0.0-salvage` (local; re-copy from git `master` if needed).
