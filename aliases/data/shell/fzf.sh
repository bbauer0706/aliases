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

# File previewer program: bat (or batcat on Debian/Ubuntu); empty if neither.
if command -v bat &>/dev/null; then
    _fzf_bat=bat
elif command -v batcat &>/dev/null; then
    _fzf_bat=batcat
else
    _fzf_bat=
fi

# Build a bat/nl whole-file preview command.
#   $1 = file placeholder (e.g. {} or {1})
#   $2 = optional line placeholder to highlight (e.g. {2})
_fzf_preview_cmd() {
    if [[ -n $_fzf_bat ]]; then
        printf '%s --style=numbers --color=always%s %s' \
            "$_fzf_bat" "${2:+ --highlight-line $2}" "$1"
    else
        printf 'nl -ba %s' "$1"
    fi
}

# Whole-file preview of {} for real file paths (Ctrl+T, ** completion).
_fzf_file_preview="$(_fzf_preview_cmd '{}')"

# --walker-skip applies to fzf's built-in file/dir walker (safe globally).
export FZF_DEFAULT_OPTS='--height 100% --border --layout reverse --info inline --walker-skip .git,node_modules,target,.cache'

# Ctrl+R: no preview — history lines are not file paths.
# Ctrl+R toggles sort, but only reorders once a query is typed (score-best-first
# vs. most-recent-first); with an empty query both orders are identical.
export FZF_CTRL_R_OPTS='--preview "" --header "History  (type to search · Ctrl+R re-sorts matches)" --color header:italic'

# Alt+C: directory picker
export FZF_ALT_C_OPTS='--preview "ls -1 {}"'

# Ctrl+T: file picker — same bat preview as Alt+F
export FZF_CTRL_T_OPTS="--preview '$_fzf_file_preview'"

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
        cd)                 fzf --preview 'ls -1 {}'                                                                            "$@" ;;
        vim|nvim|nano)      fzf --preview "$_fzf_file_preview"                                                                  "$@" ;;
        export|unset)       fzf --preview 'echo ${}'                                                                            "$@" ;;
        kill)               fzf --preview 'ps --pid={} -o pid,ppid,%cpu,%mem,cmd 2>/dev/null'                                   "$@" ;;
        which|type|command) fzf --preview 'type {}; p=$(command -v {} 2>/dev/null); [ -f "$p" ] && { echo; file -b "$p"; }'     "$@" ;;
        *)                  fzf                                                                                                 "$@" ;;
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
# Alt+G — fuzzy git commit picker → checkout the commit (preview: full diff)
# Checks out a specific commit from history (detached HEAD).
# ---------------------------------------------------------------------------
_aliases_fzf_git_commit() {
    local commit
    commit=$(
        git log --color=always \
            --format='%C(auto)%h %C(green)%cs%C(reset) %C(auto)%d %s %C(dim white)%an' 2>/dev/null |
        fzf --ansi \
            --preview 'git show --color=always --stat --patch {1} 2>/dev/null' \
            --preview-window=right:60% \
            --header='Commits  (Enter to checkout — detached HEAD)' \
            --color='header:italic' \
            --exit-0
    ) || return

    commit=$(awk '{print $1}' <<< "$commit")
    [[ -n "$commit" ]] && git checkout "$commit"
}

bind -x '"\eg": _aliases_fzf_git_commit'

# ---------------------------------------------------------------------------
# Alt+F — live ripgrep → open match in $EDITOR
# Requires: rg (ripgrep)
# ---------------------------------------------------------------------------

_aliases_fzf_live_grep() {
    local result file line preview
    # Whole-file preview, scrollable, jumped to the matched line.
    # bat highlights the match line ({2}); fall back to nl.
    preview="$(_fzf_preview_cmd '{1}' '{2}')"
    result=$(
        rg --color=always --line-number --no-heading --smart-case "" 2>/dev/null |
        fzf --ansi \
            --disabled \
            --bind 'change:reload:rg --color=always --line-number --no-heading --smart-case {q} 2>/dev/null || true' \
            --delimiter=: \
            --preview "$preview" \
            --preview-window='right,60%,+{2}+3/3' \
            --header='Live grep  ·  Alt+F  ·  type to search' \
            --color='header:italic' \
            --exit-0
    ) || return

    file=$(cut -d: -f1 <<< "$result")
    line=$(cut -d: -f2 <<< "$result")
    if [[ -n "$file" ]]; then
        # VS Code / VSCodium use `--goto file:line`; strip a blocking --wait
        # for a quick jump. Other editors use the vim-style `+line file`.
        local editor="${EDITOR:-vim}"
        case "$editor" in
            *code*|*codium*)
                READLINE_LINE="${editor/ --wait/} --goto ${file}:${line}" ;;
            *)
                READLINE_LINE="$editor +${line} ${file}" ;;
        esac
        READLINE_POINT=${#READLINE_LINE}
    fi
}

# Alt+F only if ripgrep is available.
command -v rg &>/dev/null && bind -x '"\ef": _aliases_fzf_live_grep'
