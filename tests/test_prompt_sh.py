"""Tests for data/shell/prompt.sh – the PS1 must never reach child processes.

Spring's placeholder resolver (and anything else that scans the environment)
aborts on a PS1 containing ``${...}``. These run the real script under bash.
"""

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

import pytest

PROMPT_SH = Path(__file__).resolve().parent.parent / "aliases" / "data" / "shell" / "prompt.sh"

# A parent that exported PS1 – exactly what a pre-fix VS Code server hands down.
POISONED_PS1 = "${_ALIASES_PROMPT_CACHE}\\$ "

pytestmark = pytest.mark.skipif(shutil.which("bash") is None, reason="bash not available")


def _run(snippet: str, **env: str) -> str:
    """Source prompt.sh in an interactive bash and return stdout.

    --norc/--noprofile keeps the *installed* ~/.config/aliases copy out of it;
    -i is required because bash discards PS1 from the env when non-interactive.
    """
    proc = subprocess.run(
        ["bash", "--norc", "--noprofile", "-ic", f"source {PROMPT_SH}; {snippet}"],
        env={**os.environ, "PS1": POISONED_PS1, **env},
        capture_output=True,
        text=True,
    )
    return proc.stdout


class TestPs1IsNeverExported:
    def test_inherited_export_is_dropped(self):
        """The export attribute survives reassignment – it must be cleared."""
        assert "declare -x PS1" not in _run("declare -p PS1")

    def test_children_see_no_ps1(self):
        assert "CLEAN" in _run('env | grep -a "^PS1" || echo CLEAN')

    def test_cleared_even_when_opted_out(self):
        """ALIASES_NO_PROMPT is set *because* of this bug – it must still unexport."""
        assert "CLEAN" in _run('env | grep -a "^PS1" || echo CLEAN', ALIASES_NO_PROMPT="1")

    def test_prompt_value_is_left_alone_when_opted_out(self):
        """export -n drops the attribute only; the user's own prompt is untouched."""
        # printf, not declare -p: the latter re-quotes $ and \ in its output.
        assert _run('printf "%s" "$PS1"', ALIASES_NO_PROMPT="1") == POISONED_PS1


class TestPromptStillWorks:
    def test_ps1_holds_a_literal_value_not_a_reference(self):
        out = _run("declare -p PS1")
        assert "_ALIASES_PROMPT_CACHE" not in out  # no variable reference in PS1
        assert "\\$ " in out  # trailing prompt sigil

    def test_refreshes_on_cd(self):
        before, after = _run('echo "$PS1"; cd /; _aliases_update_prompt; echo "$PS1"').splitlines()
        assert before != after
