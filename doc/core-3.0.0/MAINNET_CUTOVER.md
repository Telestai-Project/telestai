# Mainnet cutover runbook — Telestai Core 3.0.0

**Status:** Draft — execute only after TestNet peer soak sign-off.  
**Related:** [RELEASE_READINESS.md](RELEASE_READINESS.md), [PEER_JOIN.md](PEER_JOIN.md), [POOL_OPERATORS.md](POOL_OPERATORS.md)

---

## Preconditions (all must be green)

1. Independent TestNet peer synced + mined ([PEER_JOIN.md](PEER_JOIN.md))
2. Asset fee + 25% subsidy verified on TestNet
3. PR review complete on `feature/core-3.0.0` (or release tag)
4. Linux release binaries + checksums published
5. Pool operators notified (telemerakiminer, not MiniZ)
6. Explorer + Zeroa TestNet smoke complete (or explicitly waived)

---

## Lock before announcement

| Item | Value |
|------|--------|
| Meraki / ProgPoW constants | Telestai (not Meowcoin) — re-verify in tagged binary |
| Mainnet 25% + asset fees | `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe` |
| AuxPoW | Absent |
| Activation | **Height 1,150,000** — mandatory Core 3.0.0 upgrade. Asset fees switch from 2.1.x vanity burns to `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe`. ~16 Oct 2026 at 1 min/block from Aug 2026 tip. |
| Datadir | Do **not** reuse broken TestNet dirs; mainnet path separate |

---

## T−7 days

- [ ] Publish RC binaries + SHA256SUMS
- [ ] Discord / Discord mining channel / Discord: upgrade notice
- [ ] Pool dry-run on TestNet with production miner build
- [ ] Explorer team: 3.0.0 TestNet tip + sample asset tx
- [ ] Zeroa: send/receive on TestNet; note legacy address for assets

## T−1 day

- [ ] Confirm Optimus mainnet still on last good 2.1.x (or agreed baseline) until cut
- [ ] Stage 3.0.0 mainnet binary on Optimus **without** switching yet
- [ ] Backup `wallet` / conf / datadir snapshots

## T0 — cutover window (block **1,150,000**)

Everyone must be on tagged `v3.0.0` **before** this height. 2.1.9 nodes will not follow 3.0.0 asset-fee blocks after activation.

1. Announce start of mandatory upgrade window  
2. Stop mining on old major if incompatible  
3. Upgrade seed/public nodes (Optimus first or with peers)  
4. Verify: `getnetworkinfo` UA, tip advances, coinbase 25% address, no AuxPoW  
5. Pools switch stratum/GBT to 3.0.0  
6. Monitor `debug.log` for `bad-cb`, mix_hash, Assert for 24h  

## T+1 … T+7

- [ ] External miner Accept rates normal at mainnet diff  
- [ ] Asset issue smoke on mainnet (small test asset) if governance allows  
- [ ] Zeroa production point at upgraded nodes  
- [ ] Tag final `v3.0.0` if RC held  

## Rollback (only if catastrophic)

Document whether rollback is possible (usually **not** after divergent tips). Prefer halt mining + hotfix forward. If pre-activation only, stay on 2.1.x until fixed RC.

---

## Contacts / owners (fill in)

| Role | Owner |
|------|--------|
| Core binary | |
| Optimus ops | |
| Pools | |
| Explorer | |
| Zeroa | |
| Comms | |
