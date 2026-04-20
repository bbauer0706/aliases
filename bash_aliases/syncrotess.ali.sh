#!/bin/bash
##############################################################################
#                                                                            #
#                       SYNCROTESS / AUTOBUILD SHORTCUTS                     #
#                                                                            #
# Discovers the projectbuilds scripts directory in this order:               #
#   1. $PRJBUILDS_DIR                        (explicit override)             #
#   2. /nethome/svc-gb20-road/syncrotess/scripts                             #
#   3. $HOME/syncrotess/scripts                                              #
#   4. $HOME/workspaces/projectbuilds                                        #
# If no directory is found, all commands silently do nothing.                #
#                                                                            #
# Version is matched against product script filenames by substring glob.    #
# When multiple scripts match, the shortest name wins (i.e. the base        #
# variant is preferred over mssql or other specialisations).                #
# After prjsel the version is remembered in $PRJBUILDS_VERSION so           #
# subsequent commands don't need it repeated.                                #
#                                                                            #
# Usage:                                                                     #
#   prjsel <version>            project selection  (sets PRJBUILDS_VERSION)  #
#   prjserver [version]         update server only                           #
#   prjsst    [version]         update interfaces only                       #
#   prjwebtools [version]       update web-tools only                        #
#   prjupdate [version]         server + interfaces + webtools               #
#   prjstart  [version]         start system                                 #
#   prjstop   [version]         stop system                                  #
#   prjtest   [version]         run testcase                                 #
#   prjbuild  <version> <cmd> [flags]   raw wrapper                         #
#   prjscripts                  list available product scripts               #
#   prjpull                     git pull the scripts repo                    #
#                                                                            #
# Examples:                                                                  #
#   prjsel master               # full build + prjsel, remembers 'master'   #
#   prjsel 10.1.9                                                            #
#   prjserver                   # reuses PRJBUILDS_VERSION                  #
#   prjbuild master server --force                                           #
#                                                                            #
##############################################################################

_prjbuilds_find_dir() {
    if [[ -n "${PRJBUILDS_DIR}" ]]; then
        echo "${PRJBUILDS_DIR}"
        return 0
    fi
    local candidates=(
        "/nethome/svc-gb20-road/syncrotess/scripts"
        "${HOME}/syncrotess/scripts"
        "${HOME}/workspaces/projectbuilds"
    )
    for d in "${candidates[@]}"; do
        [[ -d "${d}/product" ]] && { echo "${d}"; return 0; }
    done
    return 1   # not found — callers treat this as "silently do nothing"
}

