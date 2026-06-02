#!/usr/bin/env bash
# Bash tab-completion for aliases-cli and the 'c' shorthand.
# Sourced automatically by ~/.bash_aliases when set up via: aliases-cli setup

# Per-session caches (reset on re-source)
_ALIASES_PROJECTS_CACHE=""
_ALIASES_CONFIG_KEYS_CACHE=""

# ---------------------------------------------------------------------------
# _aliases_cli_completion – completes the 'aliases-cli' command
# ---------------------------------------------------------------------------
_aliases_cli_completion() {
    local cur prev cword
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    cword=$COMP_CWORD

    local -r _SUBCMDS="code c env secrets config pwd setup completion --help --version -h -v"

    # ── word 1: subcommand itself ─────────────────────────────────────────
    if [[ $cword -eq 1 ]]; then
        COMPREPLY=($(compgen -W "$_SUBCMDS" -- "$cur"))
        return 0
    fi

    local command="${COMP_WORDS[1]}"

    case "$command" in

        # ── code / c ──────────────────────────────────────────────────────
        code|c)
            [[ -z "$_ALIASES_PROJECTS_CACHE" ]] && \
                _ALIASES_PROJECTS_CACHE=$(aliases-cli completion projects 2>/dev/null)
            _aliases_complete_project_specs "$cur"
            ;;

        # ── env ───────────────────────────────────────────────────────────
        env)
            case "$prev" in
                -e)     COMPREPLY=($(compgen -W "dev staging prod" -- "$cur")) ;;
                -i)     COMPREPLY=($(compgen -W "true false" -- "$cur")) ;;
                -t)     COMPREPLY=($(compgen -W "plain compressed" -- "$cur")) ;;
                *)      COMPREPLY=($(compgen -W "-e -p -i -t -n --show --help -h" -- "$cur")) ;;
            esac
            ;;

        # ── secrets ───────────────────────────────────────────────────────
        secrets)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=($(compgen -W "set get list delete remove rm load rotate-master --help -h" -- "$cur"))
            fi
            ;;

        # ── config ────────────────────────────────────────────────────────
        config)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=($(compgen -W "get set list ls reset edit path sync --help -h" -- "$cur"))

            elif [[ $cword -ge 3 ]]; then
                local config_sub="${COMP_WORDS[2]}"
                case "$config_sub" in
                    get|set)
                        if [[ $cword -eq 3 ]]; then
                            [[ -z "$_ALIASES_CONFIG_KEYS_CACHE" ]] && \
                                _ALIASES_CONFIG_KEYS_CACHE=$(aliases-cli completion config-keys 2>/dev/null)
                            COMPREPLY=($(compgen -W "$_ALIASES_CONFIG_KEYS_CACHE" -- "$cur"))

                        elif [[ $cword -eq 4 && "$config_sub" == "set" ]]; then
                            local key="${COMP_WORDS[3]}"
                            case "$key" in
                                general.terminal_colors|general.confirm_destructive_actions|\
                                code.reuse_window|sync.enabled|sync.auto_sync|prompt.enabled)
                                    COMPREPLY=($(compgen -W "true false" -- "$cur")) ;;
                                general.verbosity)
                                    COMPREPLY=($(compgen -W "quiet normal verbose" -- "$cur")) ;;
                                code.fallback_behavior)
                                    COMPREPLY=($(compgen -W "always never auto" -- "$cur")) ;;
                                code.preferred_component)
                                    COMPREPLY=($(compgen -W "server web ask" -- "$cur")) ;;
                                env.default_env)
                                    COMPREPLY=($(compgen -W "dev staging prod" -- "$cur")) ;;
                                sync.method)
                                    COMPREPLY=($(compgen -W "git rsync file http" -- "$cur")) ;;
                                general.editor)
                                    COMPREPLY=($(compgen -W "code vim nvim nano emacs" -- "$cur")) ;;
                            esac
                        fi
                        ;;
                    sync)
                        if [[ $cword -eq 3 ]]; then
                            COMPREPLY=($(compgen -W "pull push status setup" -- "$cur"))
                        elif [[ $cword -eq 5 && "${COMP_WORDS[3]}" == "setup" ]]; then
                            COMPREPLY=($(compgen -W "git rsync file http" -- "$cur"))
                        fi
                        ;;
                esac
            fi
            ;;

        # ── pwd ───────────────────────────────────────────────────────────
        pwd)
            COMPREPLY=($(compgen -W "--no-color --ps1 --user-host-color --help -h" -- "$cur"))
            ;;

        # ── completion ────────────────────────────────────────────────────
        completion)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=($(compgen -W "projects components config-keys" -- "$cur"))
            elif [[ $cword -eq 3 && "${COMP_WORDS[2]}" == "components" ]]; then
                [[ -z "$_ALIASES_PROJECTS_CACHE" ]] && \
                    _ALIASES_PROJECTS_CACHE=$(aliases-cli completion projects 2>/dev/null)
                local names=()
                while IFS='|' read -r display_name _rest; do
                    [[ -n "$display_name" ]] && names+=("$display_name")
                done <<< "$_ALIASES_PROJECTS_CACHE"
                COMPREPLY=($(compgen -W "${names[*]}" -- "$cur"))
            fi
            ;;

        # ── setup ─────────────────────────────────────────────────────────
        setup)
            COMPREPLY=($(compgen -W "--force --update -f -u --help -h" -- "$cur"))
            ;;

    esac
    return 0
}

