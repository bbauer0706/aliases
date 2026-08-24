#!/usr/bin/env bash
# GitHub CLI shortcuts for your own repositories.
#
# The GitLab twin of this file (glab.ali.sh) caches the repo list, because that
# group has ~300 repos behind a paginated API. A personal account is a single
# fast call, so there is nothing to cache here.
#
#   ghc [query]        pick one of your repos → clone into the workspace dir → cd
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
    local pick name target
    _gh_require gh fzf || return 1

    pick=$(gh repo list ${GH_OWNER:+"$GH_OWNER"} --limit 200 --no-archived \
               --json nameWithOwner --jq '.[].nameWithOwner' 2>/dev/null |
        fzf --query "${1:-}" --select-1 \
            --preview 'gh repo view {} 2>/dev/null | head -40' \
            --preview-window=right:50% \
            --header="${GH_OWNER:-your} repositories  (Enter to clone + cd)" \
            --color='header:italic') || return
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
