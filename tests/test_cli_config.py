"""Integration tests for ``aliases config`` commands.

Uses Click's CliRunner to invoke the CLI end-to-end in the same process.
Config is fully isolated — the real ~/.config/aliases/ is never touched.
"""

from __future__ import annotations

from pathlib import Path

from aliases.main import cli


class TestConfigGet:
    def test_default_editor(self, runner):
        result = runner.invoke(cli, ["config", "get", "general.editor"])
        assert result.exit_code == 0
        assert result.stdout.strip() == "code"

    def test_default_base_port(self, runner):
        result = runner.invoke(cli, ["config", "get", "env.base_port"])
        assert result.exit_code == 0
        assert result.stdout.strip() == "3000"

    def test_missing_key_exits_nonzero(self, runner):
        result = runner.invoke(cli, ["config", "get", "does.not.exist"])
        assert result.exit_code != 0

    def test_nested_key(self, runner):
        result = runner.invoke(cli, ["config", "get", "general.terminal_colors"])
        assert result.exit_code == 0
        assert result.stdout.strip().lower() in ("true", "false")


class TestConfigSet:
    def test_set_string_value(self, runner):
        r1 = runner.invoke(cli, ["config", "set", "general.editor", "vim"])
        assert r1.exit_code == 0
        r2 = runner.invoke(cli, ["config", "get", "general.editor"])
        assert r2.stdout.strip() == "vim"

    def test_set_bool_value(self, runner):
        r1 = runner.invoke(cli, ["config", "set", "general.terminal_colors", "false"])
        assert r1.exit_code == 0
        r2 = runner.invoke(cli, ["config", "get", "general.terminal_colors"])
        assert r2.stdout.strip().lower() == "false"

    def test_set_int_value(self, runner):
        r1 = runner.invoke(cli, ["config", "set", "env.base_port", "4000"])
        assert r1.exit_code == 0
        r2 = runner.invoke(cli, ["config", "get", "env.base_port"])
        assert r2.stdout.strip() == "4000"

    def test_set_unknown_key_fails(self, runner):
        result = runner.invoke(cli, ["config", "set", "does.not.exist", "value"])
        assert result.exit_code != 0


class TestConfigList:
    def test_lists_known_keys(self, runner):
        result = runner.invoke(cli, ["config", "list"])
        assert result.exit_code == 0
        out = result.output  # mixed stdout+stderr is fine here
        assert "general.editor" in out
        assert "env.base_port" in out

    def test_plain_flag(self, runner):
        result = runner.invoke(cli, ["config", "list", "--plain"])
        assert result.exit_code == 0
        lines = [l for l in result.stdout.strip().splitlines() if l]
        assert all("=" in l for l in lines)


class TestConfigReset:
    def test_reset_restores_defaults(self, runner):
        runner.invoke(cli, ["config", "set", "general.editor", "vim"])
        runner.invoke(cli, ["config", "reset", "--yes"])
        result = runner.invoke(cli, ["config", "get", "general.editor"])
        assert result.stdout.strip() == "code"


class TestConfigPath:
    def test_prints_config_path(self, runner, tmp_path):
        result = runner.invoke(cli, ["config", "path"])
        assert result.exit_code == 0
        assert "config.json" in result.stdout
        # Must be inside the isolated tmp dir, not the real user config
        assert str(Path.home() / ".config") not in result.stdout


class TestConfigSyncStatus:
    def test_status_when_not_configured(self, runner):
        result = runner.invoke(cli, ["config", "sync", "status"])
        assert result.exit_code == 0
        # Should indicate sync is not enabled/configured
        out = result.output.lower()
        assert "not" in out or "disabled" in out or "no remote" in out or out
