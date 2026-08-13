# Core 3.0.0 progress (feature/core-3.0.0)

## Done in this session
- Mainnet chainparams → Telestai identity (TELE magic, port 8767, prefixes, genesis, BIP44 10117, HRP `tls`)
- Meraki activation at genesis+1 (Apex KAWPOW/MEOWPOW labels; Meraki ethash constants preserved)
- AuxPoW start height INT_MAX on all nets (merge-mining out)
- Block subsidy 468 TLS; development share 25% → `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe`
- Asset fee destinations → same TesBmcg address (amounts unchanged, ADR 0002)
- Runtime rebrand: `.telestai` datadir, `telestai.conf`, currency `TLS`, message magic, pid
- CMake user-facing binary names → `telestaid` / `telestai-*`

## Still open
- Full source/file rename (`meowcoind.cpp` → `telestaid.cpp`, qt units, etc.)
- `src/CMakeLists.txt` target wiring may still reference `meowcoin*` sources
- Strip AuxPoW code paths (currently gated off, not deleted)
- Testnet genesis/params fully Telestai-accurate for soak
- Meraki nHeight bind from 2.1.8 into Apex validation
- First cmake build + TestNet sync/mine
- Do **not** merge to master until TestNet sign-off
