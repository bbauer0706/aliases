"""Integration tests for ``aliases-cli pwd``."""

from __future__ import annotations

from aliases_cli.config import Config
from aliases_cli.main import cli


class TestPwdBasic:
    def test_exits_zero(self, runner):
        result = runner.invoke(cli, ["pwd"])
        assert result.exit_code == 0
        assert result.stdout  # non-empty

    def test_no_color_strips_ansi(self, runner):
        result = runner.invoke(cli, ["pwd", "--no-color"])
        assert result.exit_code == 0
        assert "\033[" not in result.stdout
        assert "\x1b" not in result.stdout

    def test_ansi_present_without_no_color(self, runner):
        # click.echo(color=True) forces ANSI even when not a TTY
        result = runner.invoke(cli, ["pwd"], color=True)
        assert result.exit_code == 0
        assert "\x1b[" in result.stdout or "\033[" in result.stdout

    def test_ps1_has_readline_delimiters(self, runner):
        result = runner.invoke(cli, ["pwd", "--ps1"], color=True)
        assert result.exit_code == 0
        # \001 is the readline non-printing delimiter
        assert "\x01" in result.stdout

    def test_user_host_color_flag(self, runner):
        result = runner.invoke(cli, ["pwd", "--user-host-color"], color=True)
        assert result.exit_code == 0
        # Either an ANSI code or empty string when colors disabled
        assert "\x1b[" in result.stdout or result.stdout == ""


class TestPwdWithReplacement:
    def test_env_var_rule_applied(self, runner, monkeypatch, tmp_path):
        cfg = Config.instance()
        cfg._data["prompt"]["path_replacements"] = [
            {"env_var": "TESTROOT", "label": "MYROOT", "color": "bold_blue"}
        ]
        subdir = tmp_path / "subdir"
        subdir.mkdir()
        monkeypatch.setenv("TESTROOT", str(tmp_path))
        monkeypatch.setenv("PWD", str(subdir))

        result = runner.invoke(cli, ["pwd", "--no-color"])
        assert result.exit_code == 0
        assert "MYROOT" in result.stdout
        assert str(tmp_path) not in result.stdout

    def test_literal_path_rule_applied(self, runner, monkeypatch, tmp_path):
        cfg = Config.instance()
        cfg._data["prompt"]["path_replacements"] = [
            {"path": str(tmp_path), "label": "PROJECT", "color": "bold_cyan"}
        ]
        subdir = tmp_path / "src"
        subdir.mkdir()
        monkeypatch.setenv("PWD", str(subdir))

        result = runner.invoke(cli, ["pwd", "--no-color"])
        assert result.exit_code == 0
        assert "PROJECT" in result.stdout
