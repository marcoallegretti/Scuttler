# SPDX-FileCopyrightText: 2026 Marco Allegretti
# SPDX-License-Identifier: EUPL-1.2

import ctypes
import http.server
import os
import resource
import secrets
import signal
import socket
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from pathlib import Path

SCUTTLER, CAGE = sys.argv[1:3]


class Server(http.server.ThreadingHTTPServer):
    allow_reuse_address = True

    def __init__(self, port):
        super().__init__(("127.0.0.1", port), Handler)
        self.loads = 0
        self.pages = []
        self.condition = threading.Condition()

    def wait_loads(self, count, timeout):
        with self.condition:
            return self.condition.wait_for(lambda: self.loads >= count, timeout)


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/loaded":
            with self.server.condition:
                self.server.loads += 1
                self.server.condition.notify_all()
            body = b""
        else:
            with self.server.condition:
                self.server.pages.append(self.path)
            body = b"<!doctype html><script>fetch('/loaded')</script>"
        self.send_response(200)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, *args):
        pass


def prepare_child():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    PR_SET_PDEATHSIG = 1
    ctypes.CDLL(None, use_errno=True).prctl(PR_SET_PDEATHSIG, signal.SIGKILL)


def children(pid):
    found = []
    for task in Path(f"/proc/{pid}/task").glob("*"):
        try:
            found += map(int, (task / "children").read_text().split())
        except OSError:
            pass
    return found


def is_zygote(pid):
    try:
        return b"--type=zygote" in Path(f"/proc/{pid}/cmdline").read_bytes().split(b"\0")
    except OSError:
        return False


def renderers(pid):
    # Renderers are forked by the sandboxed zygote's init process and keep its command line.
    return [renderer
            for zygote in children(pid) if is_zygote(zygote)
            for init in children(zygote) if is_zygote(init)
            for renderer in children(init) if is_zygote(renderer)]


class WaylandSmokeTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        root = Path(self.tmp.name)
        runtime = root / "runtime"
        runtime.mkdir(mode=0o700)
        self.home = root / "home"
        self.home.mkdir()
        self.env = {
            "PATH": os.environ.get("PATH", "/usr/bin:/bin"),
            "HOME": str(self.home),
            "XDG_RUNTIME_DIR": str(runtime),
            "XDG_CACHE_HOME": str(self.home / ".cache"),
            "XDG_CONFIG_HOME": str(self.home / ".config"),
            "XDG_DATA_HOME": str(self.home / ".local/share"),
            "WLR_BACKENDS": "headless",
            "WLR_RENDERER": "pixman",
            "WLR_LIBINPUT_NO_DEVICES": "1",
            "QT_QPA_PLATFORM": "wayland",
            "QT_FORCE_STDERR_LOGGING": "1",
        }
        with socket.socket() as sock:
            sock.bind(("127.0.0.1", 0))
            self.port = sock.getsockname()[1]
        self.server = None
        self.cage = None

    def stop(self):
        for pid in children(self.cage.pid):
            os.kill(pid, signal.SIGTERM)
        try:
            self.cage.wait(10)
        except subprocess.TimeoutExpired:
            os.killpg(self.cage.pid, signal.SIGKILL)
            self.cage.wait()
        self.cage.stderr.close()
        self.cage = None

    def tearDown(self):
        if self.cage:
            self.stop()
        if self.server:
            self.server.shutdown()
            self.server.server_close()

    def test_recovers_and_keeps_secrets_off_disk(self):
        secret = secrets.token_hex(16)
        url = f"http://127.0.0.1:{self.port}/login?token={secret}"
        self.cage = subprocess.Popen([CAGE, "--", SCUTTLER, url], env=self.env, stdout=subprocess.DEVNULL,
                                     stderr=subprocess.PIPE, start_new_session=True, preexec_fn=prepare_child)
        failed = threading.Event()
        output = []

        def watch(stream):
            for line in stream:
                output.append(line)
                if b"ERR_CONNECTION_REFUSED" in line:
                    failed.set()

        watcher = threading.Thread(target=watch, args=(self.cage.stderr,), daemon=True)
        watcher.start()

        def wait_failed():
            deadline = time.monotonic() + 60
            while not failed.wait(0.5):
                if self.cage.poll() is not None or time.monotonic() > deadline:
                    return False
            return True

        self.assertTrue(wait_failed(), b"first load did not fail:\n" + b"".join(output))
        self.server = Server(self.port)
        threading.Thread(target=self.server.serve_forever, daemon=True).start()
        self.assertTrue(self.server.wait_loads(1, 30), "page not loaded after the server started")

        browser = children(self.cage.pid)
        targets = [pid for parent in browser for pid in renderers(parent)]
        self.assertTrue(targets)
        for pid in targets:
            os.kill(pid, signal.SIGKILL)
        self.assertTrue(self.server.wait_loads(2, 30), "page not reloaded after the renderer was killed")
        path = f"/login?token={secret}"
        self.assertEqual([page for page in self.server.pages if page != "/favicon.ico"], [path, path])

        self.stop()
        watcher.join(10)
        self.assertNotIn(secret.encode(), b"".join(output))
        leaks = [str(file) for file in self.home.rglob("*")
                 if file.is_file() and secret.encode() in file.read_bytes()]
        self.assertEqual(leaks, [])


if __name__ == "__main__":
    unittest.main(argv=sys.argv[:1])