# ---------------------------------------------------------------------------
# _aliases_complete_project_specs – shared project name completer
#
# Output format from `aliases-cli completion projects`:
#   display_name|full_name|has_server|has_web   (has_* = "1" or "0")
# ---------------------------------------------------------------------------
_aliases_complete_project_specs() {
    local cur="$1"
    local projects=()

    # ── Bracket notation: proj[ … ────────────────────────────────────────
    if [[ "$cur" =~ ^(.+)\[([sw]*)$ ]]; then
        local base="${BASH_REMATCH[1]}"
        local existing="${BASH_REMATCH[2]}"
        local project_line
        project_line=$(echo "$_ALIASES_PROJECTS_CACHE" | grep "^${base}|")
        if [[ -n "$project_line" ]]; then
            IFS='|' read -r _dn _fn has_s has_w <<< "$project_line"
            [[ "$has_s" == "1" && "$existing" != *s* ]] && projects+=("${base}[${existing}s")
            [[ "$has_w" == "1" && "$existing" != *w* ]] && projects+=("${base}[${existing}w")
            [[ -n "$existing" ]] && projects+=("${base}[${existing}]")
        fi

    # ── Normal completion ─────────────────────────────────────────────────
    else
        while IFS='|' read -r display_name _full_name has_s has_w; do
            [[ -z "$display_name" ]] && continue
            # Plain project name
            [[ "$display_name" == "$cur"* ]] && projects+=("$display_name")
            # Suffix shorthands
            if [[ "$has_s" == "1" ]]; then
                [[ "${display_name}s" == "$cur"* ]] && projects+=("${display_name}s")
            fi
            if [[ "$has_w" == "1" ]]; then
                [[ "${display_name}w" == "$cur"* ]] && projects+=("${display_name}w")
            fi
            if [[ "$has_s" == "1" && "$has_w" == "1" ]]; then
                [[ "${display_name}sw" == "$cur"* ]] && projects+=("${display_name}sw")
                [[ "${display_name}ws" == "$cur"* ]] && projects+=("${display_name}ws")
            fi
            # Bracket notation
            local variants=""
            [[ "$has_s" == "1" ]] && variants+="s"
            [[ "$has_w" == "1" ]] && variants+="w"
            if [[ -n "$variants" && "${display_name}[${variants}]" == "$cur"* ]]; then
                projects+=("${display_name}[${variants}]")
            fi
        done <<< "$_ALIASES_PROJECTS_CACHE"
    fi

    COMPREPLY=($(compgen -W "${projects[*]}" -- "$cur"))
}

# ---------------------------------------------------------------------------
# Register completions
# ---------------------------------------------------------------------------
complete -F _aliases_cli_completion aliases-cli
complete -F _aliases_code_completion c

# ---------------------------------------------------------------------------
# _aliases_code_completion – completion for the 'c' bash alias
# ---------------------------------------------------------------------------
_aliases_code_completion() {
    COMPREPLY=()
    local cur="${COMP_WORDS[COMP_CWORD]}"
    [[ -z "$_ALIASES_PROJECTS_CACHE" ]] && \
        _ALIASES_PROJECTS_CACHE=$(aliases-cli completion projects 2>/dev/null)
    _aliases_complete_project_specs "$cur"
}
