# Pool / mining operator notes — Telestai Core 3.0.0

**Audience:** pools and solo operators preparing for 3.0.0 TestNet / mainnet.  
**PoW:** Meraki (ProgPoW family; code may still say KAWPOW).  
**Supported miner:** [tele-meraki-miner 1.5.0+](https://github.com/Telestai-Project/tele-meraki-miner/releases) only for GBT/`pprpcsb`.

**Not supported for 3.0.0 solo/GBT:** MiniZ and generic KawPoW stratum miners unless a pool translates to Meraki GBT.

---

## Node requirements

| Item | TestNet | Mainnet (later) |
|------|---------|-----------------|
| Binary | `telestaid` 3.0.0 | same major |
| P2P port | 18770 | 8767 (confirm at release) |
| RPC | local only | local / VPN only |
| `miningaddress` | legacy **P2PKH** | legacy **P2PKH** for Meraki templates |
| AuxPoW RPCs | removed | removed |

Set:

```text
miningaddress=<legacy_p2pkh>
```

GBT Meraki fields: `pprpcheader`, `pprpcepoch`. Submit: **`pprpcsb`** (not BIP22 `submitblock` alone).

---

## Miner command (solo to local node)

```bash
telemerakiminer -U -P 'http://RPCUSER:RPCPASS@127.0.0.1:18766/'
```

Windows:

```text
telemerakiminer.exe -U -P http://RPCUSER:RPCPASS@127.0.0.1:18766/
```

---

## Expected Rejects on fast TestNet

On low-difficulty TestNet with multiple GPUs, **Rejected** after **Accepted** is often **stale work** (tip already moved). Prefer:

- Fresh job after each Accept (node must not reuse `pprpcheader` across tips — fixed in 3.0.0 soak)
- telemerakiminer restart / pool job push on tip change

Mainnet difficulty should make multi-sol-on-one-job far rarer.

---

## Fees / coinbase (operators)

| Network | 25% development / community output |
|---------|--------------------------------------|
| TestNet | `mgaw88zztsHWN8SyL9vXwiCed7aRiPwL26` |
| Mainnet | `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe` |

Asset issue/reissue fees (fixed TLS amounts) go to the **same** development address on 3.0.0 (not burn vanity addresses).

---

## Checklist before advertising a 3.0.0 pool

- [ ] Node `/Telestai:3.0.0/`
- [ ] `getauxblock` not present
- [ ] telemerakiminer Accepts against your GBT
- [ ] Coinbase split verified on a mined block
- [ ] Stratum (if any) correctly maps to Meraki header/nonce/mix_hash
