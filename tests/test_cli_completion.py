"""Integration tests for ``aliases-cli completion`` helpers."""

from __future__ import annotations

from aliases_cli.config import Config
from aliases_cli.main import cli
from tests.conftest import make_project


class TestCompletionProjects:
    def test_lists_projects(self, runner, workspace):
        make_project(workspace, "dispatch", server="backend")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]

        result = runner.invoke(cli, ["completion", "projects"])
        assert result.exit_code == 0
        assert "dispatch" in result.stdout

    def test_pipe_delimited_format(self, runner, workspace):
        make_project(workspace, "dispatch")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]

        result = runner.invoke(cli, ["completion", "projects"])
        line = next(l for l in result.stdout.splitlines() if "dispatch" in l)
        parts = line.split("|")
        # display_name|full_name|has_server|has_web
        assert len(parts) == 4
        assert parts[2] in ("0", "1")
        assert parts[3] in ("0", "1")

    def test_server_flag_correct(self, runner, workspace):
        make_project(workspace, "dispatch", server="backend")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]

        result = runner.invoke(cli, ["completion", "projects"])
        line = next(l for l in result.stdout.splitlines() if "dispatch" in l)
        parts = line.split("|")
        assert parts[2] == "1"  # has_server = true

    def test_empty_workspace_no_output(self, runner, workspace):
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        result = runner.invoke(cli, ["completion", "projects"])
        assert result.exit_code == 0
        assert result.stdout.strip() == ""


class TestCompletionComponents:
    def test_components_with_both(self, runner, workspace):
        make_project(workspace, "dispatch", server="backend", web="frontend")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]

        result = runner.invoke(cli, ["completion", "components", "dispatch"])
        assert result.exit_code == 0
        assert "s" in result.stdout
        assert "w" in result.stdout

    def test_server_only_project(self, runner, workspace):
        make_project(workspace, "dispatch", server="backend")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]

        result = runner.invoke(cli, ["completion", "components", "dispatch"])
        assert result.exit_code == 0
        assert "s" in result.stdout

    def test_unknown_project_empty(self, runner, workspace):
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        result = runner.invoke(cli, ["completion", "components", "nope"])
        assert result.exit_code == 0
        assert result.stdout.strip() == ""
