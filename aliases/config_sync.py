"""Remote config sync – git, rsync, file, and http methods."""

from __future__ import annotations

import shutil
import time
import urllib.request
from pathlib import Path
from typing import TYPE_CHECKING

from aliases import process_utils

if TYPE_CHECKING:
    from aliases.config import Config

SUPPORTED_METHODS = ("git", "rsync", "file", "http")


class ConfigSync:
    def __init__(self, config: "Config") -> None:
        self._config = config

    # ------------------------------------------------------------------
    # Setup
    # ------------------------------------------------------------------

    def setup(self, url: str, method: str = "git", repo_config_path: str | None = None) -> bool:
        if method not in SUPPORTED_METHODS:
            _err(f"Unknown sync method '{method}'. Supported: {', '.join(SUPPORTED_METHODS)}")
            return False

        self._config.set("sync.enabled", "true")
        self._config.set("sync.remote_url", url)
        self._config.set("sync.method", method)
        if repo_config_path is not None:
            self._config.set("sync.repo_config_path", repo_config_path)
        self._config.save()
        path_info = f", config path={self._config.get('sync.repo_config_path', 'config.json')}"
        _ok(f"Sync configured: method={method}, url={url}{path_info}")
        return True

    # ------------------------------------------------------------------
    # Pull
    # ------------------------------------------------------------------

    def pull(self) -> bool:
        if not self._assert_enabled():
            return False

        method = self._config.get("sync.method", "git")
        url = self._config.get("sync.remote_url", "")

        handlers = {
            "git": self._pull_git,
            "rsync": self._pull_rsync,
            "file": self._pull_file,
            "http": self._pull_http,
        }
        handler = handlers.get(method)
        if handler is None:
            _err(f"Unknown sync method: {method}")
            return False

        ok = handler(url)
        if ok:
            self._update_last_sync()
        return ok

    def _pull_git(self, url: str) -> bool:
        repo_dir = self._git_cache_dir()
        if not repo_dir.exists():
            _info(f"Cloning config repo from {url}…")
            code, _, err = process_utils.execute(["git", "clone", url, str(repo_dir)])
            if code != 0:
                _err(f"git clone failed: {err.strip()}")
                return False
        else:
            _info("Pulling latest config from remote…")
            code, _, err = process_utils.execute(["git", "pull"], cwd=repo_dir)
            if code != 0:
                _err(f"git pull failed: {err.strip()}")
                return False

        repo_cfg_path = self._config.get("sync.repo_config_path", "config.json")
        remote_cfg = repo_dir / repo_cfg_path
        if not remote_cfg.exists():
            _err(f"Remote repo does not contain {repo_cfg_path}")
            return False

        shutil.copy2(remote_cfg, self._config.config_file)
        _ok("Config pulled from remote.")
        return True

    def _pull_rsync(self, url: str) -> bool:
        if not process_utils.command_exists("rsync"):
            _err("rsync not found on PATH")
            return False
        code, _, err = process_utils.execute(
            ["rsync", "-az", f"{url}/config.json", str(self._config.config_file)]
        )
        if code != 0:
            _err(f"rsync failed: {err.strip()}")
            return False
        _ok("Config pulled via rsync.")
        return True

    def _pull_file(self, url: str) -> bool:
        src = Path(url).expanduser()
        if not src.exists():
            _err(f"Source path does not exist: {src}")
            return False
        shutil.copy2(src, self._config.config_file)
        _ok(f"Config copied from {src}.")
        return True

    def _pull_http(self, url: str) -> bool:
        try:
            with urllib.request.urlopen(url, timeout=15) as resp:  # noqa: S310
                data = resp.read()
            with open(self._config.config_file, "wb") as fh:
                fh.write(data)
            _ok(f"Config fetched from {url}.")
            return True
        except Exception as exc:
            _err(f"HTTP fetch failed: {exc}")
            return False

    # ------------------------------------------------------------------
    # Push
    # ------------------------------------------------------------------

    def push(self) -> bool:
        if not self._assert_enabled():
            return False

        method = self._config.get("sync.method", "git")
        url = self._config.get("sync.remote_url", "")

        if method == "http":
            _err("HTTP sync is read-only — push is not supported.")
            return False

        handlers = {
            "git": self._push_git,
            "rsync": self._push_rsync,
            "file": self._push_file,
        }
        handler = handlers.get(method)
        if handler is None:
            _err(f"Unknown sync method: {method}")
            return False

        ok = handler(url)
        if ok:
            self._update_last_sync()
        return ok

    def _push_git(self, _url: str) -> bool:
        import socket

        repo_dir = self._git_cache_dir()
        if not repo_dir.exists():
            _err("No local cache — run 'config sync pull' first.")
            return False

        repo_cfg_path = self._config.get("sync.repo_config_path", "config.json")
        dest = repo_dir / repo_cfg_path
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(self._config.config_file, dest)

        hostname = socket.gethostname()
        msg = f"chore: sync config from {hostname}"

        process_utils.execute(["git", "add", repo_cfg_path], cwd=repo_dir)
        code, _, err = process_utils.execute(
            ["git", "commit", "-m", msg], cwd=repo_dir
        )
        if code != 0 and "nothing to commit" not in err:
            _err(f"git commit failed: {err.strip()}")
            return False

        code, _, err = process_utils.execute(["git", "push"], cwd=repo_dir)
        if code != 0:
            _err(f"git push failed: {err.strip()}")
            return False

        _ok("Config pushed to remote.")
        return True

    def _push_rsync(self, url: str) -> bool:
        if not process_utils.command_exists("rsync"):
            _err("rsync not found on PATH")
            return False
        code, _, err = process_utils.execute(
            ["rsync", "-az", str(self._config.config_file), f"{url}/config.json"]
        )
        if code != 0:
            _err(f"rsync failed: {err.strip()}")
            return False
        _ok("Config pushed via rsync.")
        return True

    def _push_file(self, url: str) -> bool:
        dest = Path(url).expanduser()
        dest.mkdir(parents=True, exist_ok=True)
        shutil.copy2(self._config.config_file, dest / "config.json")
        _ok(f"Config copied to {dest}.")
        return True

    # ------------------------------------------------------------------
    # Status
    # ------------------------------------------------------------------

    def status(self) -> bool:
        enabled = self._config.get("sync.enabled", False)
        method = self._config.get("sync.method", "git")
        url = self._config.get("sync.remote_url", "")
        last_sync = self._config.get("sync.last_sync", 0)
        repo_cfg_path = self._config.get("sync.repo_config_path", "config.json")

        print(f"Sync enabled:   {enabled}")
        print(f"Method:         {method}")
        print(f"Remote URL:     {url or '(not set)'}")
        print(f"Config path:    {repo_cfg_path}")
        if last_sync:
            elapsed = int(time.time()) - last_sync
            print(f"Last sync:      {_human_elapsed(elapsed)} ago")
        else:
            print("Last sync:      never")
        return True

    # ------------------------------------------------------------------
    # Helpers
    # ------------------------------------------------------------------

    def _git_cache_dir(self) -> Path:
        return self._config.config_dir / "cache" / "sync" / "config-repo"

    def _update_last_sync(self) -> None:
        self._config.set("sync.last_sync", str(int(time.time())))
        self._config.save()

    def _assert_enabled(self) -> bool:
        if not self._config.get("sync.enabled", False):
            _err("Sync is not enabled. Run: aliases config sync setup <url>")
            return False
        url = self._config.get("sync.remote_url", "")
        if not url:
            _err("No remote URL configured. Run: aliases config sync setup <url>")
            return False
        return True


# ---------------------------------------------------------------------------
# Output helpers
# ---------------------------------------------------------------------------


def _ok(msg: str) -> None:
    print(f"\033[32m✓\033[0m {msg}")


def _err(msg: str) -> None:
    import sys

    print(f"\033[31m✗\033[0m {msg}", file=sys.stderr)


def _info(msg: str) -> None:
    print(f"\033[34mℹ\033[0m {msg}")


def _human_elapsed(seconds: int) -> str:
    if seconds < 60:
        return f"{seconds}s"
    if seconds < 3600:
        return f"{seconds // 60}m"
    if seconds < 86400:
        return f"{seconds // 3600}h"
    return f"{seconds // 86400}d"
