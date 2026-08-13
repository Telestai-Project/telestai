# Core 3.0.0 — release readiness (minus external peer)

**Branch:** `feature/core-3.0.0`  
**Updated:** 2026-08-14  
**Do not merge until this checklist is green.**

## Mining / consensus (private TestNet)

| Item | Status |
|------|--------|
| Meraki blocks advance with `telemerakiminer` | Required live |
| Coinbase 75% miner + 25% → `mgaw88zz…` (TestNet) / `TesBmcg…` (MainNet) | Required |
| Asset issue fee 500 TLS → same development address | Required |
| AuxPoW RPCs absent (`getauxblock` etc.) | Required |
| CreateNewBlock aborts on bad community address (no silent fallback) | Required |
| Stale `pprpcheader` reuse across tip changes fixed | Required |
| `pprpcsb` rejects stale/duplicate/inconclusive as RPC errors | Required |
| Watchdog restarts miner on tip advance + stale job | Required |
| `COINBASE_MATURITY = 100` | Required |
| Mainnet daemon on Optimus untouched during soak | Required |

## Explicitly deferred

| Item | Why |
|------|-----|
| External TestNet peer sync + mine | Out of scope for this pass (user) |
| Mainnet activation / cutover | After TestNet sign-off |
| MiniZ as supported miner | Use `telemerakiminer` only |

## Before opening PR → `master`

1. Private soak tip **100+** without babysitting (watchdog-only)
2. No new `errors.log` spam (`bad-cb`, mix_hash, Assert)
3. Clean commit history on the branch (no `wip(3.0.0):` stack)
4. External peer soak (when scheduled)
5. Release notes: miners must use **tele-meraki-miner 1.5.0+**, not MiniZ for solo/GBT
6. Datadir note: 3.0.0 TestNet genesis ≠ 2.1.x TestNet — fresh datadir

## Mainnet cutover (later)

- Publish Linux (then Windows) `telestaid` / `telestai-cli`
- Pool operators: Meraki GBT + `pprpcsb`; point at telemerakiminer
- Explorers / Electrum / Zeroa smoke on TestNet first
- Mandatory node upgrade window; asset-fee destination already TestNet-proven
