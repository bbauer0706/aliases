"""Subprocess and port utilities."""

from __future__ import annotations

import shutil
import socket
import subprocess
from pathlib import Path
from typing import Union


def execute(
    cmd: Union[list[str], str],
    cwd: Union[Path, str, None] = None,
    capture: bool = True,
) -> tuple[int, str, str]:
    """Run *cmd* synchronously.

    Returns ``(returncode, stdout, stderr)``.
    """
    if isinstance(cmd, str):
        cmd = cmd.split()
    result = subprocess.run(
        cmd,
        cwd=cwd,
        capture_output=capture,
        text=True,
    )
    return result.returncode, result.stdout or "", result.stderr or ""


def command_exists(cmd: str) -> bool:
    """Return True if *cmd* is on PATH."""
    return shutil.which(cmd) is not None


def is_port_available(port: int) -> bool:
    """Return True if nothing is listening on *port* on localhost."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.settimeout(0.05)
        try:
            sock.connect(("127.0.0.1", port))
            return False
        except (ConnectionRefusedError, OSError):
            return True
