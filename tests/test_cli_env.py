"""Integration tests for ``aliases-cli env``."""

from __future__ import annotations

from aliases_cli.config import Config
from aliases_cli.main import cli
from tests.conftest import make_project


class TestEnvBasic:
    def test_outputs_export_statements(self, runner, workspace, monkeypatch):
        make_project(workspace, "myapp")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        monkeypatch.setenv("PWD", str(workspace / "myapp"))

        result = runner.invoke(cli, ["env"])
        assert result.exit_code == 0
        assert "export PROJECT_NAME=" in result.stdout
        assert "export WEBPORT=" in result.stdout
        assert "export GQLPORT=" in result.stdout
        assert "export PROFILE=" in result.stdout

    def test_project_name_detected(self, runner, workspace, monkeypatch):
        make_project(workspace, "dispatch")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        monkeypatch.setenv("PWD", str(workspace / "dispatch"))

        result = runner.invoke(cli, ["env"])
        assert "export PROJECT_NAME='dispatch';" in result.stdout

    def test_custom_profile(self, runner, workspace, monkeypatch):
        make_project(workspace, "myapp")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        monkeypatch.setenv("PWD", str(workspace / "myapp"))

        result = runner.invoke(cli, ["env", "-e", "staging"])
        assert "export PROFILE='staging';" in result.stdout

    def test_no_port_offset_same_ports(self, runner, workspace, monkeypatch):
        make_project(workspace, "myapp")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        monkeypatch.setenv("PWD", str(workspace / "myapp"))

        result = runner.invoke(cli, ["env", "-n"])
        assert result.exit_code == 0
        exports = {}
        for line in result.stdout.splitlines():
            if line.startswith("export "):
                key, _, val = line[len("export "):].partition("=")
                exports[key] = val.strip("';")
        assert exports["WEBPORT"] == exports["GQLPORT"]

    def test_transfer_mode(self, runner, workspace, monkeypatch):
        make_project(workspace, "myapp")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        monkeypatch.setenv("PWD", str(workspace / "myapp"))

        result = runner.invoke(cli, ["env", "-t", "compressed"])
        assert "export GQLTRANSFERMODE='compressed';" in result.stdout

    def test_unknown_dir_uses_basename(self, runner, tmp_path, monkeypatch):
        project_dir = tmp_path / "special-project"
        project_dir.mkdir()
        monkeypatch.setenv("PWD", str(project_dir))

        result = runner.invoke(cli, ["env"])
        assert result.exit_code == 0
        assert "export PROJECT_NAME=" in result.stdout
