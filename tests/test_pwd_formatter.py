"""Tests for pwd_formatter."""

from __future__ import annotations

import pytest
from pathlib import Path
from unittest.mock import patch

from aliases_cli.config import Config
from aliases_cli.pwd_formatter import format_pwd, get_user_host_color, get_user_host_label, ANSI_COLORS


class TestFormatPwd:
    def test_replaces_home_with_tilde(self, tmp_path):
        cfg = Config.instance()
        home = str(Path.home())
        result = format_pwd(cfg, f"{home}/projects/foo", no_color=True)
        assert result == "~/projects/foo"

    def test_path_not_under_home(self, tmp_path):
        cfg = Config.instance()
        result = format_pwd(cfg, "/etc/nginx", no_color=True)
        assert result == "/etc/nginx"

    def test_env_var_replacement(self, tmp_path):
        cfg = Config.instance()
        cfg._data["prompt"]["path_replacements"] = [
            {"env_var": "TESTROOT", "label": "TESTROOT", "color": "bold_yellow"}
        ]
        with patch.dict("os.environ", {"TESTROOT": "/opt/test"}):
            result = format_pwd(cfg, "/opt/test/subdir", no_color=True)
        assert result == "TESTROOT/subdir"

    def test_literal_path_replacement(self, tmp_path):
        cfg = Config.instance()
        cfg._data["prompt"]["path_replacements"] = [
            {"path": "/opt/platform", "label": "PLATFORM", "color": "bold_cyan"}
        ]
        result = format_pwd(cfg, "/opt/platform/src", no_color=True)
        assert result == "PLATFORM/src"

    def test_first_rule_wins(self, tmp_path):
        cfg = Config.instance()
        cfg._data["prompt"]["path_replacements"] = [
            {"path": "/opt/a", "label": "A", "color": ""},
            {"path": "/opt/a/sub", "label": "B", "color": ""},
        ]
        result = format_pwd(cfg, "/opt/a/sub/file", no_color=True)
        assert result == "A/sub/file"

    def test_color_codes_present_when_enabled(self, tmp_path):
        cfg = Config.instance()
        result = format_pwd(cfg, "/etc/test", no_color=False)
        # Should contain ANSI reset code
        assert "\033[" in result

    def test_no_color_flag(self, tmp_path):
        cfg = Config.instance()
        result = format_pwd(cfg, "/etc/test", no_color=True)
        assert "\033[" not in result

    def test_ps1_wrapping(self, tmp_path):
        cfg = Config.instance()
        result = format_pwd(cfg, "/etc/test", ps1=True)
        # PS1 mode wraps codes in \001...\002
        assert "\001" in result or result == "/etc/test"

    def test_terminal_colors_off_suppresses_codes(self, tmp_path):
        cfg = Config.instance()
        cfg._data["general"]["terminal_colors"] = False
        result = format_pwd(cfg, "/etc/test")
        assert "\033[" not in result


class TestGetUserHostColor:
    def test_returns_ansi_code(self, tmp_path):
        cfg = Config.instance()
        result = get_user_host_color(cfg)
        assert result in ANSI_COLORS.values() or result == ""

    def test_respects_terminal_colors_off(self, tmp_path):
        cfg = Config.instance()
        cfg._data["general"]["terminal_colors"] = False
        assert get_user_host_color(cfg) == ""


class TestGetUserHostLabel:
    def test_returns_user_at_host(self):
        cfg = Config.instance()
        with patch("socket.gethostname", return_value="myhost"), \
             patch.dict("os.environ", {"USER": "alice"}):
            result = get_user_host_label(cfg, no_color=True)
        assert result == "alice@myhost"

    def test_host_replacement_applied(self):
        cfg = Config.instance()
        cfg._data["prompt"]["host_replacements"] = [
            {"hostname": "ip-10-80-1-32", "label": "prod"}
        ]
        with patch("socket.gethostname", return_value="ip-10-80-1-32"), \
             patch.dict("os.environ", {"USER": "alice"}):
            result = get_user_host_label(cfg, no_color=True)
        assert result == "alice@prod"

    def test_user_replacement_applied(self):
        cfg = Config.instance()
        cfg._data["prompt"]["user_replacements"] = [
            {"username": "benedikt.bauer", "label": "bb"}
        ]
        with patch("socket.gethostname", return_value="myhost"), \
             patch.dict("os.environ", {"USER": "benedikt.bauer"}):
            result = get_user_host_label(cfg, no_color=True)
        assert result == "bb@myhost"

    def test_both_replacements_applied(self):
        cfg = Config.instance()
        cfg._data["prompt"]["host_replacements"] = [{"hostname": "ip-10-80-1-32", "label": "prod"}]
        cfg._data["prompt"]["user_replacements"] = [{"username": "benedikt.bauer", "label": "bb"}]
        with patch("socket.gethostname", return_value="ip-10-80-1-32"), \
             patch.dict("os.environ", {"USER": "benedikt.bauer"}):
            result = get_user_host_label(cfg, no_color=True)
        assert result == "bb@prod"

    def test_no_match_uses_real_values(self):
        cfg = Config.instance()
        cfg._data["prompt"]["host_replacements"] = [{"hostname": "other-host", "label": "x"}]
        cfg._data["prompt"]["user_replacements"] = [{"username": "other-user", "label": "y"}]
        with patch("socket.gethostname", return_value="myhost"), \
             patch.dict("os.environ", {"USER": "alice"}):
            result = get_user_host_label(cfg, no_color=True)
        assert result == "alice@myhost"

    def test_color_codes_present(self):
        cfg = Config.instance()
        with patch("socket.gethostname", return_value="myhost"), \
             patch.dict("os.environ", {"USER": "alice"}):
            result = get_user_host_label(cfg, no_color=False)
        assert "\033[" in result

    def test_no_color_flag(self):
        cfg = Config.instance()
        with patch("socket.gethostname", return_value="myhost"), \
             patch.dict("os.environ", {"USER": "alice"}):
            result = get_user_host_label(cfg, no_color=True)
        assert "\033[" not in result

    def test_ps1_wrapping(self):
        cfg = Config.instance()
        with patch("socket.gethostname", return_value="myhost"), \
             patch.dict("os.environ", {"USER": "alice"}):
            result = get_user_host_label(cfg, ps1=True)
        assert "\001" in result

    def test_terminal_colors_off_suppresses_codes(self):
        cfg = Config.instance()
        cfg._data["general"]["terminal_colors"] = False
        with patch("socket.gethostname", return_value="myhost"), \
             patch.dict("os.environ", {"USER": "alice"}):
            result = get_user_host_label(cfg)
        assert "\033[" not in result
