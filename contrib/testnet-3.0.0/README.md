# TestNet 3.0.0 soak ops (Optimus)

Scripts live on Optimus at `/home/chief/telestai/testnet-3.0.0/bin/` and are mirrored here.

| Script | Role |
|--------|------|
| `start_testnet_miner.sh` | Start miner + governor; filter `Sol:` spam; record tip |
| `miner_watchdog.sh` | Restart on death / RSS>8GiB / tip advance / huge log |
| `mining_rpc_proxy.py` | Allowlist proxy with HTTP/1.1 + longpoll timeout |
| `restart_mining_proxy.sh` | Bounce proxy with env from `telestai.conf` |
| `watch_testnet.sh` | Status JSON + **new-only** debug error append |
| `gpu_thermal_governor.sh` | SIGSTOP/CONT heat safety net |
| `apply_gpu_power.sh` | Power/clock caps |

Cron (example):

```cron
*/2 * * * * .../watch_testnet.sh
* * * * * .../miner_watchdog.sh
*/15 * * * * .../apply_gpu_power.sh
*/30 * * * * .../ensure_upnp.sh
```

CreateNewBlock harden (invalid community address → `return nullptr`) is in
`src/node/miner.cpp` on `feature/core-3.0.0`. Rebuild/redeploy `telestaid` to
pick that up; the live soak already uses a valid TestNet community address.

### Stale-job mitigations (required for unattended soak)

1. **Core GBT** only reuses `pprpcheader` when `hashPrevBlock` + `nHeight` still match.
2. **`pprpcsb`** rejects stale prev / duplicate / inconclusive as JSON-RPC errors and
   erases the sealed template.
3. **Proxy** rewrites any non-`true` `pprpcsb` result string into an error (telemerakiminer
   otherwise prints false `**Accepted`).
4. **Watchdog** restarts the miner immediately when the tip advances (miner does not
   reliably longpoll).
