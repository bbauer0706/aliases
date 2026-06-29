"""``aliases env`` – discover current project and export shell variables.

Output is a series of ``export VAR='value';`` lines intended to be eval-d
by the calling shell via the ``project_env`` bash function.
"""

from __future__ import annotations

import hashlib
import os
import sys
from pathlib import Path

import click

from aliases.config import Config
from aliases.process_utils import is_port_available
from aliases.project_mapper import ProjectMapper


# ---------------------------------------------------------------------------
# Click command
# ---------------------------------------------------------------------------


@click.command("env")
@click.option("-e", "profile", default=None, help="Environment profile (dev/staging/prod).")
@click.option("-p", "starting_port", default=None, type=int, help="Starting port to check.")
@click.option(
    "-i",
    "introspection",
    default=None,
    type=click.Choice(["true", "false"]),
    help="GraphQL introspection (true/false).",
)
@click.option("-t", "transfer_mode", default=None, help="Transfer mode (plain/compressed).")
@click.option("-n", "no_port_offset", is_flag=True, help="Use same port for WEB and GQL.")
@click.option("--show", "show_only", is_flag=True, help="Display current env vars and exit.")
def env_command(
    profile: str | None,
    starting_port: int | None,
    introspection: str | None,
    transfer_mode: str | None,
    no_port_offset: bool,
    show_only: bool,
) -> None:
    """Export project environment variables into the current shell.

    Intended to be called via: eval "$(aliases env [OPTIONS])"
    """
    cfg = Config.instance()
    mapper = ProjectMapper(cfg)

    if show_only:
        _show_current_env()
        return

    # Resolve project from CWD
    cwd = Path(os.environ.get("PWD", str(Path.cwd())))
    project = mapper.find_project_by_path(cwd)

    project_name = project.full_name if project else _basename_project(cwd, cfg)

    resolved_profile = profile or cfg.get("env.default_env", "dev")
    resolved_transfer = transfer_mode or "plain"
    resolved_introspection = True if introspection is None else introspection == "true"
    gql_max_retries = 3

    # Port calculation
    base = starting_port if starting_port is not None else cfg.get("env.base_port", 3000)
    offset = _port_offset(project_name)
    port_start = base + offset

    is_server_dir = _is_server_dir(cwd, project) if project else False

    if no_port_offset:
        web_port = _find_available(port_start, is_server=False)
        gql_port = web_port
    else:
        web_port = _find_available(port_start, is_server=is_server_dir)
        gql_port = web_port + 1

    # Output shell exports (eval-able)
    exports = [
        ("PROJECT_NAME", project_name),
        ("PROFILE", resolved_profile),
        ("WEBPORT", web_port),
        ("GQLPORT", gql_port),
        ("GQLNUMBEROFMAXRETRIES", gql_max_retries),
        ("GQLINTROSPECTION", "true" if resolved_introspection else "false"),
        ("GQLTRANSFERMODE", resolved_transfer),
    ]
    for name, value in exports:
        click.echo(f"export {name}='{value}';")

    # Status message to stderr so it doesn't pollute the eval
    use_color = cfg.get("general.terminal_colors", True)
    if use_color:
        click.echo(
            f"\033[32m[SUCCESS]\033[0m Project env loaded: "
            f"{project_name}, PORT: {web_port}, MODE: {resolved_transfer}",
            err=True,
        )
    else:
        click.echo(
            f"[SUCCESS] Project env loaded: {project_name}, PORT: {web_port}, MODE: {resolved_transfer}",
            err=True,
        )


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _show_current_env() -> None:
    vars_ = [
        "PROJECT_NAME",
        "PROFILE",
        "WEBPORT",
        "GQLPORT",
        "GQLNUMBEROFMAXRETRIES",
        "GQLINTROSPECTION",
        "GQLTRANSFERMODE",
    ]
    click.echo("Current Project Environment Variables:")
    click.echo("─" * 38)
    for var in vars_:
        value = os.environ.get(var, "(not set)")
        click.echo(f"{var}: {value}")
    click.echo("─" * 38)


def _basename_project(cwd: Path, cfg: Config) -> str:
    """Fall back to workspace-relative directory name."""
    workspace_dirs: list[str] = cfg.get("projects.workspace_directories", [])
    for ws_str in workspace_dirs:
        ws = Path(ws_str).expanduser().resolve()
        try:
            rel = cwd.resolve().relative_to(ws)
            return rel.parts[0] if rel.parts else cwd.name
        except ValueError:
            continue
    return cwd.name


def _port_offset(project_name: str) -> int:
    """Stable, deterministic port offset derived from the project name.

    Returns a value in the range [100, 990] in steps of 10.
    Mirrors the C++ hash-based logic but uses MD5 for cross-platform stability.
    """
    digest = hashlib.md5(project_name.encode(), usedforsecurity=False).digest()
    hash_int = int.from_bytes(digest[:4], "little")
    return 100 + (hash_int % 90) * 10


def _find_available(start: int, *, is_server: bool) -> int:
    """Find the first available port starting from *start*.

    When *is_server* is True and ``start`` is taken but ``start+1`` is free,
    we assume the web service is on ``start`` and gql on ``start+1``, so
    return ``start`` (the caller adds +1 for gql).
    """
    port = start
    while not is_port_available(port):
        if is_server and port == start and is_port_available(port + 1):
            return start
        port += 1
    return port


def _is_server_dir(cwd: Path, project: object) -> bool:
    from aliases.project_mapper import ProjectInfo  # noqa: PLC0415

    if not isinstance(project, ProjectInfo) or not project.server_path:
        return False
    try:
        cwd.resolve().relative_to(project.server_path.resolve())
        return True
    except ValueError:
        return False
