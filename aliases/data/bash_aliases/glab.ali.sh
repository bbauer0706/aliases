#!/usr/bin/env bash
# glab shortcuts, scoped to one GitLab group (default: evotess).
# No-ops silently if glab is not installed.
#
# glab already covers MRs, pipelines and browsing well. The only thing it does
# not solve is *finding* a repo: `glab repo clone` wants the full nested path
# and `glab repo list` re-hits the API every time. Hence the cached fzf picker.
#
#   glc [-r] [query]   pick a group repo → clone into the workspace dir → cd
#   glmr [flags]       open an MR from the current branch (fill/squash/cleanup)
#   glco               pick an open MR → check it out
#   glmm [flags]       merge the current MR
#   glo                open the current MR (or the repo) in the browser
#   glci / glciv / glcir   pipeline status / job TUI / retry
#
# Override the group per shell:  GLAB_GROUP=othergroup glc

# No source-time `command -v glab` guard on purpose: ~/.bashrc sources
# ~/.bash_aliases before the tail of the file extends PATH (brew shellenv,
# nvm, …), so a tool installed there is not yet visible while this runs.
# Nothing here executes at source time, so the check belongs in the commands.

: "${GLAB_GROUP:=evotess}"

# ---------------------------------------------------------------------------
# Repo list cache — ~/.cache/aliases/glab-repos-<group>.txt, 24h TTL
# ---------------------------------------------------------------------------

# Echoes the cache file path on stdout, refreshing it first when stale.
# $1 non-empty forces a refresh.
_glab_repo_cache() {
    local force="$1" dir file
    dir="${XDG_CACHE_HOME:-$HOME/.cache}/aliases"
    mkdir -p "$dir" || return 1
    file="$dir/glab-repos-${GLAB_GROUP}.txt"

    if [[ -z "$force" && -s "$file" && -n $(find "$file" -mmin -1440 2>/dev/null) ]]; then
        echo "$file"
        return 0
    fi

    echo "Fetching ${GLAB_GROUP} repositories…" >&2
    # `glab api` has no --jq, so parse the ndjson stream ourselves. Write to a
    # temp file first: a failed fetch must not truncate a usable cache.
    if glab api \
        "groups/${GLAB_GROUP}/projects?include_subgroups=true&simple=true&archived=false&per_page=100" \
        --paginate --output ndjson 2>/dev/null |
       python3 -c 'import json, sys
for line in sys.stdin:
    line = line.strip()
    if line:
        print(json.loads(line)["path_with_namespace"])' > "${file}.tmp" && [[ -s "${file}.tmp" ]]
    then
        mv -f "${file}.tmp" "$file"
    else
        rm -f "${file}.tmp"
        [[ -s "$file" ]] || { echo "glc: could not list ${GLAB_GROUP} repositories" >&2; return 1; }
        echo "glc: refresh failed, using the cached list" >&2
    fi
    echo "$file"
}

# First existing directory from projects.workspace_directories.
_glab_clone_dir() {
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
# glc — pick a repo from the group, clone it into the workspace dir, cd there
# ---------------------------------------------------------------------------
glc() {
    local force= cache pick name target bin
    [[ "$1" == "-r" ]] && { force=1; shift; }

    for bin in glab fzf; do
        command -v "$bin" &>/dev/null || { echo "glc: $bin is required" >&2; return 1; }
    done
    cache=$(_glab_repo_cache "$force") || return 1

    pick=$(fzf --query "${1:-}" --select-1 \
               --preview 'glab repo view {} 2>/dev/null | head -40' \
               --preview-window=right:50% \
               --header="${GLAB_GROUP} repositories  (Enter to clone + cd · glc -r refreshes)" \
               --color='header:italic' < "$cache") || return
    [[ -n "$pick" ]] || return

    # Clone flat under the workspace dir: the project mapper only scans one
    # level deep, so a nested checkout would be invisible to `c`.
    name="${pick##*/}"
    target="$(_glab_clone_dir)/${name}"

    if [[ ! -d "$target" ]]; then
        echo "Cloning ${pick} → ${target}"
        glab repo clone "$pick" "$target" || return 1
    fi

    cd "$target" || return 1
    echo "→ c ${name}"
}

# ---------------------------------------------------------------------------
# Merge requests
# ---------------------------------------------------------------------------

_glab_branch_guard() {
    if ! git rev-parse --is-inside-work-tree &>/dev/null; then
        echo "Error: not in a git repository" >&2
        return 1
    fi
    local branch
    branch=$(git rev-parse --abbrev-ref HEAD)
    if [[ "$branch" == "main" || "$branch" == "master" ]]; then
        echo "Error: refusing to open a merge request from '$branch'" >&2
        return 1
    fi
}

# Open an MR from the current branch. Extra flags pass through, e.g. glmr --draft
glmr() {
    _glab_branch_guard || return 1
    glab mr create --fill --yes --remove-source-branch --squash-before-merge "$@"
}

# Pick one of the open MRs and check it out.
glco() {
    local pick iid
    command -v fzf &>/dev/null || { glab mr list "$@"; return; }

    pick=$(
        glab mr list "$@" --output json \
            --jq '.[] | "\(.iid)\t\(.title)\t[\(.source_branch)]\t@\(.author.username | split("_")[0])"' 2>/dev/null |
        fzf --delimiter='\t' \
            --preview 'glab mr view {1} 2>/dev/null' \
            --preview-window=right:55% \
            --header='Open merge requests  (Enter to check out)' \
            --color='header:italic' \
            --exit-0
    ) || return

    iid="${pick%%$'\t'*}"
    [[ -n "$iid" ]] && glab mr checkout "$iid"
}

alias glmm='glab mr merge --yes'

# Open the current MR in the browser, falling back to the repo page.
glo() {
    glab mr view --web 2>/dev/null || glab repo view --web
}

# ---------------------------------------------------------------------------
# Pipelines — `glab ci` already does the work, these are just short names
# ---------------------------------------------------------------------------

alias glci='glab ci status --live'   # current branch pipeline, live
alias glciv='glab ci view'           # TUI: browse jobs, trace, retry, cancel
alias glcir='glab ci retry'
