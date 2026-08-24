#!/usr/bin/env bash
# eza — a modern ls. No-ops silently when eza is not installed.
#
# Sourced after basic.ali.sh (the glob in ~/.bash_aliases is alphabetical), so
# the `la` here deliberately replaces the `ls -alh` one defined there. Without
# eza installed nothing below runs and basic.ali.sh's `la` stays in effect.
#
# ponytail: source-time guard, unlike gh/glab which check at call time. eza from
# a package manager lives in /usr/bin, already on PATH when this is sourced. Move
# the check into wrapper functions if you ever `cargo install` it to ~/.cargo/bin,
# which the tail of ~/.bashrc adds to PATH after this file is sourced.

command -v eza &>/dev/null || return 0

_eza_icons=$(aliases config get eza.icons 2>/dev/null || printf '%s' auto)
case $_eza_icons in
    auto|always|never) ;;
    *) _eza_icons=auto ;;
esac

# --icons=auto emits icons only to a TTY, so piped output stays clean.
_eza_opts="--group-directories-first --icons=$_eza_icons"

# Aliases are not expanded in scripts or non-interactive shells, and
# `command ls` still reaches coreutils — overriding ls here is safe.
# shellcheck disable=SC2139
{
    alias ls="eza $_eza_opts"
    alias ll="eza -lh --git $_eza_opts"    # long, with git status column
    alias la="eza -lha --git $_eza_opts"   # long + hidden
    alias lt="eza --tree --level=2 $_eza_opts"
    alias ltt="eza --tree --level=3 --long --no-permissions --no-user $_eza_opts"
}

unset _eza_icons _eza_opts
