"""Thin wrappers around git CLI commands."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Union

from aliases.process_utils import execute

MAIN_BRANCH_NAMES = frozenset({"main", "master", "trunk", "develop"})


@dataclass
class GitStatus:
    is_git_repo: bool
    current_branch: str = ""
    has_uncommitted_changes: bool = False
    is_main_branch: bool = False


def get_status(path: Union[Path, str]) -> GitStatus:
    """Return the git status for the repo at *path*."""
    code, _, _ = execute(["git", "rev-parse", "--git-dir"], cwd=path)
    if code != 0:
        return GitStatus(is_git_repo=False)

    _, branch_out, _ = execute(["git", "rev-parse", "--abbrev-ref", "HEAD"], cwd=path)
    branch = branch_out.strip()

    _, status_out, _ = execute(["git", "status", "--porcelain"], cwd=path)

    return GitStatus(
        is_git_repo=True,
        current_branch=branch,
        has_uncommitted_changes=bool(status_out.strip()),
        is_main_branch=branch.lower() in MAIN_BRANCH_NAMES,
    )


def get_current_branch(path: Union[Path, str]) -> str | None:
    code, out, _ = execute(["git", "rev-parse", "--abbrev-ref", "HEAD"], cwd=path)
    return out.strip() if code == 0 else None


def is_git_repo(path: Union[Path, str]) -> bool:
    code, _, _ = execute(["git", "rev-parse", "--git-dir"], cwd=path)
    return code == 0


def pull_changes(path: Union[Path, str]) -> tuple[bool, str]:
    """Run ``git pull`` in *path*. Returns ``(success, output)``."""
    code, out, err = execute(["git", "pull"], cwd=path)
    return code == 0, (out + err).strip()


def is_main_branch(branch: str) -> bool:
    return branch.lower() in MAIN_BRANCH_NAMES
