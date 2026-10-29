# Pool / mining operator notes — Telestai Core 3.0.2

**Audience:** pools and solo operators preparing for 3.0 TestNet / mainnet.  
**PoW:** Meraki (ProgPoW family; code may still say KAWPOW internally).  
**Supported miner:** [tele-meraki-miner 1.5.0+](https://github.com/Telestai-Project/tele-meraki-miner/releases) only for GBT/`pprpcsb`.

**Not supported for 3.0 solo/GBT:** MiniZ and generic KawPoW stratum miners unless a pool translates to Meraki GBT.

Use **3.0.2** (not 3.0.0 / stock 3.0.1 alone). 3.0.0 omitted `getkawpowhash`. Stock **3.0.1** raised the P2P peer floor to protocol **70030**, which disconnects current **2.1.x** seeds (**70028**). **3.0.2** keeps Meraki/mining RPCs from 3.0.1 and restores peer compatibility with 2.1.x until the network upgrades.

---

## Node requirements

| Item | TestNet | Mainnet |
|------|---------|---------|
| Binary | `telestaid` **3.0.2** | same |
| P2P port | 18770 | 8767 |
| RPC | local only | local / VPN only |
| `miningaddress` | legacy **P2PKH** | legacy **P2PKH** for Meraki templates |
| AuxPoW RPCs | removed | removed |

Set:

```text
miningaddress=<legacy_p2pkh>
```

GBT Meraki fields: `pprpcheader`, `pprpcepoch`. Submit: **`pprpcsb`** (not BIP22 `submitblock` alone).

Share check (same as 2.1.x): **`getkawpowhash`**. Alias: **`getmerakihash`**.

```text
getkawpowhash "header_hash" "mix_hash" "nonce_hex" height ["target"]
```

`result` and `meets_target` are the strings `"true"` / `"false"` (not JSON booleans).

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

**Mainnet fork height: 1,150,000** (already activated). Run **3.0.2**. Asset issue/reissue/tag fees must pay the development address, not 2.1.x vanity burns. Coinbase 25% is unchanged.

Asset issue/reissue fees (fixed TLS amounts) go to the **same** development address on 3.0 after activation (not burn vanity addresses).

---

## Mainnet GBT switch (when `v3.0.2` node binaries are on GitHub)

Do **not** point production stratum at 3.0 until `telestaid` for your OS is on https://github.com/Telestai-Project/telestai/releases/tag/v3.0.2

Then:

1. Run **3.0.2** until synced (same P2P magic/port 8767; `addnode` a seed). Prefer 3.0.2 over stock 3.0.1 so you still peer with 2.1.x seeds.
2. Point GBT / `pprpcsb` at the 3.0.2 RPC only (telemerakiminer, not MiniZ).
3. Confirm `getkawpowhash` (or `getmerakihash`) is present before cutting over.
4. Confirm coinbase vout[1] is still `TesBmcgLQsowvYEYPXpSHkkapoTbVV7Xfe` (25%). Asset fees follow that address from 1,150,000.

## Checklist before advertising a 3.0 pool

- [ ] Node `/Telestai:3.0.2/`
- [ ] Peers include existing seeds (2.1.x OK on 3.0.2; stock 3.0.1 will drop them)
- [ ] `getauxblock` not present
- [ ] `getkawpowhash` present (and `getmerakihash`)
- [ ] telemerakiminer Accepts against your GBT
- [ ] Coinbase split verified on a mined block
- [ ] Stratum (if any) correctly maps to Meraki header/nonce/mix_hash
- [ ] Mainnet switch done only from tagged `v3.0.2` binaries
