"""Configuration management for aliases-cli.

Singleton pattern — call Config.instance() after the module is imported.
Test isolation: call Config.set_test_config_directory(path) before any
Config.instance() call to redirect reads/writes to a temp directory.
"""

from __future__ import annotations

import copy
import json
from pathlib import Path
from typing import Any

# ---------------------------------------------------------------------------
# Defaults – mirrors config.template.json
# ---------------------------------------------------------------------------

DEFAULT_CONFIG: dict[str, Any] = {
    "general": {
        "editor": "code",
        "terminal_colors": True,
        "verbosity": "normal",
        "confirm_destructive_actions": True,
    },
    "code": {
        "vscode_flags": [],
        "reuse_window": True,
        "fallback_behavior": "auto",
        "preferred_component": "server",
    },
    "env": {
        "base_port": 3000,
        "port_offset": 100,
        "default_env": "dev",
    },
    "sync": {
        "enabled": False,
        "remote_url": "",
        "auto_sync": False,
        "sync_interval": 86400,
        "last_sync": 0,
        "method": "git",
    },
    "projects": {
        "workspace_directories": ["~/workspaces"],
        "shortcuts": {},
        "server_paths": {},
        "web_paths": {},
        "ignore": [],
        "default_paths": {
            "server": ["java/serverJava", "serverJava", "backend", "server"],
            "web": ["webapp", "webApp", "web", "frontend", "client"],
        },
    },
    "secrets": {
        "password_env_var": "ALIASES_MASTER_PASSWORD",
    },
    "prompt": {
        "enabled": True,
        "user_host_color": "bold_green",
        "default_path_color": "bold_blue",
        "path_replacements": [],
        "host_replacements": [],
        "user_replacements": [],
    },
}

# ---------------------------------------------------------------------------
# Config class
# ---------------------------------------------------------------------------


class Config:
    _instance: Config | None = None
    _test_dir: Path | None = None

    def __init__(self, config_dir: Path) -> None:
        self._config_dir = config_dir
        self._config_file = config_dir / "config.json"
        self._data: dict[str, Any] = {}
        self._load()

    # ------------------------------------------------------------------
    # Singleton access
    # ------------------------------------------------------------------

    @classmethod
    def instance(cls) -> "Config":
        if cls._instance is None:
            config_dir = cls._test_dir or Path.home() / ".config" / "aliases-cli"
            cls._instance = cls(config_dir)
        return cls._instance

    @classmethod
    def set_test_config_directory(cls, path: Path | str) -> None:
        """Redirect config to a temp directory for test isolation."""
        cls._test_dir = Path(path)
        cls._instance = None

    @classmethod
    def reset(cls) -> None:
        """Reset singleton (for tests)."""
        cls._instance = None
        cls._test_dir = None

    # ------------------------------------------------------------------
    # Load / save
    # ------------------------------------------------------------------

    def _load(self) -> None:
        data: dict[str, Any] = {}
        if self._config_file.exists():
            try:
                with open(self._config_file, encoding="utf-8") as fh:
                    data = json.load(fh)
            except (json.JSONDecodeError, OSError):
                data = {}
        # Merge: defaults fill in any missing keys
        self._data = _deep_merge(copy.deepcopy(DEFAULT_CONFIG), data)

    def save(self) -> None:
        self._config_dir.mkdir(parents=True, exist_ok=True)
        with open(self._config_file, "w", encoding="utf-8") as fh:
            json.dump(self._data, fh, indent=2)
            fh.write("\n")

    # ------------------------------------------------------------------
    # Access
    # ------------------------------------------------------------------

    def get(self, key: str, default: Any = None) -> Any:
        """Get value by dot-notation key (e.g. ``'general.editor'``)."""
        node: Any = self._data
        for part in key.split("."):
            if not isinstance(node, dict) or part not in node:
                return default
            node = node[part]
        return node

    def set(self, key: str, value: str) -> None:
        """Set value by dot-notation key, coercing to the stored type."""
        existing = self.get(key)
        coerced: Any = value
        if isinstance(existing, bool):
            coerced = value.lower() in ("true", "1", "yes", "on")
        elif isinstance(existing, int):
            try:
                coerced = int(value)
            except ValueError:
                pass
        elif isinstance(existing, float):
            try:
                coerced = float(value)
            except ValueError:
                pass
        elif isinstance(existing, list):
            # Accept JSON array strings or comma-separated values
            try:
                coerced = json.loads(value)
            except json.JSONDecodeError:
                coerced = [v.strip() for v in value.split(",")]

        parts = key.split(".")
        node = self._data
        for part in parts[:-1]:
            node = node.setdefault(part, {})
        node[parts[-1]] = coerced

    def all(self) -> dict[str, Any]:
        return copy.deepcopy(self._data)

    # ------------------------------------------------------------------
    # Convenience
    # ------------------------------------------------------------------

    @property
    def config_file(self) -> Path:
        return self._config_file

    @property
    def config_dir(self) -> Path:
        return self._config_dir


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _deep_merge(base: dict[str, Any], override: dict[str, Any]) -> dict[str, Any]:
    """Return a new dict that merges *override* into *base*.

    Keys present only in *base* are preserved (fills in defaults).
    Keys present only in *override* are included as-is (user additions).
    Nested dicts are merged recursively.
    """
    result = copy.deepcopy(base)
    for key, value in override.items():
        if key in result and isinstance(result[key], dict) and isinstance(value, dict):
            result[key] = _deep_merge(result[key], value)
        else:
            result[key] = copy.deepcopy(value)
    return result
