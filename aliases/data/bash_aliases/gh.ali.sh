#!/usr/bin/env bash
# GitHub CLI shortcuts for your own repositories.
#
#   ghc [-r] [query]   pick one of your repos → clone into the workspace dir → cd
#   ghpr [flags]       open a PR from the current branch (pushes it first)
#   ghco               pick an open PR → check it out
#   ghm                squash-merge the current PR and delete the branch
#   gho                open the current PR — or the repo — in the browser
#   ghci / ghciv / ghcir   watch checks / inspect a run / rerun failed jobs
#
# List someone else's repos for one shell:  GH_OWNER=someorg ghc
#
# No source-time `command -v gh` guard on purpose — see the note in
# glab.ali.sh: ~/.bash_aliases is sourced before the tail of ~/.bashrc extends
# PATH. Nothing here runs at source time, so the checks live in the commands.

_gh_require() {
    local bin
    for bin in "$@"; do
        command -v "$bin" &>/dev/null || { echo "${FUNCNAME[1]}: $bin is required" >&2; return 1; }
    done
}

# ---------------------------------------------------------------------------
# Repo list cache — ~/.cache/aliases/gh-repos-<owner>.txt, 24h TTL
# ---------------------------------------------------------------------------

_gh_refresh_repo_cache() {
    local file="$1"
    echo "Fetching ${GH_OWNER:-your} repositories…" >&2
    if gh repo list ${GH_OWNER:+"$GH_OWNER"} --limit 200 --no-archived \
        --json nameWithOwner --jq '.[].nameWithOwner' > "${file}.tmp" 2>/dev/null &&
       [[ -s "${file}.tmp" ]]
    then
        mv -f "${file}.tmp" "$file"
    else
        rm -f "${file}.tmp"
        [[ -s "$file" ]] || { echo "ghc: could not list ${GH_OWNER:-your} repositories" >&2; return 1; }
        echo "ghc: refresh failed, using the cached list" >&2
    fi
}

_gh_repo_cache() {
    local force="$1" dir file owner
    dir="${XDG_CACHE_HOME:-$HOME/.cache}/aliases"
    mkdir -p "$dir" || return 1
    owner="${GH_OWNER:-self}"
    file="$dir/gh-repos-${owner}.txt"

    if [[ -z "$force" && -s "$file" ]]; then
        if [[ -z $(find "$file" -mmin -1440 2>/dev/null) ]]; then
            _gh_refresh_repo_cache "$file" >/dev/null 2>&1 &
        fi
        echo "$file"
        return 0
    fi

    _gh_refresh_repo_cache "$file" || return 1
    echo "$file"
}

# First existing directory from projects.workspace_directories.
# ponytail: duplicated from glab.ali.sh — two copies is cheaper than inventing a
# shared bash lib; fold them together if a third file needs it.
_gh_clone_dir() {
    aliases config get projects.workspace_directories 2>/dev/null | python3 -c 'import json, os, sys
dirs = []
try:
    dirs = json.load(sys.stdin)
except Exception:
    pass
for d in dirs:
    d = os.path.expanduser(d)
    if os.path.isdir(d):
        print(d)
        break
else:
    print(os.path.expanduser("~/workspaces"))'
}

# ---------------------------------------------------------------------------
# ghc — pick one of your repos, clone it into the workspace dir, cd there
# ---------------------------------------------------------------------------
ghc() {
    local force= cache pick name target
    [[ "$1" == "-r" ]] && { force=1; shift; }
    _gh_require gh fzf || return 1
    cache=$(_gh_repo_cache "$force") || return 1

    pick=$(fzf --query "${1:-}" --select-1 \
            --preview 'gh repo view {} 2>/dev/null | head -40' \
            --preview-window=right:50% \
            --header="${GH_OWNER:-your} repositories  (Enter to clone + cd · ghc -r refreshes)" \
            --color='header:italic' < "$cache") || return
    [[ -n "$pick" ]] || return

    # Clone flat: the project mapper only scans one level under each workspace
    # directory, so a nested checkout would be invisible to `c`.
    name="${pick##*/}"
    target="$(_gh_clone_dir)/${name}"

    if [[ ! -d "$target" ]]; then
        echo "Cloning ${pick} → ${target}"
        gh repo clone "$pick" "$target" || return 1
    fi

    cd "$target" || return 1
    echo "→ c ${name}"
}

# ---------------------------------------------------------------------------
# Pull requests
# ---------------------------------------------------------------------------

_gh_branch_guard() {
    if ! git rev-parse --is-inside-work-tree &>/dev/null; then
        echo "Error: not in a git repository" >&2
        return 1
    fi
    local branch
    branch=$(git rev-parse --abbrev-ref HEAD)
    if [[ "$branch" == "main" || "$branch" == "master" ]]; then
        echo "Error: refusing to open a pull request from '$branch'" >&2
        return 1
    fi
}

# Open a PR from the current branch. Extra flags pass through, e.g. ghpr --draft
ghpr() {
    _gh_require gh || return 1
    _gh_branch_guard || return 1

    # Unlike glab's --fill, gh does not push for you — it prompts instead.
    # Pushing up front keeps this non-interactive.
    if ! git rev-parse --abbrev-ref '@{u}' &>/dev/null; then
        git push -u origin HEAD || return 1
    fi

    gh pr create --fill "$@"
}

# Pick one of the open PRs and check it out.
ghco() {
    local pick num
    _gh_require gh || return 1
    command -v fzf &>/dev/null || { gh pr list "$@"; return; }

    pick=$(
        gh pr list "$@" --json number,title,headRefName,author \
            --jq '.[] | "\(.number)\t\(.title)\t[\(.headRefName)]\t@\(.author.login)"' 2>/dev/null |
        fzf --delimiter='\t' \
            --preview 'gh pr view {1} 2>/dev/null' \
            --preview-window=right:55% \
            --header='Open pull requests  (Enter to check out)' \
            --color='header:italic' \
            --exit-0
    ) || return

    num="${pick%%$'\t'*}"
    [[ -n "$num" ]] && gh pr checkout "$num"
}

alias ghm='gh pr merge --squash --delete-branch'

# Open the current PR in the browser, falling back to the repo page.
gho() {
    gh pr view --web 2>/dev/null || gh browse
}

# ---------------------------------------------------------------------------
# Actions — `gh run` already does the work, these are just short names
# ---------------------------------------------------------------------------

alias ghci='gh pr checks --watch'    # checks for the current branch's PR, live
alias ghciv='gh run view'            # pick a run, drill into jobs and logs
alias ghcir='gh run rerun --failed'  # rerun just the failed jobs
