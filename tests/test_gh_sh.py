"""Tests for data/bash_aliases/gh.ali.sh.

Same contract as the glab twin: the file must survive being sourced before PATH
is complete, must never open a PR from main, and must clone flat into the
workspace directory. Plus the one behaviour gh does not share with glab — it
does not push the branch for you, so ghpr has to.
"""

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

import pytest

GH_SH = Path(__file__).resolve().parent.parent / "aliases" / "data" / "bash_aliases" / "gh.ali.sh"

pytestmark = pytest.mark.skipif(shutil.which("bash") is None, reason="bash not available")

# Stubs write their argv to $STUB_LOG so the tests can assert on the calls made.
_STUBS = {
    "gh": """#!/usr/bin/env bash
echo "gh $*" >> "$STUB_LOG"
case "$1 $2" in
    "repo clone") mkdir -p "$4" ;;
    "repo list")  printf '%s\\n' bbauer0706/aliases bbauer0706/sysmon ;;
esac
""",
    "fzf": """#!/usr/bin/env bash
echo "fzf $*" >> "$STUB_LOG"
cat > /dev/null
printf '%s\\n' "$FZF_PICK"
""",
    "aliases": """#!/usr/bin/env bash
printf '["%s"]\\n' "$WORKSPACE_DIR"
""",
    "git": """#!/usr/bin/env bash
echo "git $*" >> "$STUB_LOG"
case "$*" in
    *"--is-inside-work-tree"*) exit "${GIT_NOT_A_REPO:-0}" ;;
    *"@{u}"*)                  exit "${GIT_NO_UPSTREAM:-0}" ;;
    *"--abbrev-ref"*)          echo "${FAKE_BRANCH:-feature/x}" ;;
esac
""",
}


@pytest.fixture
def shell(tmp_path):
    """Return a runner that sources gh.ali.sh with stubbed binaries on PATH."""
    bin_dir = tmp_path / "bin"
    bin_dir.mkdir()
    for name, body in _STUBS.items():
        stub = bin_dir / name
        stub.write_text(body, encoding="utf-8")
        stub.chmod(0o755)

    workspace = tmp_path / "workspaces"
    workspace.mkdir()
    log = tmp_path / "calls.log"
    log.touch()

    def run(snippet: str, **env: str):
        proc = subprocess.run(
            ["bash", "--norc", "--noprofile", "-c", f"source {GH_SH}; {snippet}"],
            env={
                **os.environ,
                "PATH": f"{bin_dir}:{os.environ['PATH']}",
                "STUB_LOG": str(log),
                "WORKSPACE_DIR": str(workspace),
                "FZF_PICK": "bbauer0706/sysmon",
                **env,
            },
            capture_output=True,
            text=True,
        )
        return proc, log.read_text(encoding="utf-8")

    run.workspace = workspace
    return run


class TestSourcingIsPathIndependent:
    """~/.bashrc sources ~/.bash_aliases before its tail extends PATH, so gh may
    not be visible yet while this file is read. The commands must exist anyway."""

    def test_commands_exist_without_gh_on_path(self, tmp_path):
        proc = subprocess.run(
            # Absolute bash, empty PATH: nothing at all is discoverable, gh least of all.
            [shutil.which("bash"), "--norc", "--noprofile", "-c",
             f"source {GH_SH}; type -t ghc ghpr ghco gho"],
            env={"PATH": str(tmp_path), "HOME": str(tmp_path)},
            capture_output=True,
            text=True,
        )
        assert proc.stdout.split() == ["function"] * 4, proc.stderr

    def test_ghc_reports_the_missing_binary(self, shell):
        proc, _ = shell("PATH=/nonexistent ghc")
        assert proc.returncode != 0
        assert "gh is required" in proc.stderr


class TestGhpr:
    def test_refuses_on_main(self, shell):
        proc, calls = shell("ghpr", FAKE_BRANCH="main")
        assert proc.returncode != 0
        assert "pr create" not in calls
        assert "refusing" in proc.stderr

    def test_refuses_outside_a_repo(self, shell):
        proc, calls = shell("ghpr", GIT_NOT_A_REPO="1")
        assert proc.returncode != 0
        assert "pr create" not in calls

    def test_pushes_when_the_branch_has_no_upstream(self, shell):
        proc, calls = shell("ghpr", GIT_NO_UPSTREAM="1")
        assert proc.returncode == 0, proc.stderr
        assert "git push -u origin HEAD" in calls
        assert "gh pr create --fill" in calls

    def test_does_not_push_when_an_upstream_exists(self, shell):
        proc, calls = shell("ghpr --draft")
        assert proc.returncode == 0, proc.stderr
        assert "git push" not in calls
        assert "gh pr create --fill --draft" in calls


class TestGhc:
    def test_clones_flat_into_the_workspace_dir(self, shell):
        target = shell.workspace / "sysmon"
        proc, calls = shell("ghc")
        assert proc.returncode == 0, proc.stderr
        assert f"gh repo clone bbauer0706/sysmon {target}" in calls
        assert target.is_dir()

    def test_existing_checkout_is_not_recloned(self, shell):
        (shell.workspace / "sysmon").mkdir()
        proc, calls = shell("ghc")
        assert proc.returncode == 0, proc.stderr
        assert "repo clone" not in calls

    def test_lists_another_owner_when_gh_owner_is_set(self, shell):
        proc, calls = shell("ghc", GH_OWNER="someorg")
        assert proc.returncode == 0, proc.stderr
        assert "gh repo list someorg" in calls

    def test_lists_your_own_repos_by_default(self, shell):
        proc, calls = shell("ghc")
        assert proc.returncode == 0, proc.stderr
        assert "gh repo list --limit" in calls
