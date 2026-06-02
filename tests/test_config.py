"""Tests for the Config singleton."""

from __future__ import annotations

import json
import pytest
from pathlib import Path

from aliases_cli.config import Config, DEFAULT_CONFIG, _deep_merge


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------


class TestDefaults:
    def test_default_editor(self):
        assert Config.instance().get("general.editor") == "code"

    def test_default_base_port(self):
        assert Config.instance().get("env.base_port") == 3000

    def test_unknown_key_returns_none(self):
        assert Config.instance().get("does.not.exist") is None

    def test_unknown_key_returns_default(self):
        assert Config.instance().get("does.not.exist", "fallback") == "fallback"


class TestSetAndSave:
    def test_set_string(self):
        cfg = Config.instance()
        cfg.set("general.editor", "vim")
        assert cfg.get("general.editor") == "vim"

    def test_set_bool_true(self):
        cfg = Config.instance()
        cfg.set("general.terminal_colors", "false")
        assert cfg.get("general.terminal_colors") is False

    def test_set_bool_false(self):
        cfg = Config.instance()
        cfg.set("code.reuse_window", "true")
        assert cfg.get("code.reuse_window") is True

    def test_set_int(self):
        cfg = Config.instance()
        cfg.set("env.base_port", "8080")
        assert cfg.get("env.base_port") == 8080

    def test_save_and_reload(self, tmp_path):
        cfg = Config.instance()
        cfg.set("general.editor", "nvim")
        cfg.save()

        # Force reload
        Config.reset()
        Config.set_test_config_directory(cfg.config_dir)
        cfg2 = Config.instance()
        assert cfg2.get("general.editor") == "nvim"


class TestDeepMerge:
    def test_missing_key_filled_from_base(self):
        base = {"a": {"x": 1, "y": 2}}
        override = {"a": {"x": 99}}
        result = _deep_merge(base, override)
        assert result["a"]["x"] == 99
        assert result["a"]["y"] == 2  # preserved from base

    def test_extra_key_in_override(self):
        base = {"a": 1}
        override = {"a": 1, "b": 2}
        result = _deep_merge(base, override)
        assert result["b"] == 2

    def test_nested_merge(self):
        base = {"outer": {"inner": {"deep": "default"}}}
        override = {"outer": {"inner": {"new_key": "value"}}}
        result = _deep_merge(base, override)
        assert result["outer"]["inner"]["deep"] == "default"
        assert result["outer"]["inner"]["new_key"] == "value"


class TestCorruptConfig:
    def test_invalid_json_falls_back_to_defaults(self, tmp_path):
        cfg_dir = tmp_path / "aliases-cli"
        cfg_dir.mkdir()
        (cfg_dir / "config.json").write_text("{ invalid json }", encoding="utf-8")

        Config.reset()
        Config.set_test_config_directory(cfg_dir)
        cfg = Config.instance()
        assert cfg.get("general.editor") == "code"
