#!/usr/bin/env python3
"""Allowlist proxy: mining RPCs to local telestaid TestNet.

- Forwards getblocktemplate / pprpcsb (and a small read-only allowlist).
- Converts non-true pprpcsb results into JSON-RPC errors so telemerakiminer
  prints Rejected instead of a false **Accepted (it treats any HTTP 200
  result string like "inconclusive" as success).
- Supports long getblocktemplate longpoll timeouts.
"""
from __future__ import annotations

import base64
import json
import os
import urllib.error
import urllib.request
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

UPSTREAM = os.environ.get("TLS_UPSTREAM", "http://127.0.0.1:18766")
LISTEN = os.environ.get("TLS_PROXY_LISTEN", "127.0.0.1")
PORT = int(os.environ.get("TLS_PROXY_PORT", "18768"))
USER = os.environ["TLS_RPC_USER"]
PASS = os.environ["TLS_RPC_PASS"]
DEBUG_LOG = os.environ.get("TLS_PROXY_DEBUG_LOG", "")

DEFAULT_TIMEOUT = float(os.environ.get("TLS_PROXY_TIMEOUT", "60"))
LONGPOLL_TIMEOUT = float(os.environ.get("TLS_PROXY_LONGPOLL_TIMEOUT", "300"))

ALLOWED = {
    "getblocktemplate",
    "pprpcsb",
    "getmininginfo",
    "getblockchaininfo",
    "getblockcount",
    "getblockhash",
    "getblock",
    "getblockheader",
    "getnetworkinfo",
    "getpeerinfo",
    "help",
    "uptime",
}

AUTH = base64.b64encode(f"{USER}:{PASS}".encode()).decode()


def _debug(line: str) -> None:
    if not DEBUG_LOG:
        return
    try:
        with open(DEBUG_LOG, "a", encoding="utf-8") as f:
            f.write(line + "\n")
    except OSError:
        pass


class Handler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def log_message(self, fmt, *args):
        pass

    def _send_json(self, payload: dict):
        body = json.dumps(payload).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(body)

    def _deny(self, code, msg, req_id=None):
        self._send_json({"result": None, "error": {"code": code, "message": msg}, "id": req_id})

    def do_POST(self):
        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length)
        hdr = self.headers.get("Authorization", "")
        expect = "Basic " + AUTH
        if hdr != expect:
            # telemerakiminer expects a JSON body; empty 401 → "invalid Json message".
            body = json.dumps(
                {
                    "result": None,
                    "error": {
                        "code": -32001,
                        "message": "unauthorized: use http://USER:PASSWORD@host:18768/ in -P",
                    },
                    "id": None,
                }
            ).encode()
            self.send_response(401)
            self.send_header("WWW-Authenticate", 'Basic realm="telestai-testnet"')
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Connection", "close")
            self.end_headers()
            self.wfile.write(body)
            return

        req_id = None
        try:
            req = json.loads(raw.decode() or "{}")
            req_id = req.get("id")
        except Exception:
            return self._deny(-32700, "parse error")

        method = req.get("method")
        if method not in ALLOWED:
            return self._deny(-32601, f"method not allowed: {method}", req_id)

        timeout = DEFAULT_TIMEOUT
        if method == "getblocktemplate":
            params = req.get("params") or []
            if params and isinstance(params[0], dict) and params[0].get("longpollid"):
                timeout = LONGPOLL_TIMEOUT

        upstream_req = urllib.request.Request(
            UPSTREAM,
            data=raw,
            headers={
                "Content-Type": "application/json",
                "Authorization": expect,
                "Connection": "keep-alive",
            },
            method="POST",
        )
        try:
            with urllib.request.urlopen(upstream_req, timeout=timeout) as resp:
                data = resp.read()
        except urllib.error.HTTPError as e:
            data = e.read() if e.fp else b""
            if not data:
                return self._deny(-1, f"upstream http {e.code}", req_id)
        except Exception as e:
            return self._deny(-1, f"upstream error: {e}", req_id)

        # telemerakiminer treats any non-error JSON-RPC result as **Accepted,
        # including BIP22 strings like "inconclusive" / "duplicate". Rewrite
        # those into errors so the miner refreshes work.
        if method == "pprpcsb":
            try:
                parsed = json.loads(data.decode() or "{}")
            except Exception:
                parsed = None
            if isinstance(parsed, dict) and parsed.get("error") is None:
                result = parsed.get("result")
                if result is True:
                    _debug(f"pprpcsb ok id={req_id}")
                    # Signal ops watchers that tip likely advanced (fresh jobs needed).
                    try:
                        tip_flag = os.environ.get(
                            "TLS_PROXY_TIP_FLAG",
                            "/home/chief/telestai/testnet-3.0.0/watch/tip_advanced_at",
                        )
                        with open(tip_flag, "w", encoding="utf-8") as f:
                            f.write(str(int(__import__("time").time())) + "\n")
                    except OSError:
                        pass
                elif result is False or isinstance(result, str):
                    msg = result if isinstance(result, str) else "rejected"
                    parsed = {
                        "result": None,
                        "error": {"code": -25, "message": str(msg)},
                        "id": parsed.get("id", req_id),
                    }
                    data = json.dumps(parsed).encode()
                    _debug(f"pprpcsb rewritten-reject id={req_id} msg={msg}")

        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(data)


if __name__ == "__main__":
    httpd = ThreadingHTTPServer((LISTEN, PORT), Handler)
    httpd.daemon_threads = True
    print(
        f"mining_rpc_proxy on {LISTEN}:{PORT} -> {UPSTREAM} "
        f"(timeout={DEFAULT_TIMEOUT}s longpoll={LONGPOLL_TIMEOUT}s)",
        flush=True,
    )
    httpd.serve_forever()
