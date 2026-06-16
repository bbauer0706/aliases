"""Tests for ProjectMapper."""

from __future__ import annotations

import pytest
from pathlib import Path

from aliases_cli.config import Config
from aliases_cli.project_mapper import ProjectMapper
from tests.conftest import make_project


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------


def _mk_project(workspace: Path, name: str, server: str | None = None, web: str | None = None) -> Path:
    p = workspace / name
    p.mkdir()
    if server:
        (p / server).mkdir(parents=True)
    if web:
        (p / web).mkdir(parents=True)
    return p


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------


class TestDiscoverProjects:
    def test_empty_workspace(self, workspace):
        cfg = Config.instance()
        cfg.set("projects.workspace_directories", str(workspace))
        mapper = ProjectMapper(cfg)
        assert mapper.discover_projects() == []

    def test_simple_project(self, workspace):
        _mk_project(workspace, "myproject")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        mapper = ProjectMapper(cfg)
        projects = mapper.discover_projects()
        assert len(projects) == 1
        assert projects[0].full_name == "myproject"
        assert projects[0].display_name == "myproject"

    def test_server_component_auto_detected(self, workspace):
        _mk_project(workspace, "myproject", server="backend")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        mapper = ProjectMapper(cfg)
        projects = mapper.discover_projects()
        assert projects[0].has_server_component
        assert projects[0].server_path is not None
        assert projects[0].server_path.name == "backend"

    def test_web_component_auto_detected(self, workspace):
        _mk_project(workspace, "myproject", web="webapp")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        mapper = ProjectMapper(cfg)
        projects = mapper.discover_projects()
        assert projects[0].has_web_component

    def test_hidden_dirs_ignored(self, workspace):
        _mk_project(workspace, ".hidden")
        _mk_project(workspace, "visible")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        mapper = ProjectMapper(cfg)
        projects = mapper.discover_projects()
        names = [p.full_name for p in projects]
        assert "visible" in names
        assert ".hidden" not in names

    def test_ignore_list(self, workspace):
        _mk_project(workspace, "ignored_project")
        _mk_project(workspace, "kept_project")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        cfg._data["projects"]["ignore"] = ["ignored_project"]
        mapper = ProjectMapper(cfg)
        names = [p.full_name for p in mapper.discover_projects()]
        assert "kept_project" in names
        assert "ignored_project" not in names

    def test_shortcut_applied(self, workspace):
        _mk_project(workspace, "my-long-project-name")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        cfg._data["projects"]["shortcuts"] = {"my-long-project-name": "mlpn"}
        mapper = ProjectMapper(cfg)
        projects = mapper.discover_projects()
        assert projects[0].display_name == "mlpn"
        assert projects[0].full_name == "my-long-project-name"


class TestFindProject:
    def test_find_by_full_name(self, workspace):
        _mk_project(workspace, "dispatch")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        mapper = ProjectMapper(cfg)
        p = mapper.find_project("dispatch")
        assert p is not None
        assert p.full_name == "dispatch"

    def test_find_by_display_name(self, workspace):
        _mk_project(workspace, "dispatch-backend")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        cfg._data["projects"]["shortcuts"] = {"dispatch-backend": "dis"}
        mapper = ProjectMapper(cfg)
        p = mapper.find_project("dis")
        assert p is not None

    def test_find_nonexistent_returns_none(self, workspace):
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        mapper = ProjectMapper(cfg)
        assert mapper.find_project("ghost") is None


class TestFindProjectByPath:
    def test_find_from_project_root(self, workspace):
        proj = _mk_project(workspace, "dispatch")
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        mapper = ProjectMapper(cfg)
        found = mapper.find_project_by_path(proj)
        assert found is not None
        assert found.full_name == "dispatch"

    def test_find_from_subdir(self, workspace):
        proj = _mk_project(workspace, "dispatch")
        subdir = proj / "src" / "deep"
        subdir.mkdir(parents=True)
        cfg = Config.instance()
        cfg._data["projects"]["workspace_directories"] = [str(workspace)]
        mapper = ProjectMapper(cfg)
        found = mapper.find_project_by_path(subdir)
        assert found is not None
        assert found.full_name == "dispatch"
