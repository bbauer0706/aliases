"""aliases entry point."""

from __future__ import annotations

import os
import sys

import click

from aliases import __version__
from aliases.commands.code_navigator import run as _code_run
from aliases.commands.config_cmd import config_group
from aliases.commands.project_env import env_command
from aliases.commands.secrets_cmd import secrets_group
from aliases.commands.setup_cmd import setup_command
from aliases.commands.update_cmd import update_command
from aliases.config import Config
from aliases.project_mapper import ProjectMapper
from aliases import pwd_formatter


# ---------------------------------------------------------------------------
# Root CLI group
# ---------------------------------------------------------------------------


@click.group(invoke_without_command=True, context_settings={"help_option_names": ["-h", "--help"]})
@click.version_option(__version__, "-v", "--version", prog_name="aliases")
@click.pass_context
def cli(ctx: click.Context) -> None:
    """aliases – developer workspace management.

    Quick-open projects in VS Code, set up project environment variables,
    manage encrypted secrets, and format your shell prompt.

    \b
    After installation, run once:
        aliases setup
    """
    if ctx.invoked_subcommand is None:
        click.echo(ctx.get_help())


# ---------------------------------------------------------------------------
# code / c
# ---------------------------------------------------------------------------


@cli.command("code", context_settings={"ignore_unknown_options": True, "allow_extra_args": True})
@click.argument("args", nargs=-1, type=click.UNPROCESSED)
def code_command(args: tuple[str, ...]) -> None:
    """Open a project (or path) in VS Code.

    \b
    Examples:
      c dispatch          Open the dispatch project
      c dispatch s        Open its server component
      c dispatch w        Open its web component
      c dispatch[sw]      Open both components
      c dispatch s w      Open server then web (same as [sw])
      c dispatch sw       Shorthand for server + web
      c proj1 proj2       Open multiple projects
      c ..                Fall back to: code ..
    """
    cfg = Config.instance()
    mapper = ProjectMapper(cfg)
    sys.exit(_code_run(args, cfg, mapper))


# Register 'c' as a short alias
cli.add_command(code_command, name="c")


# ---------------------------------------------------------------------------
# env
# ---------------------------------------------------------------------------

cli.add_command(env_command)

# ---------------------------------------------------------------------------
# config
# ---------------------------------------------------------------------------

cli.add_command(config_group)

# ---------------------------------------------------------------------------
# secrets
# ---------------------------------------------------------------------------

cli.add_command(secrets_group)

# ---------------------------------------------------------------------------
# pwd
# ---------------------------------------------------------------------------


@cli.command("pwd")
@click.option("--no-color", is_flag=True, help="Disable ANSI colour codes.")
@click.option("--ps1", is_flag=True, help="Wrap codes in \\001..\\002 for readline safety.")
@click.option("--user-host-color", "user_host_color", is_flag=True, help="Return user@host colour code only.")
@click.option("--user-host", "user_host", is_flag=True, help="Return formatted user@host string (with label replacements applied).")
@click.option("--full-prompt", "full_prompt", is_flag=True, help="Return full user@host:path string in one call (faster PS1 integration).")
def pwd_command(no_color: bool, ps1: bool, user_host_color: bool, user_host: bool, full_prompt: bool) -> None:
    """Print a formatted working directory path for use in PS1."""
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
# completion (helpers for bash tab-completion)
# ---------------------------------------------------------------------------


@cli.group("completion", hidden=True)
def completion_group() -> None:
    """Tab-completion data helpers (used by aliases-completion.sh)."""


@completion_group.command("projects")
def completion_projects() -> None:
    """Print all projects in pipe-delimited format for tab-completion."""
    cfg = Config.instance()
    mapper = ProjectMapper(cfg)
    for p in mapper.discover_projects():
        has_s = "1" if p.has_server_component else "0"
        has_w = "1" if p.has_web_component else "0"
        click.echo(f"{p.display_name}|{p.full_name}|{has_s}|{has_w}")


@completion_group.command("components")
@click.argument("project_name")
def completion_components(project_name: str) -> None:
    """Print available component suffixes for PROJECT_NAME."""
    cfg = Config.instance()
    mapper = ProjectMapper(cfg)
    project = mapper.find_project(project_name)
    if project:
        if project.has_server_component:
            click.echo("s")
            click.echo("server")
        if project.has_web_component:
            click.echo("w")
            click.echo("web")


@completion_group.command("config-keys")
def completion_config_keys() -> None:
    """Print all valid config keys for tab-completion."""
    cfg = Config.instance()
    data = cfg.all()
    for section, values in sorted(data.items()):
        if isinstance(values, dict):
            for key in sorted(values.keys()):
                click.echo(f"{section}.{key}")


# ---------------------------------------------------------------------------
# setup
# ---------------------------------------------------------------------------

cli.add_command(setup_command)


# ---------------------------------------------------------------------------
# update
# ---------------------------------------------------------------------------

cli.add_command(update_command)


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------


def main() -> None:
    # Trigger auto-sync in the background so it never blocks command execution.
    try:
        import threading  # noqa: PLC0415
        from aliases.config_sync import ConfigSync  # noqa: PLC0415

        t = threading.Thread(
            target=ConfigSync(Config.instance()).maybe_auto_sync,
            daemon=True,
        )
        t.start()
    except Exception:
        pass

    cli()
