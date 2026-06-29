"""PS1 / prompt path formatting.

Reads ``prompt.path_replacements``, ``prompt.host_replacements``, and
``prompt.user_replacements`` from config and applies the first matching rule.
ANSI escape codes can be wrapped in ``\\001...\\002`` (readline non-printing
delimiters) for use inside PS1.
"""

from __future__ import annotations

import fnmatch
import os
import re
import socket
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from aliases.config import Config

# ---------------------------------------------------------------------------
# Colour map (name → ANSI escape sequence)
# ---------------------------------------------------------------------------

ANSI_COLORS: dict[str, str] = {
    # Standard foreground
    "black": "\033[30m",
    "red": "\033[31m",
    "green": "\033[32m",
    "yellow": "\033[33m",
    "blue": "\033[34m",
    "magenta": "\033[35m",
    "cyan": "\033[36m",
    "white": "\033[37m",
    # Bold foreground
    "bold_black": "\033[1;30m",
    "bold_red": "\033[1;31m",
    "bold_green": "\033[1;32m",
    "bold_yellow": "\033[1;33m",
    "bold_blue": "\033[1;34m",
    "bold_magenta": "\033[1;35m",
    "bold_cyan": "\033[1;36m",
    "bold_white": "\033[1;37m",
    # Bright (high-intensity) foreground
    "bright_black": "\033[90m",
    "bright_red": "\033[91m",
    "bright_green": "\033[92m",
    "bright_yellow": "\033[93m",
    "bright_blue": "\033[94m",
    "bright_magenta": "\033[95m",
    "bright_cyan": "\033[96m",
    "bright_white": "\033[97m",
    # Bold + bright
    "bold_bright_black": "\033[1;90m",
    "bold_bright_red": "\033[1;91m",
    "bold_bright_green": "\033[1;92m",
    "bold_bright_yellow": "\033[1;93m",
    "bold_bright_blue": "\033[1;94m",
    "bold_bright_magenta": "\033[1;95m",
    "bold_bright_cyan": "\033[1;96m",
    "bold_bright_white": "\033[1;97m",
    # Dim (faint) foreground
    "dim_black": "\033[2;30m",
    "dim_red": "\033[2;31m",
    "dim_green": "\033[2;32m",
    "dim_yellow": "\033[2;33m",
    "dim_blue": "\033[2;34m",
    "dim_magenta": "\033[2;35m",
    "dim_cyan": "\033[2;36m",
    "dim_white": "\033[2;37m",
    "reset": "\033[0m",
}

_RESET = "\033[0m"

# Resolved once per process — hostname never changes mid-session.
try:
    _HOSTNAME: str = socket.gethostname()
except OSError:
    _HOSTNAME = ""


def _wrap(code: str, ps1_mode: bool) -> str:
    """Optionally wrap an ANSI code in readline non-printing delimiters."""
    if not code:
        return ""
    return f"\001{code}\002" if ps1_mode else code


def _expand_vars(value: str) -> str:
    """Expand ``${VAR}`` and ``$VAR`` references using the current environment."""
    result = re.sub(r"\$\{([^}]+)\}", lambda m: os.environ.get(m.group(1), m.group(0)), value)
    result = re.sub(r"\$([A-Za-z_][A-Za-z0-9_]*)", lambda m: os.environ.get(m.group(1), m.group(0)), result)
    return result


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
        raw_path = rule.get("path")

        if env_var:
            # Legacy: {"env_var": "MYVAR", ...}  still supported.
            prefix = os.environ.get(env_var, "")
        elif raw_path is not None:
            # Preferred: {"path": "$MYVAR/subdir", ...} — $VAR expanded then ~.
            prefix = str(Path(_expand_vars(raw_path)).expanduser())
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


def get_user_host_label(config: "Config", *, no_color: bool = False, ps1: bool = False) -> str:
    """Return the formatted ``user@host`` string with optional label replacements.

    Checks ``prompt.user_replacements`` and ``prompt.host_replacements`` in config.
    Each rule is a dict with ``username``/``hostname`` (to match) and ``label`` (to display).
    First matching rule wins.  Falls back to the real username/hostname if no rule matches.

    Example config::

        "host_replacements": [{"hostname": "ip-10-80-1-32", "label": "prod"}],
        "user_replacements": [{"username": "benedikt.bauer", "label": "bb"}]
    """
    real_user: str = os.environ.get("USER") or os.environ.get("LOGNAME") or ""
    real_host: str = _HOSTNAME

    # Resolve user label — patterns support fnmatch globs and ${VAR} expansion.
    user_label = real_user
    for rule in config.get("prompt.user_replacements", []):
        pattern = _expand_vars(rule.get("username", ""))
        if pattern and fnmatch.fnmatch(real_user, pattern):
            user_label = _expand_vars(rule.get("label", real_user))
            break

    # Resolve host label — patterns support fnmatch globs and ${VAR} expansion.
    host_label = real_host
    for rule in config.get("prompt.host_replacements", []):
        pattern = _expand_vars(rule.get("hostname", ""))
        if pattern and fnmatch.fnmatch(real_host, pattern):
            host_label = _expand_vars(rule.get("label", real_host))
            break

    use_color = config.get("general.terminal_colors", True) and not no_color
    if use_color:
        color_name: str = config.get("prompt.user_host_color", "bold_green")
        code = ANSI_COLORS.get(color_name, "")
        open_code = _wrap(code, ps1) if code else ""
        close_code = _wrap(_RESET, ps1) if code else ""
        return f"{open_code}{user_label}@{host_label}{close_code}"

    return f"{user_label}@{host_label}"


def format_full_prompt(
    config: "Config",
    pwd: str | None = None,
    *,
    no_color: bool = False,
    ps1: bool = False,
) -> str:
    """Return the complete ``user@host:path`` prompt string in a single call.

    Combines :func:`get_user_host_label` and :func:`format_pwd` so the shell
    only needs to spawn one Python process per prompt refresh.
    """
    user_host = get_user_host_label(config, no_color=no_color, ps1=ps1)
    path = format_pwd(config, pwd, no_color=no_color, ps1=ps1)
    return f"{user_host}:{path}"