_prjbuilds_find_script() {
    local dir="$1" version="$2"
    # collect all matches
    local all=( "${dir}/product/"*"${version}"*.sh )
    local found=()
    for f in "${all[@]}"; do
        [[ -f "${f}" ]] && found+=("${f}")
    done

    if [[ ${#found[@]} -eq 0 ]]; then
        echo "prjbuild: no product script found matching '${version}'" >&2
        echo "  Available: $(ls -1 "${dir}/product/"*.sh 2>/dev/null | xargs -I{} basename {} | paste -sd ' ')" >&2
        return 1
    fi

    if [[ ${#found[@]} -gt 1 ]]; then
        # prefer shortest basename (base variant over mssql / other suffixes)
        local best="${found[0]}"
        for f in "${found[@]:1}"; do
            [[ ${#f} -lt ${#best} ]] && best="${f}"
        done
        echo "${best}"
    else
        echo "${found[0]}"
    fi
}

# Core dispatcher.  Usage: _prjbuild_run <autobuild-cmd> [version] [extra-flags...]
_prjbuild_run() {
    local cmd="$1"; shift

    local version="${1:-${PRJBUILDS_VERSION}}"
    if [[ -z "${version}" ]]; then
        echo "prjbuild: no version given and PRJBUILDS_VERSION is not set." >&2
        echo "  Usage: ${cmd} <version>   e.g. ${cmd} master  or  ${cmd} 10.1.9" >&2
        return 1
    fi
    [[ $# -gt 0 ]] && shift   # consume version; rest are extra flags for autobuild

    local dir
    dir=$(_prjbuilds_find_dir) || return 0  # directory not found — do nothing silently

    local script
    script=$(_prjbuilds_find_script "${dir}" "${version}") || return 1

    # remember the version so follow-up commands don't need to repeat it
    [[ "${cmd}" == "prjsel" ]] && export PRJBUILDS_VERSION="${version}"

    source "${script}" "${cmd}" "$@"
}

# --- public commands ---

prjsel()      { _prjbuild_run prjsel     "$@"; }
prjserver()   { _prjbuild_run server     "$@"; }
prjsst()      { _prjbuild_run sst        "$@"; }
prjwebtools() { _prjbuild_run webtools   "$@"; }
prjupdate()   { _prjbuild_run update     "$@"; }
prjstart()    { _prjbuild_run start      "$@"; }
prjstop()     { _prjbuild_run stop       "$@"; }
prjtest()     { _prjbuild_run test       "$@"; }

# Raw wrapper: prjbuild <version> <command> [flags]
prjbuild() {
    local version="$1" cmd="${2:-}"
    shift; [[ $# -gt 0 ]] && shift
    _prjbuild_run "${cmd}" "${version}" "$@"
}

# List available product scripts
prjscripts() {
    local dir
    dir=$(_prjbuilds_find_dir) || { echo "prjbuild: scripts directory not found." >&2; return 1; }
    ls -1 "${dir}/product/"*.sh 2>/dev/null | xargs -I{} basename {}
}

# Pull latest autobuild scripts
prjpull() {
    local dir
    dir=$(_prjbuilds_find_dir) || { echo "prjbuild: scripts directory not found." >&2; return 1; }
    echo "Updating autobuild scripts in ${dir} ..."
    git -C "${dir}" pull
}

# ---------------------------------------------------------------------------
# relink — repoint an _Actual.jar symlink in $INSTROOT/bin to a dev jar.
#
# Usage:  relink <jar-file>
#
# <jar-file> is a versioned .jar in the current directory (e.g. ~/link).
# The prefix is extracted as everything before the first '-', then the
# matching  $INSTROOT/bin/<prefix>_Actual.jar  symlink is repointed to it.
#
# Example:
#   cd ~/link
#   relink SyncroTessErpClient-main-dev.jar
#   → ln -sf ~/link/SyncroTessErpClient-main-dev.jar \
#             $INSTROOT/bin/SyncroTessErpClient_Actual.jar
# ---------------------------------------------------------------------------
relink() {
    local jar_file="${1:-}"
    if [[ -z "${jar_file}" ]]; then
        echo "relink: usage: relink <jar-file>" >&2
        return 1
    fi

    if [[ -z "${INSTROOT}" ]]; then
        echo "relink: INSTROOT is not set." >&2
        return 1
    fi

    # Resolve to absolute path
    local abs_path
    case "${jar_file}" in
        /*) abs_path="${jar_file}" ;;
        *)  abs_path="${PWD}/${jar_file}" ;;
    esac

    if [[ ! -f "${abs_path}" ]]; then
        echo "relink: file not found: ${abs_path}" >&2
        return 1
    fi

    # Extract the name prefix (everything before the first '-')
    local bn
    bn="$(basename "${abs_path}")"
    local prefix="${bn%%-*}"

    local actual="${INSTROOT}/bin/${prefix}_Actual.jar"
    if [[ ! -e "${actual}" && ! -L "${actual}" ]]; then
        echo "relink: no matching _Actual.jar for prefix '${prefix}' in ${INSTROOT}/bin" >&2
        echo "  Expected: ${actual}" >&2
        return 1
    fi

    echo "Relinking: ${actual}"
    echo "       -> ${abs_path}"
    ln -sf "${abs_path}" "${actual}"
}

_relink_completion() {
    local cur="${COMP_WORDS[COMP_CWORD]}"
    COMPREPLY=( $(compgen -f -X '!*.jar' -- "${cur}") )
}
complete -o filenames -F _relink_completion relink

# ---------------------------------------------------------------------------
# stpush — SCP a dev jar to ~/link on the Syncrotess VM and relink it there.
#
# Usage:  stpush <jar-file>
#
# Copies <jar-file> to ${SYNCROTESS_REMOTE_HOST:-p23-cmpl09}:~/link/, then
# runs the equivalent of 'relink' on the remote via SSH:
#   ln -sf ~/link/<jar>  $INSTROOT/bin/<prefix>_Actual.jar
#
# Override defaults with env vars:
#   SYNCROTESS_REMOTE_HOST   (default: p23-cmpl09)
#   SYNCROTESS_REMOTE_USER   (default: current $USER)
# ---------------------------------------------------------------------------
stpush() {
    local jar_file="${1:-}"
    if [[ -z "${jar_file}" ]]; then
        echo "stpush: usage: stpush <jar-file>" >&2
        return 1
    fi

    local remote_host="${SYNCROTESS_REMOTE_HOST:-p23-cmpl09}"
    local remote_user="${SYNCROTESS_REMOTE_USER:-${USER}}"
    local remote_target="${remote_user}@${remote_host}"

    # Resolve to absolute path
    local abs_path
    case "${jar_file}" in
        /*) abs_path="${jar_file}" ;;
        *)  abs_path="${PWD}/${jar_file}" ;;
    esac

    if [[ ! -f "${abs_path}" ]]; then
        echo "stpush: file not found: ${abs_path}" >&2
        return 1
    fi

    local bn
    bn="$(basename "${abs_path}")"

    echo "Pushing ${bn} → ${remote_target}:~/link/"
    scp "${abs_path}" "${remote_target}:~/link/"
}

_stpush_completion() {
    local cur="${COMP_WORDS[COMP_CWORD]}"
    COMPREPLY=( $(compgen -f -X '!*.jar' -- "${cur}") )
}
complete -o filenames -F _stpush_completion stpush
