"""Tests for aliases update command."""

from __future__ import annotations

from unittest.mock import MagicMock, patch

import pytest
from click.testing import CliRunner

from aliases.main import cli


@pytest.fixture
def runner() -> CliRunner:
    return CliRunner()


class TestUpdateCheck:
    def test_already_up_to_date(self, runner):
        with patch("aliases.commands.update_cmd._fetch_latest_tag", return_value="v2.1.1"), \
             patch("aliases.commands.update_cmd.__version__", "2.1.1"):
            result = runner.invoke(cli, ["update"])
        assert result.exit_code == 0
        assert "Already up to date" in result.output

    def test_update_available_triggers_install(self, runner):
        mock_proc = MagicMock()
        mock_proc.returncode = 0
        with patch("aliases.commands.update_cmd._fetch_latest_tag", return_value="v9.9.9"), \
             patch("aliases.commands.update_cmd.__version__", "2.1.1"), \
             patch("shutil.which", return_value="/usr/bin/uv"), \
             patch("subprocess.run", return_value=mock_proc) as mock_run:
            result = runner.invoke(cli, ["update"])
        assert result.exit_code == 0
        assert "Updating" in result.output
        assert "Updated to v9.9.9" in result.output
        mock_run.assert_called_once()
        args = mock_run.call_args[0][0]
        assert "uv" in args[0]
        assert "tool" in args
        assert "install" in args

    def test_check_only_exits_1_when_outdated(self, runner):
        with patch("aliases.commands.update_cmd._fetch_latest_tag", return_value="v9.9.9"), \
             patch("aliases.commands.update_cmd.__version__", "2.1.1"):
            result = runner.invoke(cli, ["update", "--check"])
        assert result.exit_code == 1
        assert "Update available" in result.output

    def test_check_only_exits_0_when_current(self, runner):
        with patch("aliases.commands.update_cmd._fetch_latest_tag", return_value="v2.1.1"), \
             patch("aliases.commands.update_cmd.__version__", "2.1.1"):
            result = runner.invoke(cli, ["update", "--check"])
        assert result.exit_code == 0
        assert "Already up to date" in result.output

    def test_force_reinstalls_when_current(self, runner):
        mock_proc = MagicMock()
        mock_proc.returncode = 0
        with patch("aliases.commands.update_cmd._fetch_latest_tag", return_value="v2.1.1"), \
             patch("aliases.commands.update_cmd.__version__", "2.1.1"), \
             patch("shutil.which", return_value="/usr/bin/uv"), \
             patch("subprocess.run", return_value=mock_proc) as mock_run:
            result = runner.invoke(cli, ["update", "--force"])
        assert result.exit_code == 0
        mock_run.assert_called_once()

    def test_github_unreachable_exits_1(self, runner):
        with patch("aliases.commands.update_cmd._fetch_latest_tag", return_value=None):
            result = runner.invoke(cli, ["update"])
        assert result.exit_code == 1

    def test_uv_not_found_exits_1(self, runner):
        mock_proc = MagicMock()
        mock_proc.returncode = 0
        with patch("aliases.commands.update_cmd._fetch_latest_tag", return_value="v9.9.9"), \
             patch("aliases.commands.update_cmd.__version__", "2.1.1"), \
             patch("shutil.which", return_value=None):
            result = runner.invoke(cli, ["update"])
        assert result.exit_code == 1

    def test_uv_failure_propagates_exit_code(self, runner):
        mock_proc = MagicMock()
        mock_proc.returncode = 2
        with patch("aliases.commands.update_cmd._fetch_latest_tag", return_value="v9.9.9"), \
             patch("aliases.commands.update_cmd.__version__", "2.1.1"), \
             patch("shutil.which", return_value="/usr/bin/uv"), \
             patch("subprocess.run", return_value=mock_proc):
            result = runner.invoke(cli, ["update"])
        assert result.exit_code == 2


class TestFetchLatestTag:
    def test_returns_first_v_tag(self):
        from aliases.commands.update_cmd import _fetch_latest_tag

        fake_response = b'[{"name": "v3.0.0"}, {"name": "v2.9.0"}]'
        mock_resp = MagicMock()
        mock_resp.read.return_value = fake_response
        mock_resp.__enter__ = lambda s: s
        mock_resp.__exit__ = MagicMock(return_value=False)

        with patch("urllib.request.urlopen", return_value=mock_resp):
            tag = _fetch_latest_tag()
        assert tag == "v3.0.0"

    def test_returns_none_on_network_error(self):
        from aliases.commands.update_cmd import _fetch_latest_tag

        with patch("urllib.request.urlopen", side_effect=OSError("timeout")):
            tag = _fetch_latest_tag()
        assert tag is None
