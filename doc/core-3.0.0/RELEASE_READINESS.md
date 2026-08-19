# Core 3.0.0 — release readiness & gap plan

**Branch:** `feature/core-3.0.0`  
**Updated:** 2026-08-14  
**Live soak signal:** private TestNet tip 1000+, external Windows `telemerakiminer` Accepts, asset fee + 25% subsidy verified.

**Verdict:** TestNet soak functionally strong. Remaining work tracked below is **in progress / actionable**.

---

## Docs index

| Doc | Purpose |
|-----|---------|
| [PEER_JOIN.md](PEER_JOIN.md) | Independent P2P peer + local mining |
| [POOL_OPERATORS.md](POOL_OPERATORS.md) | Pool / miner operator notes |
| [MAINNET_CUTOVER.md](MAINNET_CUTOVER.md) | Mainnet upgrade runbook |
| [ECOSYSTEM_SMOKE.md](ECOSYSTEM_SMOKE.md) | Explorer / Zeroa / pool checklist |
| [EXTERNAL_TESTNET_MINING.md](EXTERNAL_TESTNET_MINING.md) | Shared-proxy miner quickstart |
| [TESTNET_SOAK.md](TESTNET_SOAK.md) | Soak procedure |

---

## Already green (soak)

| Item | Evidence |
|------|----------|
| Meraki blocks + tip 1000+ | Optimus private TestNet |
| Coinbase 351 + 117 → `mgaw88zz…` | Tip coinbase |
| Asset issue 500 → same address | tx `af508a40…` |
| AuxPoW RPCs absent | confirmed |
| CreateNewBlock abort + stale-job harden | live binary |
| External GPU via shared proxy | Windows telemerakiminer |
| Dual-miner race | Optimus + external |
| Clean branch history | chore import + feat port |
| Tip refresh daemon + proxy tip flag | contrib ops |
| Peer / pool / cutover / ecosystem docs | this folder |
| Linux TestNet tarball script | `package_testnet_tarball.sh` |

---

## Execution checklist (do now / next)

| # | Item | Action | Status |
|---|------|--------|--------|
| 1 | Independent peer | Publish tarball; tester follows PEER_JOIN | **Tarball ready** (`releases/…-20260814.tar.gz`); needs external operator |
| 2 | Stale Reject UX | `tip_refresh_daemon` + proxy tip flag + core pprpcsb harden | **Ops live** on Optimus + `@reboot` |
| 3 | Release packaging | Run `package_testnet_tarball.sh` on Optimus; attach to GH prerelease | **Packaged** sha256 `84769d43…` |
| 4 | PR | Push branch + `gh pr create` | **This PR** |
| 5 | Mainnet cutover | Follow MAINNET_CUTOVER; fork height **1,150,000** is compiled in | **Height locked; execute later** |
| 6 | Ecosystem smoke | ECOSYSTEM_SMOKE checklist | **Doc ready; needs humans** |

**P2P note:** Optimus listens on `0.0.0.0:18770` (UPnP cron active). `localaddresses` may be empty behind NAT — peers must use `addnode=114.73.210.115:18770`.

---

## Suggested sequence

```text
1) Package tarball + open PR
2) External peer join (PEER_JOIN) — blocking for merge sign-off
3) Ecosystem smoke in parallel
4) TestNet sign-off → MAINNET_CUTOVER
```
