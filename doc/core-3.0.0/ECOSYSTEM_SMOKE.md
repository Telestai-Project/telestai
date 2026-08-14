# Ecosystem smoke — Telestai 3.0.0 TestNet

Run after an independent peer is on the tip (or in parallel with shared-proxy mining).

## Explorer / Cryptoscope

- [ ] Tip height matches Optimus ±1  
- [ ] Block page shows coinbase outputs (miner + `mgaw88zz…`)  
- [ ] Asset issue tx `af508a40f13053fee55b19297fd2ecf1011ece9463ae2ab997cee40f18505c7e` (or newer) decodes with 500 TLS to development address  
- [ ] No AuxPoW UI assumptions  

## Zeroa

- [ ] Point a **TestNet** profile at a 3.0.0 RPC (never mainnet wallet)  
- [ ] Receive to legacy address; send small TLS  
- [ ] Optional: asset issue UI / fee quote shows development destination (not burn vanity)  

## Halo

- [ ] No Core mining dependency  
- [ ] If fee addresses are displayed anywhere, TestNet uses `mgaw88zz…`  

## Pools

- [ ] Follow [POOL_OPERATORS.md](POOL_OPERATORS.md)  
- [ ] telemerakiminer Accept against pool GBT  

## Sign-off

| Area | Tester | Date | Pass? |
|------|--------|------|-------|
| Explorer | | | |
| Zeroa | | | |
| Pool | | | |
