"""Project discovery and component resolution.

Scans workspace directories configured in ``projects.workspace_directories``
and builds a list of :class:`ProjectInfo` objects.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from aliases_cli.config import Config


@dataclass
class ProjectInfo:
    full_name: str
    display_name: str
    path: Path
    server_path: Path | None = None
    web_path: Path | None = None

    @property
    def has_server_component(self) -> bool:
        return self.server_path is not None

    @property
    def has_web_component(self) -> bool:
        return self.web_path is not None


class ProjectMapper:
    def __init__(self, config: "Config") -> None:
        self._config = config

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def discover_projects(self) -> list[ProjectInfo]:
        """Scan workspace directories and return all discovered projects."""
        workspace_dirs: list[str] = self._config.get("projects.workspace_directories", [])
        shortcuts: dict[str, str] = self._config.get("projects.shortcuts", {})
        ignore: list[str] = self._config.get("projects.ignore", [])

        # shortcuts maps display_name → full_name; invert for lookup
        full_to_display: dict[str, str] = {v: k for k, v in shortcuts.items()}

        projects: list[ProjectInfo] = []
        seen: set[Path] = set()

        for ws_str in workspace_dirs:
            ws = Path(ws_str).expanduser().resolve()
            if not ws.is_dir():
                continue
            for entry in sorted(ws.iterdir()):
                if not entry.is_dir():
                    continue
                if entry.name.startswith(".") or entry.name in ignore:
                    continue
                resolved = entry.resolve()
                if resolved in seen:
                    continue
                seen.add(resolved)

                display_name = full_to_display.get(entry.name, entry.name)
                projects.append(
                    ProjectInfo(
                        full_name=entry.name,
                        display_name=display_name,
                        path=entry,
                        server_path=self._find_server_path(entry),
                        web_path=self._find_web_path(entry),
                    )
                )

        return projects

    def find_project(self, name: str) -> ProjectInfo | None:
        """Find a project by display name or full name (case-insensitive fallback)."""
        projects = self.discover_projects()
        # Exact match first
        for p in projects:
            if p.display_name == name or p.full_name == name:
                return p
        # Case-insensitive fallback
        name_lower = name.lower()
        for p in projects:
            if p.display_name.lower() == name_lower or p.full_name.lower() == name_lower:
                return p
        return None

    def find_project_by_path(self, path: Path) -> ProjectInfo | None:
        """Return the project that *path* lives inside, or None."""
        resolved = path.resolve()
        for project in self.discover_projects():
            proj_resolved = project.path.resolve()
            if resolved == proj_resolved or str(resolved).startswith(str(proj_resolved) + "/"):
                return project
        return None

    # ------------------------------------------------------------------
    # Component discovery
    # ------------------------------------------------------------------

    def _find_server_path(self, project_dir: Path) -> Path | None:
        custom: dict[str, str] = self._config.get("projects.server_paths", {})
        if project_dir.name in custom:
            candidate = project_dir / custom[project_dir.name]
            return candidate if candidate.is_dir() else None

        defaults: list[str] = self._config.get(
            "projects.default_paths.server",
            ["java/serverJava", "serverJava", "backend", "server"],
        )
        for rel in defaults:
            candidate = project_dir / rel
            if candidate.is_dir():
                return candidate
        return None

    def _find_web_path(self, project_dir: Path) -> Path | None:
        custom: dict[str, str] = self._config.get("projects.web_paths", {})
        if project_dir.name in custom:
            candidate = project_dir / custom[project_dir.name]
            return candidate if candidate.is_dir() else None

        defaults: list[str] = self._config.get(
            "projects.default_paths.web",
            ["webapp", "webApp", "web", "frontend", "client"],
        )
        for rel in defaults:
            candidate = project_dir / rel
            if candidate.is_dir():
                return candidate
        return None
