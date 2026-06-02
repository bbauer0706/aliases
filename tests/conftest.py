"""Shared pytest fixtures for aliases-cli tests.

Every test runs inside a fully isolated Config directory (tmp_path / "aliases-cli").
The real ~/.config/aliases-cli/ is NEVER touched.
"""

from __future__ import annotations

from pathlib import Path

import pytest
from click.testing import CliRunner

from aliases_cli.config import Config


# ---------------------------------------------------------------------------
# Core isolation fixture — runs for EVERY test automatically
# ---------------------------------------------------------------------------


@pytest.fixture(autouse=True)
def isolated_config(tmp_path: Path):
    """Redirect Config to a temp directory and reset the singleton after each test.

    Guarantees ~/.config/aliases-cli/ is never touched by any test.
    """
    Config.set_test_config_directory(tmp_path / "aliases-cli")
    yield Config.instance()
    Config.reset()


# ---------------------------------------------------------------------------
# CLI runner
# ---------------------------------------------------------------------------


@pytest.fixture
def runner() -> CliRunner:
    """Click test runner (Click 8.4+: stdout/stderr always separated)."""
    return CliRunner()


# ---------------------------------------------------------------------------
# Workspace helpers
# ---------------------------------------------------------------------------


@pytest.fixture
def workspace(tmp_path: Path) -> Path:
    """An empty workspace directory under tmp_path."""
    ws = tmp_path / "workspaces"
    ws.mkdir()
    return ws


def make_project(
    workspace: Path,
    name: str,
    server: str | None = None,
    web: str | None = None,
) -> Path:
    """Create a project directory (and optional components) inside workspace.

    Not a fixture — call directly: ``make_project(workspace, "myproj", server="backend")``.
    """
    p = workspace / name
    p.mkdir()
    if server:
        (p / server).mkdir(parents=True)
    if web:
        (p / web).mkdir(parents=True)
    return p
