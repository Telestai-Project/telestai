# Telestai 3.0.0 TestNet — easy mining guide (Optimus soak)

This is a **private TestNet** for Core 3.0.0. You are helping us check that Meraki mining works before any mainnet upgrade.

You do **not** need to sync the whole mainnet. You only talk to the Optimus TestNet.

---

## What you need

1. A **NVIDIA GPU** PC (Windows or Linux).
2. The **Tele Meraki miner** (download below).
3. The **mining link** we send you (password included).  
   Ask Andrew for the current link if you don’t have it — it can change when the tunnel restarts.

Optional: the **TestNet node** zip/tarball if you want to run your own peer. Most people only need the miner.

---

## Step 1 — Download the miner

Go to:

**https://github.com/Telestai-Project/tele-meraki-miner/releases/tag/1.5.0**

Download:

- **Windows:** `WindowsRelease.zip`
- **Linux:** `linuxRelease.tar.xz`

Unpack it. You should see a program called `telemerakiminer` (or `telemerakiminer.exe`).

You also need **NVIDIA drivers** installed (normal Game Ready / Studio drivers are fine).  
On Linux you may also need CUDA’s `libnvrtc` (CUDA 12). If the miner says it can’t find `libnvrtc.so.12`, ask Andrew for the small library pack.

---

## Step 2 — Mine to Optimus TestNet

Open a terminal (Command Prompt / PowerShell / Terminal) in the miner folder.

**Windows example:**

```bat
telemerakiminer.exe -U -P PASTE_THE_MINING_LINK_HERE
```

**Linux example:**

```bash
./telemerakiminer -U -P 'PASTE_THE_MINING_LINK_HERE'
```

Replace `PASTE_THE_MINING_LINK_HERE` with the full `http://…` link Andrew gives you.

You should see the miner start, find a “job”, then show **Accepted** now and then. That means a TestNet block was found and the Optimus node took it.

Leave it running. You can stop anytime with Ctrl+C.

---

## Step 3 — What “good” looks like

- Miner stays connected (no constant disconnect spam).
- Occasional **Accepted** lines.
- Not only **Rejected** forever.

If it only rejects, or crashes, copy the last 20 lines of the miner window and send them to Andrew.

---

## Important notes (please read)

- This is **TestNet play money**, not real TLS mainnet coins.
- Rewards on this soak go to the Optimus TestNet soak wallet (shared test address), not your personal mainnet wallet.
- Do **not** put your mainnet wallet password or seed into anything here.
- Do **not** share the mining link publicly (it’s like a temporary password).

---

## Optional — run your own TestNet node (advanced)

Only if you want to connect as a peer and mine to your own TestNet address:

1. Get the **Core 3.0.0 TestNet binary pack** from the GitHub prerelease for this soak (or build `feature/core-3.0.0`).
2. Use a **new empty folder** as the data directory (don’t reuse an old Telestai wallet folder).
3. Point your node at Optimus with:

   `addnode=TESTNET_PEER_HOST:18770`

   (Andrew will give you the peer host/IP once P2P is open.)
4. Set `miningaddress=` to a TestNet address from your node.
5. Mine to **your own** `http://USER:PASS@127.0.0.1:18766/` as usual.

If you can’t connect as a peer yet, just use Step 2 (mine through the Optimus mining link).

---

## Help

Ping Andrew with:

- Your OS (Windows/Linux)
- GPU model
- Miner log snippet
- Whether you saw any **Accepted** lines
