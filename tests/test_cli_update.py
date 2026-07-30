"""Tests for aliases update command."""

from __future__ import annotations

from unittest.mock import MagicMock, patch

import pytest
from click.testing import CliRunner

from aliases.main import cli


@pytest.fixture
def runner() -> CliRunner:
    return CliRunner()


def _which(binary: str) -> str | None:
    """shutil.which stub – update_cmd looks up both 'uv' and 'aliases'."""
    return {"uv": "/usr/bin/uv", "aliases": "/usr/bin/aliases"}.get(binary)


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
             patch("shutil.which", side_effect=_which), \
             patch("subprocess.run", return_value=mock_proc) as mock_run:
            result = runner.invoke(cli, ["update"])
        assert result.exit_code == 0
        assert "Updating" in result.output
        assert "Updated to v9.9.9" in result.output
        install_args, setup_args = (c[0][0] for c in mock_run.call_args_list)
        assert "uv" in install_args[0]
        assert "tool" in install_args
        assert "install" in install_args
        # Shell files are refreshed with the newly installed binary.
        assert setup_args == ["/usr/bin/aliases", "setup", "--update"]

    def test_setup_refresh_failure_warns_but_update_succeeds(self, runner):
        install_ok = MagicMock(returncode=0)
        setup_failed = MagicMock(returncode=1)
        with patch("aliases.commands.update_cmd._fetch_latest_tag", return_value="v9.9.9"), \
             patch("aliases.commands.update_cmd.__version__", "2.1.1"), \
             patch("shutil.which", side_effect=_which), \
             patch("subprocess.run", side_effect=[install_ok, setup_failed]):
            result = runner.invoke(cli, ["update"])
        assert result.exit_code == 0
        assert "Updated to v9.9.9" in result.output
        assert "run 'aliases setup --update'" in result.output

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
             patch("shutil.which", side_effect=_which), \
             patch("subprocess.run", return_value=mock_proc) as mock_run:
            result = runner.invoke(cli, ["update", "--force"])
        assert result.exit_code == 0
        assert mock_run.call_count == 2  # uv install, then setup --update

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
