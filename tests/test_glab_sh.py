"""Tests for data/bash_aliases/glab.ali.sh.

The wrapper is thin, but three things must hold: it never opens a merge request
from main, it does not re-hit the API while the repo cache is fresh, and it
clones flat into the workspace directory (the project mapper only scans one
level deep). These run the real script under bash with stubbed binaries.
"""

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

import pytest

GLAB_SH = Path(__file__).resolve().parent.parent / "aliases" / "data" / "bash_aliases" / "glab.ali.sh"

pytestmark = pytest.mark.skipif(shutil.which("bash") is None, reason="bash not available")

# Stubs write their argv to $STUB_LOG so the tests can assert on the calls made.
_STUBS = {
    "glab": """#!/usr/bin/env bash
echo "glab $*" >> "$STUB_LOG"
case "$1 $2" in
    "repo clone") mkdir -p "$4" ;;
    "api "*|"api") cat "$GLAB_API_NDJSON" ;;
esac
""",
    "fzf": """#!/usr/bin/env bash
echo "fzf $*" >> "$STUB_LOG"
cat > /dev/null
printf '%s\\n' "$FZF_PICK"
""",
    "aliases": """#!/usr/bin/env bash
echo "aliases $*" >> "$STUB_LOG"
printf '["%s"]\\n' "$WORKSPACE_DIR"
""",
    "git": """#!/usr/bin/env bash
echo "git $*" >> "$STUB_LOG"
case "$*" in
    *"--is-inside-work-tree"*) exit "${GIT_NOT_A_REPO:-0}" ;;
    *"--abbrev-ref"*)          echo "${FAKE_BRANCH:-feature/x}" ;;
esac
""",
}


@pytest.fixture
def shell(tmp_path):
    """Return a runner that sources glab.ali.sh with stubbed binaries on PATH."""
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

    ndjson = tmp_path / "projects.ndjson"
    ndjson.write_text(
        '{"path_with_namespace": "evotess/syncrotess/frontend/lander"}\n'
        '{"path_with_namespace": "evotess/platform/ci-components/python"}\n',
        encoding="utf-8",
    )

    cache_dir = tmp_path / "cache" / "aliases"
    cache_dir.mkdir(parents=True)
    cache_file = cache_dir / "glab-repos-evotess.txt"

    def run(snippet: str, **env: str):
        proc = subprocess.run(
            ["bash", "--norc", "--noprofile", "-c", f"source {GLAB_SH}; {snippet}"],
            env={
                **os.environ,
                "PATH": f"{bin_dir}:{os.environ['PATH']}",
                "XDG_CACHE_HOME": str(tmp_path / "cache"),
                "STUB_LOG": str(log),
                "GLAB_API_NDJSON": str(ndjson),
                "WORKSPACE_DIR": str(workspace),
                "FZF_PICK": "evotess/syncrotess/frontend/lander",
                **env,
            },
            capture_output=True,
            text=True,
        )
        return proc, log.read_text(encoding="utf-8")

    run.workspace = workspace
    run.cache_file = cache_file
    return run


class TestGlmr:
    def test_refuses_on_main(self, shell):
        proc, calls = shell("glmr", FAKE_BRANCH="main")
        assert proc.returncode != 0
        assert "mr create" not in calls
        assert "refusing" in proc.stderr

    def test_refuses_outside_a_repo(self, shell):
        proc, calls = shell("glmr", GIT_NOT_A_REPO="1")
        assert proc.returncode != 0
        assert "mr create" not in calls

    def test_creates_with_defaults_on_a_feature_branch(self, shell):
        proc, calls = shell("glmr --draft", FAKE_BRANCH="feature/ST-1")
        assert proc.returncode == 0, proc.stderr
        assert (
            "glab mr create --fill --yes --remove-source-branch "
            "--squash-before-merge --draft" in calls
        )


class TestGlc:
    def test_fresh_cache_is_not_refetched(self, shell):
        shell.cache_file.write_text("evotess/syncrotess/frontend/lander\n", encoding="utf-8")
        proc, calls = shell("glc")
        assert proc.returncode == 0, proc.stderr
        assert "glab api" not in calls

    def test_stale_cache_is_refetched(self, shell):
        shell.cache_file.write_text("evotess/old/repo\n", encoding="utf-8")
        os.utime(shell.cache_file, (0, 0))  # epoch — well past the 24h TTL
        proc, calls = shell("glc")
        assert proc.returncode == 0, proc.stderr
        assert "glab api" in calls
        assert "ci-components/python" in shell.cache_file.read_text(encoding="utf-8")

    def test_clones_flat_into_the_workspace_dir(self, shell):
        shell.cache_file.write_text("evotess/syncrotess/frontend/lander\n", encoding="utf-8")
        target = shell.workspace / "lander"
        proc, calls = shell("glc")
        assert proc.returncode == 0, proc.stderr
        assert f"glab repo clone evotess/syncrotess/frontend/lander {target}" in calls
        assert target.is_dir()

    def test_existing_checkout_is_not_recloned(self, shell):
        shell.cache_file.write_text("evotess/syncrotess/frontend/lander\n", encoding="utf-8")
        (shell.workspace / "lander").mkdir()
        proc, calls = shell("glc")
        assert proc.returncode == 0, proc.stderr
        assert "repo clone" not in calls

    def test_failed_refresh_keeps_the_old_cache(self, shell):
        shell.cache_file.write_text("evotess/old/repo\n", encoding="utf-8")
        os.utime(shell.cache_file, (0, 0))
        empty = shell.cache_file.parent / "empty.ndjson"
        empty.touch()
        proc, _ = shell("glc", GLAB_API_NDJSON=str(empty))
        assert proc.returncode == 0, proc.stderr
        assert shell.cache_file.read_text(encoding="utf-8") == "evotess/old/repo\n"
