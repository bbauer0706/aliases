"""``aliases-cli config`` – read, write, and sync configuration."""

from __future__ import annotations

import json
import subprocess
import sys

import click

from aliases_cli.config import Config
from aliases_cli.config_sync import ConfigSync


# ---------------------------------------------------------------------------
# Click command group
# ---------------------------------------------------------------------------


@click.group("config")
def config_group() -> None:
    """Manage aliases-cli configuration."""


# ── get ────────────────────────────────────────────────────────────────────


@config_group.command("get")
@click.argument("key")
def config_get(key: str) -> None:
    """Print the value of a configuration key (dot-notation)."""
    cfg = Config.instance()
    value = cfg.get(key)
    if value is None:
        click.echo(f"Key '{key}' not found.", err=True)
        sys.exit(1)
    if isinstance(value, (dict, list)):
        click.echo(json.dumps(value, indent=2))
    else:
        click.echo(str(value))


# ── set ────────────────────────────────────────────────────────────────────


@config_group.command("set")
@click.argument("key")
@click.argument("value")
def config_set(key: str, value: str) -> None:
    """Set a configuration value (dot-notation key)."""
    cfg = Config.instance()
    if cfg.get(key) is None:
        click.echo(f"Unknown key '{key}'.", err=True)
        sys.exit(1)
    cfg.set(key, value)
    cfg.save()
    coerced = cfg.get(key)
    from rich.console import Console  # noqa: PLC0415
    Console().print(f"[green]✓[/green] {key} = {coerced}")


# ── list ───────────────────────────────────────────────────────────────────


@config_group.command("list")
@click.option("--plain", is_flag=True, help="Plain text output (key = value).")
def config_list(plain: bool) -> None:
    """List all configuration settings."""
    cfg = Config.instance()
    data = cfg.all()

    if plain:
        for section, values in sorted(data.items()):
            if isinstance(values, dict):
                for k, v in sorted(values.items()):
                    click.echo(f"{section}.{k} = {v}")
        return

    from rich.console import Console  # noqa: PLC0415
    from rich.table import Table  # noqa: PLC0415
    console = Console()
    for section, values in sorted(data.items()):
        table = Table(
            title=f"[bold cyan][{section}][/bold cyan]",
            show_header=False,
            box=None,
            padding=(0, 2),
        )
        table.add_column("key", style="dim")
        table.add_column("value")
        if isinstance(values, dict):
            for k, v in sorted(values.items()):
                table.add_row(f"{section}.{k}", _fmt_value(v))
        console.print(table)


# ── reset ──────────────────────────────────────────────────────────────────


@config_group.command("reset")
@click.option("--yes", "-y", is_flag=True, help="Skip confirmation prompt.")
def config_reset(yes: bool) -> None:
    """Reset all configuration to defaults."""
    cfg = Config.instance()
    confirm = yes or click.confirm(
        "Reset all configuration to defaults?", default=False
    )
    if not confirm:
        click.echo("Aborted.")
        return
    # Re-initialise by wiping the saved data
    cfg._data = json.loads(json.dumps(cfg.all()))  # snapshot
    from aliases_cli.config import DEFAULT_CONFIG  # noqa: PLC0415

    import copy

    cfg._data = copy.deepcopy(DEFAULT_CONFIG)
    cfg.save()
    console.print("[green]✓[/green] Configuration reset to defaults.")


# ── edit ───────────────────────────────────────────────────────────────────


@config_group.command("edit")
def config_edit() -> None:
    """Open the config file in the configured editor."""
    cfg = Config.instance()
    editor = cfg.get("general.editor", "code")
    subprocess.run([editor, str(cfg.config_file)])  # noqa: S603


# ── path ───────────────────────────────────────────────────────────────────


@config_group.command("path")
def config_path() -> None:
    """Show the path to the config file."""
    click.echo(str(Config.instance().config_file))


# ── sync subgroup ──────────────────────────────────────────────────────────


@config_group.group("sync")
def sync_group() -> None:
    """Sync configuration with a remote source."""


@sync_group.command("setup")
@click.argument("url")
@click.argument("method", default="git")
def sync_setup(url: str, method: str) -> None:
    """Configure remote sync.

    METHOD is one of: git (default), rsync, file, http.
    """
    sync = ConfigSync(Config.instance())
    if not sync.setup(url, method):
        sys.exit(1)


@sync_group.command("pull")
def sync_pull() -> None:
    """Pull config from remote."""
    sync = ConfigSync(Config.instance())
    if not sync.pull():
        sys.exit(1)


@sync_group.command("push")
def sync_push() -> None:
    """Push config to remote."""
    sync = ConfigSync(Config.instance())
    if not sync.push():
        sys.exit(1)


@sync_group.command("status")
def sync_status() -> None:
    """Show sync configuration and last-sync time."""
    sync = ConfigSync(Config.instance())
    sync.status()


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _fmt_value(v: object) -> str:
    if isinstance(v, bool):
        return "[green]true[/green]" if v else "[red]false[/red]"
    if isinstance(v, (list, dict)):
        return json.dumps(v)
    return str(v)
