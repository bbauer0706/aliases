"""Tests for data/bash_aliases/eza.ali.sh.

The file overrides `ls`, so the contract is: aliases appear only when eza is
actually on PATH, and sourcing without eza is a silent no-op that leaves the
real `ls` (and basic.ali.sh's `la`) alone.
"""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

DATA = Path(__file__).resolve().parent.parent / "aliases" / "data" / "bash_aliases"
EZA_SH = DATA / "eza.ali.sh"
BASIC_SH = DATA / "basic.ali.sh"

BASH = shutil.which("bash")
LS = shutil.which("ls")

pytestmark = pytest.mark.skipif(BASH is None, reason="bash not available")


def _run(
    snippet: str,
    *,
    with_eza: bool,
    tmp_path: Path,
    icons: str | None = None,
) -> subprocess.CompletedProcess:
    """Source the alias files in an interactive bash with a controlled PATH.

    PATH holds *only* the tmp bin/, so `with_eza=False` is a real absence test
    even on a machine that has eza installed. -i is required: bash does not
    expand aliases in non-interactive shells.
    """
    bin_dir = tmp_path / "bin"
    bin_dir.mkdir(exist_ok=True)
    # coreutils ls, so `command ls` has something to reach.
    (bin_dir / "ls").symlink_to(LS)
    if with_eza:
        stub = bin_dir / "eza"
        stub.write_text(f'#!{BASH}\necho "eza $*"\n')
        stub.chmod(0o755)
    if icons is not None:
        config = bin_dir / "aliases"
        config.write_text(f'#!{BASH}\nprintf "%s\\n" {icons!r}\n')
        config.chmod(0o755)
    proc = subprocess.run(
        [BASH, "--norc", "--noprofile", "-ic",
         f"source {BASIC_SH}; source {EZA_SH}; {snippet}"],
        env={"PATH": str(bin_dir), "HOME": str(tmp_path)},
        capture_output=True,
        text=True,
    )
    # A non-tty interactive bash always warns about job control; drop that noise
    # so the tests can assert on real stderr.
    proc.stderr = "\n".join(
        ln for ln in proc.stderr.splitlines()
        if "job control" not in ln and "terminal process group" not in ln
    )
    return proc


class TestWithEza:
    def test_ls_becomes_eza(self, tmp_path):
        assert "eza --group-directories-first" in _run(
            "alias ls", with_eza=True, tmp_path=tmp_path
        ).stdout

    def test_la_overrides_the_basic_one(self, tmp_path):
        """eza.ali.sh sorts after basic.ali.sh, so its `la` must win."""
        out = _run("alias la", with_eza=True, tmp_path=tmp_path).stdout
        assert "eza" in out and "ls -alh" not in out

    def test_command_ls_still_reaches_coreutils(self, tmp_path):
        out = _run("command ls --version", with_eza=True, tmp_path=tmp_path).stdout
        assert "coreutils" in out and "eza" not in out

    def test_icons_mode_is_configurable(self, tmp_path):
        out = _run(
            "alias ls", with_eza=True, icons="always", tmp_path=tmp_path
        ).stdout
        assert "--icons=always" in out

    def test_invalid_icons_mode_uses_auto(self, tmp_path):
        out = _run(
            "alias ls", with_eza=True, icons="invalid", tmp_path=tmp_path
        ).stdout
        assert "--icons=auto" in out


class TestWithoutEza:
    def test_sourcing_is_a_silent_noop(self, tmp_path):
        proc = _run("echo DONE", with_eza=False, tmp_path=tmp_path)
        assert proc.returncode == 0
        assert "DONE" in proc.stdout
        assert proc.stderr.strip() == ""

    def test_ls_is_left_alone(self, tmp_path):
        # `alias ls` exits non-zero when no such alias exists.
        assert _run("alias ls", with_eza=False, tmp_path=tmp_path).returncode != 0

    def test_basic_la_survives(self, tmp_path):
        assert "ls -alh" in _run("alias la", with_eza=False, tmp_path=tmp_path).stdout
