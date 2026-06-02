"""Integration tests for ``aliases-cli code`` / ``aliases-cli c``.

subprocess.Popen is mocked — VS Code is never actually launched.
"""

from __future__ import annotations

from pathlib import Path
from unittest.mock import patch

from aliases_cli.config import Config
from aliases_cli.main import cli
from tests.conftest import make_project


def _no_popen():
    """Patch subprocess.Popen to prevent VS Code from actually launching."""
    return patch("aliases_cli.commands.code_navigator.subprocess.Popen")


class TestCodeNoArgs:
    def test_opens_home_dir(self, runner):
        with _no_popen() as mock_popen:
            result = runner.invoke(cli, ["code"])
        assert result.exit_code == 0
        args = mock_popen.call_args[0][0]
        assert str(Path.home()) in args


class TestCodeProjectLookup:
    def test_opens_project_root(self, runner, workspace):
        make_project(workspace, "dispatch")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]

        with _no_popen() as mock_popen:
            result = runner.invoke(cli, ["code", "dispatch"])
        assert result.exit_code == 0
        args = mock_popen.call_args[0][0]
        assert "dispatch" in " ".join(args)

    def test_case_insensitive_match(self, runner, workspace):
        make_project(workspace, "dispatch")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]

        with _no_popen() as mock_popen:
            result = runner.invoke(cli, ["code", "DISPATCH"])
        assert result.exit_code == 0
        args = mock_popen.call_args[0][0]
        assert "dispatch" in " ".join(args)

    def test_server_component(self, runner, workspace):
        make_project(workspace, "dispatch", server="backend")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]

        with _no_popen() as mock_popen:
            result = runner.invoke(cli, ["code", "dispatch", "s"])
        assert result.exit_code == 0
        args = mock_popen.call_args[0][0]
        assert "backend" in " ".join(args)

    def test_web_component(self, runner, workspace):
        make_project(workspace, "dispatch", web="frontend")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]

        with _no_popen() as mock_popen:
            result = runner.invoke(cli, ["code", "dispatch", "w"])
        assert result.exit_code == 0
        args = mock_popen.call_args[0][0]
        assert "frontend" in " ".join(args)

    def test_bracket_notation_opens_both(self, runner, workspace):
        make_project(workspace, "dispatch", server="backend", web="frontend")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]

        with _no_popen() as mock_popen:
            result = runner.invoke(cli, ["code", "dispatch[sw]"])
        assert result.exit_code == 0
        assert mock_popen.call_count == 2

    def test_shortcut_resolves(self, runner, workspace):
        make_project(workspace, "dispatch-backend")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        cfg._data["projects"]["shortcuts"] = {"dis": "dispatch-backend"}

        with _no_popen() as mock_popen:
            result = runner.invoke(cli, ["code", "dis"])
        assert result.exit_code == 0
        args = mock_popen.call_args[0][0]
        assert "dispatch-backend" in " ".join(args)


class TestCodeFallback:
    def test_unknown_path_falls_back_to_code(self, runner):
        with _no_popen() as mock_popen:
            result = runner.invoke(cli, ["code", "/some/random/path"])
        assert result.exit_code == 0
        args = mock_popen.call_args[0][0]
        assert args[0] == "code"
        assert "/some/random/path" in args


class TestCAlias:
    def test_c_alias_invokes_code(self, runner):
        with _no_popen() as mock_popen:
            result = runner.invoke(cli, ["c"])
        assert result.exit_code == 0
        assert mock_popen.called
