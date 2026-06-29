#!/usr/bin/env bash
# fzf shell enhancements.
# Sourced automatically by ~/.bash_aliases when aliases setup has run.
# No-ops silently if fzf is not installed.
#
# This file is the single source of truth for all FZF_* env vars.
# It intentionally overrides anything set by system profile.d scripts.

command -v fzf &>/dev/null || return 0
# Only set up interactive features — fzf --bash guards itself with [[ $- =~ i ]]
[[ $- =~ i ]] || return 0

# ---------------------------------------------------------------------------
# FZF environment — override system/devbox defaults
# ---------------------------------------------------------------------------

export FZF_DEFAULT_OPTS='--height 50% --border --layout reverse --info inline'

# Ctrl+R: no preview — history lines are not file paths
export FZF_CTRL_R_OPTS='--preview "" --header "History  (Ctrl+R toggles sort)" --color header:italic'

# Alt+C: directory picker
export FZF_ALT_C_OPTS='--preview "ls -1 {}"'

# Ctrl+T: file picker — no preview by default
export FZF_CTRL_T_OPTS='--preview ""'

# ---------------------------------------------------------------------------
# Native fzf bash integration (Ctrl+R, Ctrl+T, Alt+C, ** completion)
# ---------------------------------------------------------------------------
eval "$(fzf --bash)"

# ---------------------------------------------------------------------------
# _fzf_comprun — per-command previews for **<TAB> fuzzy completion
# Usage: vim **<TAB>  /  cd **<TAB>  /  kill **<TAB>  (not regular Tab)
# ---------------------------------------------------------------------------
_fzf_comprun() {
    local cmd=$1; shift
    case "$cmd" in
        cd)             fzf --preview 'ls -1 {}'                                           "$@" ;;
        vim|nvim|nano)  fzf --preview 'head -100 {}'                                      "$@" ;;
        export|unset)   fzf --preview 'eval "echo \${}"'                                  "$@" ;;
        kill)           fzf --preview 'ps --pid={} -o pid,ppid,%cpu,%mem,cmd 2>/dev/null' "$@" ;;
        *)              fzf                                                                "$@" ;;
    esac
}

# ---------------------------------------------------------------------------
# Alt+B — fuzzy git branch picker (preview: recent log for that branch)
# ---------------------------------------------------------------------------
_aliases_fzf_git_branch() {
    local branch
    branch=$(
        git branch --all --color=always 2>/dev/null |
        grep -v HEAD |
        sed 's/^[* ]*//' |
        fzf --ansi \
            --preview 'git log --oneline --color=always {1} 2>/dev/null | head -20' \
            --preview-window=right:50% \
            --header='Branches  (Enter to checkout)' \
            --color='header:italic' \
            --exit-0
    ) || return

    branch=$(sed 's#^remotes/[^/]*/##' <<< "$branch" | xargs)
    [[ -n "$branch" ]] && git checkout "$branch"
}

bind -x '"\eb": _aliases_fzf_git_branch'

# ---------------------------------------------------------------------------
# Alt+F — live ripgrep → open match in $EDITOR
# Requires: rg (ripgrep)
# ---------------------------------------------------------------------------
command -v rg &>/dev/null || return 0

_aliases_fzf_live_grep() {
    local result file line
    result=$(
        rg --color=always --line-number --no-heading --smart-case "" 2>/dev/null |
        fzf --ansi \
            --disabled \
            --bind 'change:reload:rg --color=always --line-number --no-heading --smart-case {q} 2>/dev/null || true' \
            --delimiter=: \
            --preview 'sed -n "$(( {2} > 5 ? {2} - 5 : 1 )),$(( {2} + 15 ))p" {1}' \
            --preview-window='right:50%' \
            --header='Live grep  ·  Alt+F  ·  type to search' \
            --color='header:italic' \
            --exit-0
    ) || return

    file=$(cut -d: -f1 <<< "$result")
    line=$(cut -d: -f2 <<< "$result")
    [[ -n "$file" ]] && READLINE_LINE="${EDITOR:-vim} +${line} ${file}" && READLINE_POINT="${#READLINE_LINE}"
}

bind -x '"\ef": _aliases_fzf_live_grep'
