"""``aliases-cli update`` – check for a newer version and self-update.

Fetches the latest release tag from GitHub, compares it to the installed
version, and re-installs via ``uv tool install git+<repo>`` if an update
is available (or when ``--force`` is given).
"""

from __future__ import annotations

import shutil
import subprocess
import urllib.request
import json
import sys
from typing import TYPE_CHECKING

import click

from aliases_cli import __version__

_REPO = "bbauer0706/aliases"
_INSTALL_URL = "git+https://github.com/bbauer0706/aliases"
_TAGS_API = f"https://api.github.com/repos/{_REPO}/tags"


def _fetch_latest_tag() -> str | None:
    """Return the latest vX.Y.Z tag from GitHub, or None on failure."""
    try:
        req = urllib.request.Request(
            _TAGS_API,
            headers={"Accept": "application/vnd.github+json", "User-Agent": "aliases-cli"},
        )
        with urllib.request.urlopen(req, timeout=10) as resp:  # noqa: S310
            tags: list[dict] = json.loads(resp.read())
        for tag in tags:
            name: str = tag.get("name", "")
            if name.startswith("v"):
                return name  # tags are newest-first
    except Exception:
        pass
    return None


def _parse_version(tag: str) -> tuple[int, ...]:
    """Convert 'v2.1.0' → (2, 1, 0).  Returns (0,) on parse failure."""
    try:
        return tuple(int(x) for x in tag.lstrip("v").split("."))
    except ValueError:
        return (0,)


@click.command("update")
@click.option("--check", "check_only", is_flag=True, help="Only check; do not install.")
@click.option("--force", is_flag=True, help="Re-install even if already up to date.")
def update_command(check_only: bool, force: bool) -> None:
    """Check for a newer version and update aliases-cli.

    \b
    Examples:
      aliases-cli update            Check and install if newer
      aliases-cli update --check    Check only, exit 1 if outdated
      aliases-cli update --force    Re-install regardless of version
    """
    current_tag = f"v{__version__}"
    click.echo(f"Installed : {current_tag}")

    click.echo("Checking  : https://github.com/bbauer0706/aliases …", err=False)
    latest_tag = _fetch_latest_tag()

    if latest_tag is None:
        click.echo("Error: could not reach GitHub to check for updates.", err=True)
        raise SystemExit(1)

    click.echo(f"Latest    : {latest_tag}")

    current_ver = _parse_version(current_tag)
    latest_ver = _parse_version(latest_tag)

    if not force and current_ver >= latest_ver:
        click.echo("Already up to date.")
        raise SystemExit(0)

    if check_only:
        click.echo(f"Update available: {current_tag} → {latest_tag}")
        raise SystemExit(1)

    # Locate uv
    uv_path = shutil.which("uv")
    if uv_path is None:
        click.echo(
            "Error: 'uv' not found on PATH. Install from https://docs.astral.sh/uv/",
            err=True,
        )
        raise SystemExit(1)

    click.echo(f"Updating  : {current_tag} → {latest_tag} …")
    result = subprocess.run(  # noqa: S603
        [uv_path, "tool", "install", "--force-reinstall", _INSTALL_URL],
        text=True,
    )

    if result.returncode != 0:
        click.echo("Error: update failed (see output above).", err=True)
        raise SystemExit(result.returncode)

    click.echo(f"Updated to {latest_tag}.")
