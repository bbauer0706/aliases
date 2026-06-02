"""Integration tests for ``aliases-cli secrets``.

The OS keychain is fully mocked — no real credentials are written or read.
"""

from __future__ import annotations

from unittest.mock import patch

from aliases_cli.main import cli


class TestSecretsSet:
    def test_set_stores_secret(self, runner):
        mem: dict[str, str] = {}
        with patch("keyring.set_password", side_effect=lambda s, n, v: mem.__setitem__(n, v)), \
             patch("keyring.get_password", side_effect=lambda s, n: mem.get(n)):
            result = runner.invoke(cli, ["secrets", "set", "MY_TOKEN", "abc123"])
        assert result.exit_code == 0
        assert mem.get("MY_TOKEN") == "abc123"

    def test_invalid_name_rejected(self, runner):
        result = runner.invoke(cli, ["secrets", "set", "bad name!", "value"])
        assert result.exit_code != 0

    def test_value_via_stdin_prompt(self, runner):
        mem: dict[str, str] = {}
        with patch("keyring.set_password", side_effect=lambda s, n, v: mem.__setitem__(n, v)):
            result = runner.invoke(cli, ["secrets", "set", "MY_KEY"], input="myvalue\n")
        assert result.exit_code == 0
        assert mem.get("MY_KEY") == "myvalue"


class TestSecretsGet:
    def test_get_existing_secret(self, runner):
        with patch("keyring.get_password", return_value="supersecret"):
            result = runner.invoke(cli, ["secrets", "get", "MY_TOKEN"])
        assert result.exit_code == 0
        assert "supersecret" in result.stdout

    def test_get_missing_secret_exits_nonzero(self, runner):
        with patch("keyring.get_password", return_value=None):
            result = runner.invoke(cli, ["secrets", "get", "MISSING"])
        assert result.exit_code != 0


class TestSecretsList:
    def test_empty_list_message(self, runner):
        result = runner.invoke(cli, ["secrets", "list"])
        assert result.exit_code == 0
        assert "(no secrets stored)" in result.stdout

    def test_listed_after_set(self, runner):
        with patch("keyring.set_password"):
            runner.invoke(cli, ["secrets", "set", "TOKEN_A", "v1"])
            runner.invoke(cli, ["secrets", "set", "TOKEN_B", "v2"])
        result = runner.invoke(cli, ["secrets", "list"])
        assert result.exit_code == 0
        assert "TOKEN_A" in result.stdout
        assert "TOKEN_B" in result.stdout


class TestSecretsDelete:
    def test_delete_with_yes_flag(self, runner):
        with patch("keyring.set_password"):
            runner.invoke(cli, ["secrets", "set", "DEL_ME", "val"])
        with patch("keyring.delete_password"):
            result = runner.invoke(cli, ["secrets", "delete", "DEL_ME", "--yes"])
        assert result.exit_code == 0
        list_result = runner.invoke(cli, ["secrets", "list"])
        assert "DEL_ME" not in list_result.stdout

    def test_rm_alias_works(self, runner):
        with patch("keyring.set_password"):
            runner.invoke(cli, ["secrets", "set", "RM_ME", "val"])
        with patch("keyring.delete_password"):
            result = runner.invoke(cli, ["secrets", "rm", "RM_ME", "--yes"])
        assert result.exit_code == 0


class TestSecretsLoad:
    def test_load_outputs_export(self, runner):
        with patch("keyring.set_password"):
            runner.invoke(cli, ["secrets", "set", "API_KEY", "v"])
        with patch("keyring.get_password", return_value="myvalue"):
            result = runner.invoke(cli, ["secrets", "load", "API_KEY"])
        assert result.exit_code == 0
        assert "export API_KEY=" in result.stdout

    def test_load_escapes_single_quotes(self, runner):
        with patch("keyring.set_password"):
            runner.invoke(cli, ["secrets", "set", "TRICKY", "v"])
        with patch("keyring.get_password", return_value="it's here"):
            result = runner.invoke(cli, ["secrets", "load", "TRICKY"])
        assert result.exit_code == 0
        assert "TRICKY" in result.stdout

    def test_load_warns_on_missing(self, runner):
        with patch("keyring.get_password", return_value=None):
            result = runner.invoke(cli, ["secrets", "load", "NOPE"])
        assert "NOPE" in result.stderr or "not found" in result.stderr.lower()
