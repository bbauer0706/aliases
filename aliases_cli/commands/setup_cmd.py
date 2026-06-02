"""``aliases-cli setup`` – post-install shell integration wizard.

After ``uv tool install git+<url>`` run this once to wire up:
  • ~/.config/aliases-cli/  (config, shell scripts, bash aliases, completion)
  • ~/.bash_aliases          (sources all integration files)
  • ~/.bashrc                (sources ~/.bash_aliases)
"""

from __future__ import annotations

import shutil
import sys
from importlib.resources import as_file, files
from pathlib import Path

import click

from aliases_cli.config import Config, DEFAULT_CONFIG

# Data files bundled inside the Python package under aliases_cli/data/
_DATA_ROOT = files("aliases_cli") / "data"

# Files to copy: (package-relative source, destination relative to config_dir)
_SHELL_FILES = [
    ("shell/project-env.sh", "shell/project-env.sh"),
    ("shell/prompt.sh", "shell/prompt.sh"),
    ("shell/secrets.sh", "shell/secrets.sh"),
]
_ALIAS_FILES = [
    ("bash_aliases/basic.ali.sh", "bash_aliases/basic.ali.sh"),
    ("bash_aliases/clear.ali.sh", "bash_aliases/clear.ali.sh"),
    ("bash_aliases/git.ali.sh", "bash_aliases/git.ali.sh"),
    ("bash_aliases/maven.ali.sh", "bash_aliases/maven.ali.sh"),
    ("bash_aliases/npm.ali.sh", "bash_aliases/npm.ali.sh"),
    ("bash_aliases/syncrotess.ali.sh", "bash_aliases/syncrotess.ali.sh"),
]
_COMPLETION_FILES = [
    ("bash_completion/aliases-completion.sh", "bash_completion/aliases-completion.sh"),
]


@click.command("setup")
@click.option("--force", "-f", is_flag=True, help="Overwrite existing files without prompting.")
@click.option(
    "--update",
    "-u",
    is_flag=True,
    help="Update shell/alias files only; do not touch .bash_aliases or .bashrc.",
)
def setup_command(force: bool, update: bool) -> None:
    """Set up shell integration after installing aliases-cli.

    Run once after installation:

    \b
        uv tool install git+https://github.com/bbauer0706/aliases
        aliases-cli setup
    """
    config_dir = Config.instance().config_dir
    bash_aliases_path = Path.home() / ".bash_aliases"
    bashrc_path = Path.home() / ".bashrc"

    click.echo(f"Config directory: {config_dir}")

    # ── 1. Create config directory ─────────────────────────────────────────
    config_dir.mkdir(parents=True, exist_ok=True)

    # ── 2. Copy config template if no config exists ────────────────────────
    config_file = config_dir / "config.json"
    if not config_file.exists():
        _copy_data_file("config.template.json", config_file)
        click.echo(f"  Created {config_file}")
    else:
        # Config exists — save it back after _deep_merge so any new default
        # keys added since last install are written to disk.
        cfg = Config.instance()
        cfg.save()
        click.echo(f"  Config updated with new defaults: {config_file}")

    # ── 3. Copy shell integration, aliases, completion ─────────────────────
    all_files = _SHELL_FILES + _ALIAS_FILES + _COMPLETION_FILES
    for src_rel, dest_rel in all_files:
        dest = config_dir / dest_rel
        dest.parent.mkdir(parents=True, exist_ok=True)
        if dest.exists() and not force and not update:
            click.echo(f"  Skipping (exists): {dest}")
        else:
            _copy_data_file(src_rel, dest)
            dest.chmod(0o644)
            click.echo(f"  Installed: {dest}")

    if update:
        click.echo("\n\033[32m✓\033[0m Shell files updated.")
        return

    # ── 4. Create / update ~/.bash_aliases ────────────────────────────────
    bash_aliases_content = _generate_bash_aliases(config_dir)

    if bash_aliases_path.exists() and not force:
        click.echo(f"\n{bash_aliases_path} already exists.")
        if not click.confirm("  Overwrite it?", default=False):
            click.echo("  Skipped. You can run 'aliases-cli setup --force' to overwrite.")
        else:
            _backup_and_write(bash_aliases_path, bash_aliases_content)
    else:
        if bash_aliases_path.exists():
            _backup_and_write(bash_aliases_path, bash_aliases_content)
        else:
            bash_aliases_path.write_text(bash_aliases_content, encoding="utf-8")
        click.echo(f"  Written: {bash_aliases_path}")

    # ── 5. Ensure ~/.bashrc sources ~/.bash_aliases ────────────────────────
    _ensure_bashrc(bashrc_path, bash_aliases_path)

    click.echo(
        "\n\033[32m✓\033[0m Setup complete. Restart your shell or run:\n"
        "    source ~/.bash_aliases"
    )


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _copy_data_file(src_rel: str, dest: Path) -> None:
    """Copy a bundled data file to *dest*."""
    src_traversable = _DATA_ROOT
    for part in src_rel.split("/"):
        src_traversable = src_traversable / part

    with as_file(src_traversable) as src_path:
        shutil.copy2(src_path, dest)


def _generate_bash_aliases(config_dir: Path) -> str:
    return f"""\
# aliases-cli shell integration
# Generated by: aliases-cli setup
# To update run: aliases-cli setup --update

ALIASES_CONFIG_DIR="{config_dir}"

# ── Shell integration ───────────────────────────────────────────────────────
[ -f "${{ALIASES_CONFIG_DIR}}/shell/project-env.sh" ] && source "${{ALIASES_CONFIG_DIR}}/shell/project-env.sh"
[ -f "${{ALIASES_CONFIG_DIR}}/shell/secrets.sh" ]     && source "${{ALIASES_CONFIG_DIR}}/shell/secrets.sh"
[ -f "${{ALIASES_CONFIG_DIR}}/shell/prompt.sh" ]      && source "${{ALIASES_CONFIG_DIR}}/shell/prompt.sh"

# ── Bash aliases ────────────────────────────────────────────────────────────
for _ali_f in "${{ALIASES_CONFIG_DIR}}/bash_aliases/"*.ali.sh; do
    [ -f "$_ali_f" ] && source "$_ali_f"
done
unset _ali_f

# ── Bash completion ─────────────────────────────────────────────────────────
[ -f "${{ALIASES_CONFIG_DIR}}/bash_completion/aliases-completion.sh" ] && \\
    source "${{ALIASES_CONFIG_DIR}}/bash_completion/aliases-completion.sh"

# ── Prompt setup ────────────────────────────────────────────────────────────
aliases_setup_prompt 2>/dev/null || true

# ── Short alias ─────────────────────────────────────────────────────────────
alias c='aliases-cli code'
"""


def _backup_and_write(path: Path, content: str) -> None:
    backup = path.with_suffix(".bak")
    shutil.copy2(path, backup)
    click.echo(f"  Backed up: {backup}")
    path.write_text(content, encoding="utf-8")


def _ensure_bashrc(bashrc: Path, bash_aliases: Path) -> None:
    source_line = f'[ -f "{bash_aliases}" ] && source "{bash_aliases}"'
    if not bashrc.exists():
        bashrc.write_text(source_line + "\n", encoding="utf-8")
        click.echo(f"  Created {bashrc}")
        return

    content = bashrc.read_text(encoding="utf-8")
    if "bash_aliases" in content:
        click.echo(f"  {bashrc} already sources .bash_aliases — skipping.")
        return

    with open(bashrc, "a", encoding="utf-8") as fh:
        fh.write(f"\n# aliases-cli\n{source_line}\n")
    click.echo(f"  Updated {bashrc}")
