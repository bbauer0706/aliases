"""``aliases pwd`` – PS1 path formatting and prompt colour/label configuration."""

from __future__ import annotations

import sys

import click

from aliases.config import Config
from aliases import pwd_formatter


# ---------------------------------------------------------------------------
# pwd group  (invoke_without_command keeps existing flag-only usage working)
# ---------------------------------------------------------------------------


@click.group(
    "pwd",
    invoke_without_command=True,
    context_settings={"help_option_names": ["-h", "--help"]},
)
@click.option("--no-color", is_flag=True, help="Disable ANSI colour codes.")
@click.option("--ps1", is_flag=True, help="Wrap codes in \\001..\\002 for readline safety.")
@click.option("--user-host-color", "user_host_color", is_flag=True, help="Return user@host colour code only.")
@click.option("--user-host", "user_host", is_flag=True, help="Return formatted user@host string (with label replacements applied).")
@click.option("--full-prompt", "full_prompt", is_flag=True, help="Return full user@host:path string in one call (faster PS1 integration).")
@click.pass_context
def pwd_group(
    ctx: click.Context,
    no_color: bool,
    ps1: bool,
    user_host_color: bool,
    user_host: bool,
    full_prompt: bool,
) -> None:
    """Print formatted working directory path for PS1, or configure prompt settings."""
    if ctx.invoked_subcommand is not None:
        return
    cfg = Config.instance()
    if full_prompt:
        click.echo(pwd_formatter.format_full_prompt(cfg, no_color=no_color, ps1=ps1), nl=False, color=True)
    elif user_host:
        click.echo(pwd_formatter.get_user_host_label(cfg, no_color=no_color, ps1=ps1), nl=False, color=True)
    elif user_host_color:
        click.echo(pwd_formatter.get_user_host_color(cfg, ps1=ps1), nl=False, color=True)
    else:
        click.echo(pwd_formatter.format_pwd(cfg, no_color=no_color, ps1=ps1), nl=False, color=True)


# ---------------------------------------------------------------------------
# pwd colors
# ---------------------------------------------------------------------------

_COLOR_GROUPS: list[tuple[str, list[str]]] = [
    ("standard", ["black", "red", "green", "yellow", "blue", "magenta", "cyan", "white"]),
    ("bold", ["bold_black", "bold_red", "bold_green", "bold_yellow", "bold_blue", "bold_magenta", "bold_cyan", "bold_white"]),
    ("bright", ["bright_black", "bright_red", "bright_green", "bright_yellow", "bright_blue", "bright_magenta", "bright_cyan", "bright_white"]),
    ("bold_bright", ["bold_bright_black", "bold_bright_red", "bold_bright_green", "bold_bright_yellow", "bold_bright_blue", "bold_bright_magenta", "bold_bright_cyan", "bold_bright_white"]),
    ("dim", ["dim_black", "dim_red", "dim_green", "dim_yellow", "dim_blue", "dim_magenta", "dim_cyan", "dim_white"]),
]


@pwd_group.command("colors")
def pwd_colors() -> None:
    """List all available color names with a live swatch."""
    shown: set[str] = set()
    for group_name, names in _COLOR_GROUPS:
        sys.stdout.write(f"\n  {group_name}\n")
        for name in names:
            code = pwd_formatter.ANSI_COLORS.get(name, "")
            sys.stdout.write(f"    {code}████\033[0m  {name}\n")
            shown.add(name)
    # Any remaining entries not covered by the static groups
    extras = [n for n in pwd_formatter.ANSI_COLORS if n not in shown]
    if extras:
        sys.stdout.write("\n  other\n")
        for name in extras:
            code = pwd_formatter.ANSI_COLORS[name]
            sys.stdout.write(f"    {code}████\033[0m  {name}\n")
    sys.stdout.write("\n")


# ---------------------------------------------------------------------------
# pwd set group
# ---------------------------------------------------------------------------


@pwd_group.group("set")
def pwd_set_group() -> None:
    """Update prompt configuration settings."""


@pwd_set_group.command("default-color")
@click.argument("color")
def pwd_set_default_color(color: str) -> None:
    """Set the default path color (prompt.default_path_color)."""
    _require_color(color)
    cfg = Config.instance()
    cfg._data["prompt"]["default_path_color"] = color
    cfg.save()
    _ok(f"default_path_color = {color}")


@pwd_set_group.command("user-host-color")
@click.argument("color")
def pwd_set_user_host_color(color: str) -> None:
    """Set the user@host label color (prompt.user_host_color)."""
    _require_color(color)
    cfg = Config.instance()
    cfg._data["prompt"]["user_host_color"] = color
    cfg.save()
    _ok(f"user_host_color = {color}")


@pwd_set_group.command("host-label")
@click.argument("pattern")
@click.argument("label")
def pwd_set_host_label(pattern: str, label: str) -> None:
    """Add or update a hostname label replacement rule.

    \b
    PATTERN  fnmatch glob or ${VAR} expression matched against the hostname.
    LABEL    replacement string shown in the prompt (${VAR} also expanded).

    \b
    Examples:
      aliases pwd set host-label 'ip-10-*'    prod
      aliases pwd set host-label '*'           local
      aliases pwd set host-label '${HOSTNAME}' mybox
    """
    cfg = Config.instance()
    rules: list[dict] = cfg._data["prompt"].setdefault("host_replacements", [])
    _upsert(rules, "hostname", pattern, label=label)
    cfg.save()
    _ok(f"host {pattern!r} → {label!r}")


@pwd_set_group.command("user-label")
@click.argument("pattern")
@click.argument("label")
def pwd_set_user_label(pattern: str, label: str) -> None:
    """Add or update a username label replacement rule.

    \b
    PATTERN  fnmatch glob or ${VAR} expression matched against the username.
    LABEL    replacement string shown in the prompt.
    """
    cfg = Config.instance()
    rules: list[dict] = cfg._data["prompt"].setdefault("user_replacements", [])
    _upsert(rules, "username", pattern, label=label)
    cfg.save()
    _ok(f"user {pattern!r} → {label!r}")


@pwd_set_group.command("path-rule")
@click.argument("path")
@click.argument("label")
@click.option("--color", "color_name", default="", help="Color name for the label (see 'aliases pwd colors').")
def pwd_set_path_rule(path: str, label: str, color_name: str) -> None:
    """Add or update a path prefix replacement rule.

    \b
    PATH   prefix to match; $VAR references are expanded at runtime.
    LABEL  replacement label shown in the prompt.

    \b
    Examples:
      aliases pwd set path-rule '$GOPATH'  go   --color bright_cyan
      aliases pwd set path-rule '~/work'   work --color bold_yellow
    """
    if color_name:
        _require_color(color_name)
    cfg = Config.instance()
    rules: list[dict] = cfg._data["prompt"].setdefault("path_replacements", [])
    _upsert(rules, "path", path, label=label, color=color_name)
    cfg.save()
    suffix = f" [{color_name}]" if color_name else ""
    _ok(f"path {path!r} → {label!r}{suffix}")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _require_color(name: str) -> None:
    if name not in pwd_formatter.ANSI_COLORS:
        click.echo(
            f"Unknown color '{name}'. Run 'aliases pwd colors' to see all options.",
            err=True,
        )
        sys.exit(1)


def _upsert(rules: list[dict], key: str, pattern: str, **fields: str) -> None:
    for rule in rules:
        if rule.get(key) == pattern:
            rule.update(fields)
            return
    rules.append({key: pattern, **fields})


def _ok(msg: str) -> None:
    from rich.console import Console  # noqa: PLC0415

    Console().print(f"[green]✓[/green] {msg}")
