"""``aliases-cli secrets`` – OS-keychain-backed secrets management.

Uses the ``keyring`` library as the secure backend (GNOME Keyring, macOS
Keychain, Windows Credential Manager, etc.).  A plaintext index file at
``~/.config/aliases-cli/secrets_names.json`` tracks the *names* of stored
secrets so that ``secrets list`` does not require decrypting anything.

For headless / CI environments without a system keychain install the extra:
    uv tool install git+<url> --extra keyring-fallback
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

import click

from aliases_cli.config import Config

SERVICE_NAME = "aliases-cli"
_NAME_RE = re.compile(r"^[A-Za-z0-9_-]+$")


# ---------------------------------------------------------------------------
# Click command group
# ---------------------------------------------------------------------------


@click.group("secrets")
def secrets_group() -> None:
    """Manage encrypted secrets (backed by the OS keychain)."""


# ── set ────────────────────────────────────────────────────────────────────


@secrets_group.command("set")
@click.argument("name")
@click.argument("value", required=False)
def secrets_set(name: str, value: str | None) -> None:
    """Store a secret in the OS keychain.

    If VALUE is omitted you will be prompted securely (no echo).
    """
    if not _NAME_RE.match(name):
        click.echo(
            f"Invalid name '{name}'. Use only [A-Za-z0-9_-].", err=True
        )
        sys.exit(1)

    if value is None:
        value = click.prompt(f"Value for '{name}'", hide_input=True)

    _keyring_set(name, value)
    _index_add(name)
    click.echo(f"\033[32m✓\033[0m Secret '{name}' stored.")


# ── get ────────────────────────────────────────────────────────────────────


@secrets_group.command("get")
@click.argument("name")
def secrets_get(name: str) -> None:
    """Print the value of a secret."""
    value = _keyring_get(name)
    if value is None:
        click.echo(f"Secret '{name}' not found.", err=True)
        sys.exit(1)
    click.echo(value)


# ── list ───────────────────────────────────────────────────────────────────


@secrets_group.command("list")
def secrets_list() -> None:
    """List all stored secret names."""
    names = _index_load()
    if not names:
        click.echo("(no secrets stored)")
        return
    for name in sorted(names):
        click.echo(name)


# ── delete ─────────────────────────────────────────────────────────────────


@secrets_group.command("delete")
@click.argument("name")
@click.option("--yes", "-y", is_flag=True, help="Skip confirmation.")
def secrets_delete(name: str, yes: bool) -> None:
    """Remove a secret from the keychain."""
    if not yes:
        click.confirm(f"Delete secret '{name}'?", default=False, abort=True)
    try:
        _keyring_delete(name)
    except Exception:
        pass  # May not exist; index clean-up handles it
    _index_remove(name)
    click.echo(f"\033[32m✓\033[0m Secret '{name}' deleted.")


# Add remove / rm as aliases
secrets_group.add_command(secrets_delete, name="remove")
secrets_group.add_command(secrets_delete, name="rm")


# ── load ───────────────────────────────────────────────────────────────────


@secrets_group.command("load")
@click.argument("names", nargs=-1)
def secrets_load(names: tuple[str, ...]) -> None:
    """Output shell export statements for use with eval.

    Without NAME arguments, exports all stored secrets.
    """
    targets = list(names) if names else sorted(_index_load())
    missing: list[str] = []

    for name in targets:
        value = _keyring_get(name)
        if value is None:
            missing.append(name)
            continue
        # Single-quote the value and escape any embedded single quotes.
        safe_value = value.replace("'", "'\\''")
        click.echo(f"export {name}='{safe_value}';")

    if missing:
        click.echo(
            f"Warning: secrets not found: {', '.join(missing)}", err=True
        )


# ── rotate-master ──────────────────────────────────────────────────────────


@secrets_group.command("rotate-master")
def secrets_rotate_master() -> None:
    """Not applicable – the OS keychain manages credentials automatically."""
    click.echo(
        "Secrets are stored in the OS keychain.\n"
        "Key rotation is handled by the keychain itself.\n"
        "To change access, use your OS keychain manager."
    )


# ---------------------------------------------------------------------------
# Keyring helpers
# ---------------------------------------------------------------------------


def _keyring_set(name: str, value: str) -> None:
    import keyring  # noqa: PLC0415

    try:
        keyring.set_password(SERVICE_NAME, name, value)
    except Exception as exc:
        click.echo(f"Keychain error: {exc}", err=True)
        sys.exit(1)


def _keyring_get(name: str) -> str | None:
    import keyring  # noqa: PLC0415

    try:
        return keyring.get_password(SERVICE_NAME, name)
    except Exception as exc:
        click.echo(f"Keychain error: {exc}", err=True)
        sys.exit(1)


def _keyring_delete(name: str) -> None:
    import keyring  # noqa: PLC0415

    keyring.delete_password(SERVICE_NAME, name)


# ---------------------------------------------------------------------------
# Name index (plaintext list of secret names for `secrets list`)
# ---------------------------------------------------------------------------


def _index_path() -> Path:
    return Config.instance().config_dir / "secrets_names.json"


def _index_load() -> list[str]:
    p = _index_path()
    if not p.exists():
        return []
    try:
        data = json.loads(p.read_text(encoding="utf-8"))
        return data if isinstance(data, list) else []
    except (json.JSONDecodeError, OSError):
        return []


def _index_save(names: list[str]) -> None:
    p = _index_path()
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(sorted(set(names)), indent=2) + "\n", encoding="utf-8")


def _index_add(name: str) -> None:
    names = _index_load()
    if name not in names:
        names.append(name)
    _index_save(names)


def _index_remove(name: str) -> None:
    names = [n for n in _index_load() if n != name]
    _index_save(names)
