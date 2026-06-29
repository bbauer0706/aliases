#!/usr/bin/env bash
# ripgrep wrapper with sensible defaults.
# Falls back silently if rg is not installed.

command -v rg &>/dev/null || return 0

rg() {
    if [ -t 1 ]; then
        # Interactive: page output so colors and long results are readable
        command rg \
            --smart-case \
            --color=always \
            --colors 'match:fg:yellow' \
            --colors 'match:style:bold' \
            "$@" | less -RFX
    else
        # In a pipeline: pass through without pager or forced color
        command rg --smart-case "$@"
    fi
}
