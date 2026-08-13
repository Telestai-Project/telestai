# Telestai 3.0.0 TestNet — easy mining guide

Thanks for helping test. This is **TestNet only** (play chain), not real mainnet money.

---

## Fast path (most people)

### 1) Download the miner

Open:

**https://github.com/Telestai-Project/tele-meraki-miner/releases/tag/1.5.0**

Download:

- **Windows:** `WindowsRelease.zip`
- **Linux:** `linuxRelease.tar.xz`

Unpack the file. You should see `telemerakiminer` / `telemerakiminer.exe`.

You need an **NVIDIA GPU** and normal NVIDIA drivers installed.

### 2) Run the mining command Andrew sent you

Andrew will send you **one line** that looks like:

```text
telemerakiminer.exe -U -P http://USER:PASSWORD@IP:18768/
```

or on Linux:

```bash
./telemerakiminer -U -P 'http://USER:PASSWORD@IP:18768/'
```

1. Open a terminal in the miner folder.
2. Paste that whole line.
3. Press Enter.
4. Leave it running.

### 3) What success looks like

- The miner stays connected.
- You occasionally see **Accepted**.

If it keeps saying **Rejected**, crashes, or can’t connect, screenshot the window and send it to Andrew.

### Please don’t

- Don’t share the mining link publicly (it’s a password).
- Don’t use your mainnet wallet seed/password here.
- Don’t expect these TestNet coins to be worth anything.

---

## Optional: run your own TestNet node (advanced)

Linux binaries for this soak:

See the GitHub prerelease **`testnet-3.0.0-soak`** on the Telestai Core repo (asset `telestai-3.0.0-testnet-linux-x86_64.tar.gz`).

Or build branch `feature/core-3.0.0`.

Then:

1. Use a **brand-new empty folder** for data (don’t reuse old Telestai folders).
2. Add peer: `addnode=114.73.210.115:18770`
3. Create a TestNet address and set `miningaddress=...`
4. Mine to your **local** node (`127.0.0.1:18766`), not the shared mining link.

Most testers can skip this and just use the Fast path.

---

## Help

Send Andrew:

- Windows or Linux
- GPU model
- Miner screenshot / last lines of output
