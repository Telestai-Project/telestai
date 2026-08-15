# Telestai Core 2.1.9

## Summary

Rebuild of the 2.1.8 security release against **Berkeley DB 4.8**, restoring `wallet.dat` compatibility with historical Telestai / Meowcoin / Ravencoin-style Core wallets.

**2.1.8** was published with `--with-incompatible-bdb` (system BDB **5.3**). Opening a legacy 4.8 wallet in that build can rewrite wallet logs so older 4.8-based clients can no longer open the file. **2.1.9** fixes the packaging mistake; consensus code is unchanged from 2.1.8.

## Who should upgrade

- Anyone who downloaded **2.1.8** and uses (or may use) a Core `wallet.dat`
- Operators who deferred 2.1.8 because of the incompatible-BDB warning
- All nodes that still need the KAWPOW `nHeight` security fix (same as 2.1.8)

If you already opened an important wallet exclusively under 2.1.8+BDB5.3, keep a backup before moving between BDB generations, and prefer restoring from a pre-2.1.8 backup when possible.

## What changed vs 2.1.8

- Build Telestai against **Berkeley DB 4.8.30** via `contrib/install_db4.sh` (no `--with-incompatible-bdb`)
- `install_db4.sh`: fallback download mirror (`bitcoincore.org/depends-sources`) if Oracle is unreachable
- Version bump to **v2.1.9.0**

## Unchanged from 2.1.8 (security)

Upgrade before block height **1,100,000** if you have not already taken 2.1.8/2.1.9:

| Rule | Height |
|---|---|
| Absolute epoch bound (`invalid-kawpow-epoch`) | Immediate |
| Strict height bind (`bad-blk-height`) | **1,100,000** |

- Bind KAWPOW header `nHeight` to the expected chain height
- Absolute epoch height limit (`15,000,000`) enforced before PoW hashing
- PoW only computed after parent is found and height is verified

## Downloads

Linux **x86_64** daemon package (`telestaid` + `telestai-cli`, runtime libs bundled). Daemon-only (no Qt).

```bash
tar -xzf telestai-2.1.9-<commit>-x86_64-linux-gnu.tar.gz
cd telestai-2.1.9-<commit>-x86_64-linux-gnu
./bin/telestaid -version
# Telestai Core Daemon version v2.1.9.0
# strings ./bin/telestaid | grep 'Berkeley DB 4.8'
```

## Verify

```bash
telestai-cli getnetworkinfo
# "subversion": "/Telestai:2.1.9/"
ldd $(which telestaid) | grep -i db || echo "BDB statically linked or unused at runtime (expected for 4.8 static)"
```

## Source

- Tag: `2.1.9`
- Branch: `release/2.1.9`
- Parent security fix: 2.1.8 / PR #7
