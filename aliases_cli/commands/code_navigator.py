"""``aliases-cli code`` / ``aliases-cli c`` – open projects in VS Code.

Parsing rules (evaluated in order for each positional argument):
  1. ``project[sw]``          bracket notation → open multiple components
  2. ``projects``             exact or case-insensitive project name
  3. ``projects``             + next arg is ``s``/``server`` or ``w``/``web``
  4. ``projectsw`` / ``projectws`` composite suffix shorthand
  5. Fallback: pass all args straight to ``code``
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

import click

from aliases_cli.config import Config
from aliases_cli.project_mapper import ProjectInfo, ProjectMapper


def run(args: tuple[str, ...], config: Config, mapper: ProjectMapper) -> int:
    if not args:
        _launch(str(Path.home()), config)
        return 0

    projects = mapper.discover_projects()

    # Build fast lookup by both display_name and full_name
    by_name: dict[str, ProjectInfo] = {}
    for p in projects:
        by_name[p.display_name] = p
        by_name[p.full_name] = p

    # Check if all args look like valid project specs; if so open them all.
    # Otherwise fall back to plain `code` call.
    parsed: list[tuple[ProjectInfo, str]] = []  # [(project, component: '' | 's' | 'w')]
    i = 0
    args_list = list(args)

    while i < len(args_list):
        arg = args_list[i]

        # ── bracket notation: project[sw] ───────────────────────────────
        if "[" in arg and arg.endswith("]"):
            bracket_start = arg.index("[")
            name = arg[:bracket_start]
            components = arg[bracket_start + 1 : -1]
            project = _lookup(name, by_name)
            if project:
                for ch in components:
                    if ch == "s":
                        parsed.append((project, "s"))
                    elif ch == "w":
                        parsed.append((project, "w"))
                i += 1
                continue

        # ── exact project name ─────────────────────────────────────────
        project = _lookup(arg, by_name)
        if project:
            # Peek at next arg for component modifier
            if i + 1 < len(args_list) and args_list[i + 1] in ("s", "server"):
                parsed.append((project, "s"))
                i += 2
            elif i + 1 < len(args_list) and args_list[i + 1] in ("w", "web"):
                parsed.append((project, "w"))
                i += 2
            else:
                parsed.append((project, ""))
                i += 1
            continue

        # ── suffix shorthand: ``projects`` / ``projectw`` / ``projectsw`` ─
        resolved = _parse_with_suffix(arg, by_name)
        if resolved:
            parsed.extend(resolved)
            i += 1
            continue

        # ── composite: ``dipws`` → ``dipw`` + ``dips`` ─────────────────
        composite = _parse_composite(arg, projects)
        if composite:
            parsed.extend(composite)
            i += 1
            continue

        # ── fallback: hand everything to `code` ────────────────────────
        _fallback(args_list, config)
        return 0

    if not parsed:
        _fallback(args_list, config)
        return 0

    for project, component in parsed:
        _open_component(project, component, config)

    return 0


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _lookup(name: str, by_name: dict[str, ProjectInfo]) -> ProjectInfo | None:
    if name in by_name:
        return by_name[name]
    name_lower = name.lower()
    for key, proj in by_name.items():
        if key.lower() == name_lower:
            return proj
    return None


def _parse_with_suffix(
    spec: str, by_name: dict[str, ProjectInfo]
) -> list[tuple[ProjectInfo, str]] | None:
    """Try ``spec`` as ``<name>s``, ``<name>w``, ``<name>sw``, or ``<name>ws``."""
    if not spec:
        return None
    suffixes_to_try = [("sw", ["s", "w"]), ("ws", ["w", "s"]), ("s", ["s"]), ("w", ["w"])]
    for suffix, components in suffixes_to_try:
        if spec.endswith(suffix):
            base = spec[: -len(suffix)]
            project = _lookup(base, by_name)
            if project:
                result = []
                for c in components:
                    if c == "s" and project.has_server_component:
                        result.append((project, "s"))
                    elif c == "w" and project.has_web_component:
                        result.append((project, "w"))
                    else:
                        # Component doesn't exist — don't match
                        result = []
                        break
                if result:
                    return result
    return None


def _parse_composite(
    spec: str, projects: list[ProjectInfo]
) -> list[tuple[ProjectInfo, str]] | None:
    """Try ``spec`` as ``<base><suffixes>`` where base matches a project."""
    for proj in projects:
        for base in (proj.display_name, proj.full_name):
            if len(spec) > len(base) and spec.startswith(base):
                suffixes = spec[len(base):]
                result = []
                valid = True
                for ch in suffixes:
                    if ch == "s" and proj.has_server_component:
                        result.append((proj, "s"))
                    elif ch == "w" and proj.has_web_component:
                        result.append((proj, "w"))
                    else:
                        valid = False
                        break
                if valid and len(result) >= 2:
                    return result
    return None


def _open_component(project: ProjectInfo, component: str, config: Config) -> None:
    if component == "s":
        if project.server_path:
            click.echo(f"Opening server: {project.display_name} ({project.server_path})")
            _launch(str(project.server_path), config)
        else:
            click.echo(
                f"No server component found for '{project.display_name}'",
                err=True,
            )
    elif component == "w":
        if project.web_path:
            click.echo(f"Opening web: {project.display_name} ({project.web_path})")
            _launch(str(project.web_path), config)
        else:
            click.echo(
                f"No web component found for '{project.display_name}'",
                err=True,
            )
    else:
        click.echo(f"Opening project: {project.display_name} ({project.path})")
        _launch(str(project.path), config)


def _launch(path: str, config: Config) -> None:
    flags: list[str] = config.get("code.vscode_flags", [])
    reuse = config.get("code.reuse_window", True)
    cmd = ["code"]
    if reuse:
        cmd.append("--reuse-window")
    cmd.extend(flags)
    cmd.append(path)
    subprocess.Popen(cmd)  # noqa: S603 – deliberate user-initiated launch


def _fallback(args: list[str], config: Config) -> None:
    flags: list[str] = config.get("code.vscode_flags", [])
    cmd = ["code", *flags, *args]
    subprocess.Popen(cmd)  # noqa: S603
