# Core 3.0.0 progress (feature/core-3.0.0)

## Done in this session
- Mainnet chainparams → Telestai identity (TELE magic, port 8767, prefixes, genesis, BIP44 10117, HRP `tls`)
- Meraki activation at genesis+1 (Apex KawPoW/Meraki labels; Meraki ethash constants preserved)
- AuxPoW start height INT_MAX on all nets (merge-mining out)
- Block subsidy 468 TLS; development share 25% → `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe`
- Asset fee destinations → same TesBmcg address (amounts unchanged, ADR 0002)
- Runtime rebrand: `.telestai` datadir, `telestai.conf`, currency `TLS`, message magic, pid
- CMake user-facing binary names → `telestaid` / `telestai-*`

## Still open
- Full source/file rename (`telestaid.cpp` → `telestaid.cpp`, qt units, etc.)
- `src/CMakeLists.txt` target wiring may still reference `telestai*` sources
- Strip AuxPoW code paths (currently gated off, not deleted)
- Testnet genesis/params fully Telestai-accurate for soak
- Meraki nHeight bind from 2.1.8 into Apex validation
- First cmake build + TestNet sync/mine
- Do **not** merge to master until TestNet sign-off

## Later session
- Meraki nHeight bind: absolute epoch limit + soft activation at 1,100,000; range check before GetHash/PoW

## Branding / AuxPoW (this session)
- Version/license text: Telestai copyright + https://github.com/Telestai-Project/telestai
- UA_NAME /Telestai:…/; EXE_NAME telestaid; help strings Telestai
- AuxPoW mining RPCs unregistered; activation still INT_MAX
- TestNet soak guide: doc/core-3.0.0/TESTNET_SOAK.md
