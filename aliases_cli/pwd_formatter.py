"""PS1 / prompt path formatting.

Reads ``prompt.path_replacements`` from config and applies the first matching
rule to the current working directory.  ANSI escape codes can be wrapped in
``\\001...\\002`` (readline non-printing delimiters) for use inside PS1.
"""

from __future__ import annotations

import os
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from aliases_cli.config import Config

# ---------------------------------------------------------------------------
# Colour map (name → ANSI escape sequence)
# ---------------------------------------------------------------------------

ANSI_COLORS: dict[str, str] = {
    "black": "\033[30m",
    "red": "\033[31m",
    "green": "\033[32m",
    "yellow": "\033[33m",
    "blue": "\033[34m",
    "magenta": "\033[35m",
    "cyan": "\033[36m",
    "white": "\033[37m",
    "bold_black": "\033[1;30m",
    "bold_red": "\033[1;31m",
    "bold_green": "\033[1;32m",
    "bold_yellow": "\033[1;33m",
    "bold_blue": "\033[1;34m",
    "bold_magenta": "\033[1;35m",
    "bold_cyan": "\033[1;36m",
    "bold_white": "\033[1;37m",
    "reset": "\033[0m",
}

_RESET = "\033[0m"


def _wrap(code: str, ps1_mode: bool) -> str:
    """Optionally wrap an ANSI code in readline non-printing delimiters."""
    if not code:
        return ""
    return f"\001{code}\002" if ps1_mode else code


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------


def format_pwd(
    config: "Config",
    pwd: str | None = None,
    *,
    no_color: bool = False,
    ps1: bool = False,
) -> str:
    """Return a formatted representation of *pwd* (defaults to ``$PWD``)."""
    path = pwd or os.environ.get("PWD") or str(Path.cwd())

    use_color = config.get("general.terminal_colors", True) and not no_color
    replacements: list[dict] = config.get("prompt.path_replacements", [])
    default_color_name: str = config.get("prompt.default_path_color", "bold_blue")

    def color(name: str) -> str:
        if not use_color:
            return ""
        return _wrap(ANSI_COLORS.get(name, ""), ps1)

    def reset() -> str:
        if not use_color:
            return ""
        return _wrap(_RESET, ps1)

    # Apply replacement rules – first match wins.
    for rule in replacements:
        env_var = rule.get("env_var")
        literal_path = rule.get("path")

        if env_var:
            prefix = os.environ.get(env_var, "")
        elif literal_path:
            prefix = str(Path(literal_path).expanduser())
        else:
            continue

        if not prefix:
            continue

        if path.startswith(prefix):
            label = rule.get("label", env_var or "")
            color_name = rule.get("color", "")
            remainder = path[len(prefix):]
            return f"{color(color_name)}{label}{reset()}{remainder}"

    # Default: replace $HOME with ~
    home = str(Path.home())
    if path.startswith(home):
        path = "~" + path[len(home):]

    return f"{color(default_color_name)}{path}{reset()}"


def get_user_host_color(config: "Config", *, ps1: bool = False) -> str:
    """Return the ANSI colour code for the user@host portion of the prompt."""
    use_color = config.get("general.terminal_colors", True)
    if not use_color:
        return ""
    color_name: str = config.get("prompt.user_host_color", "bold_green")
    code = ANSI_COLORS.get(color_name, "")
    return _wrap(code, ps1) if code else ""
