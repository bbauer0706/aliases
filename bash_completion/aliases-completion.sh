#!/bin/bash
##############################################################################
#                                                                            #
#                    BASH COMPLETION FOR ALIASES-CLI                        #
#                                                                            #
##############################################################################

# ---------------------------------------------------------------------------
# Full tab-completion for the 'aliases-cli' binary
# ---------------------------------------------------------------------------
_aliases_cli_completion() {
  local cur prev cword
  COMPREPLY=()
  cur="${COMP_WORDS[COMP_CWORD]}"
  prev="${COMP_WORDS[COMP_CWORD-1]}"
  cword=$COMP_CWORD

  # Top-level subcommands
  local -r _SUBCMDS="code c env secrets config pwd completion version help --help --version -h -v"

  # ---- word 1: complete the subcommand itself ----------------------------
  if [[ $cword -eq 1 ]]; then
    COMPREPLY=($(compgen -W "$_SUBCMDS" -- "$cur"))
    return 0
  fi

  local command="${COMP_WORDS[1]}"

  case "$command" in

    # ---- code / c : project names with optional [sw] bracket notation ---
    code|c)
      if [[ -z "$_ALIASES_PROJECTS_CACHE" ]]; then
        _ALIASES_PROJECTS_CACHE=$(aliases-cli completion projects 2>/dev/null)
      fi
      _complete_project_specs "$cur"
      ;;

    # ---- env : option flags ---------------------------------------------
    env)
      case "$prev" in
        -e)     COMPREPLY=($(compgen -W "dev staging prod" -- "$cur")) ;;
        -s|-i)  COMPREPLY=($(compgen -W "true false" -- "$cur")) ;;
        -t)     COMPREPLY=($(compgen -W "http https" -- "$cur")) ;;
        *)      COMPREPLY=($(compgen -W "-e -s -p -i -t --host -n --show --help -h" -- "$cur")) ;;
      esac
      ;;

    # ---- secrets : subcommands + flags ----------------------------------
    secrets)
      if [[ $cword -eq 2 ]]; then
        COMPREPLY=($(compgen -W "set get list delete remove rm load export rotate-master --help -h" -- "$cur"))
      fi
      ;;

    # ---- config : subcommands, keys, and value hints --------------------
    config)
      if [[ $cword -eq 2 ]]; then
        COMPREPLY=($(compgen -W "get set list ls reset edit path sync --help -h" -- "$cur"))

      elif [[ $cword -ge 3 ]]; then
        local config_sub="${COMP_WORDS[2]}"

        case "$config_sub" in
          get|set|reset)
            if [[ $cword -eq 3 ]]; then
              # Fetch config keys from the binary (cached per session)
              if [[ -z "$_ALIASES_CONFIG_KEYS_CACHE" ]]; then
                _ALIASES_CONFIG_KEYS_CACHE=$(aliases-cli completion config-keys 2>/dev/null)
              fi
              COMPREPLY=($(compgen -W "$_ALIASES_CONFIG_KEYS_CACHE" -- "$cur"))

            elif [[ $cword -eq 4 && "$config_sub" == "set" ]]; then
              # Value hints keyed on the config key
              local key="${COMP_WORDS[3]}"
              case "$key" in
                general.terminal_colors|general.confirm_destructive_actions|\
                code.reuse_window|\
                sync.enabled|sync.auto_sync|prompt.enabled)
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
              # Optional method argument for: config sync setup <url> [method]
              COMPREPLY=($(compgen -W "git rsync file http" -- "$cur"))
            fi
            ;;
        esac
      fi
      ;;

    # ---- pwd : flags ----------------------------------------------------
    pwd)
      COMPREPLY=($(compgen -W "--no-color --ps1 --help -h" -- "$cur"))
      ;;

    # ---- completion : internal subcommands ------------------------------
    completion)
      if [[ $cword -eq 2 ]]; then
        COMPREPLY=($(compgen -W "projects components config-keys" -- "$cur"))
      elif [[ $cword -eq 3 && "${COMP_WORDS[2]}" == "components" ]]; then
        # complete project name for: completion components <project>
        if [[ -z "$_ALIASES_PROJECTS_CACHE" ]]; then
          _ALIASES_PROJECTS_CACHE=$(aliases-cli completion projects 2>/dev/null)
        fi
        local names=()
        while IFS='|' read -r display_name _rest; do
          [[ -n "$display_name" ]] && names+=("$display_name")
        done <<< "$_ALIASES_PROJECTS_CACHE"
        COMPREPLY=($(compgen -W "${names[*]}" -- "$cur"))
      fi
      ;;

  esac
  return 0
}

complete -F _aliases_cli_completion aliases-cli

# ---------------------------------------------------------------------------
# Clear caches when the completion file is (re-)sourced
# ---------------------------------------------------------------------------
unset _ALIASES_CONFIG_KEYS_CACHE

# ---------------------------------------------------------------------------
# Tab completion for the 'c' command (aliases-cli code shorthand)
# ---------------------------------------------------------------------------
_aliases_code_completion() {
  local cur prev words cword
  
  # Try to use _init_completion if available, otherwise fallback
  if type -t _init_completion >/dev/null; then
    _init_completion || return
  else
    # Manual initialization for systems without bash-completion package
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    cword=$COMP_CWORD
  fi

  # Get current word being completed
  cur=${COMP_WORDS[COMP_CWORD]}
  
  # Cache project data (only fetch once per completion session)
  if [[ -z "$_ALIASES_PROJECTS_CACHE" ]]; then
    _ALIASES_PROJECTS_CACHE=$(aliases-cli completion projects 2>/dev/null)
  fi
  
  # If we're completing the first argument after 'c'
  if [[ $cword -eq 1 ]]; then
    _complete_project_specs "$cur"
  else
    # For multiple projects, complete each additional argument as a project
    _complete_project_specs "$cur"
  fi
  
  return 0
}

# Complete project specifications (projects with optional s/w suffixes)
_complete_project_specs() {
  local cur="$1"
  local projects=()
  
  # Handle bracket notation completion: proj[sw]
  if [[ "$cur" =~ ^(.+)\[([sw]*)$ ]]; then
    local base="${BASH_REMATCH[1]}"
    local existing_variants="${BASH_REMATCH[2]}"
    
    # Find the project info from cache
    local project_line
    project_line=$(echo "$_ALIASES_PROJECTS_CACHE" | grep "^$base|")
    
    if [[ -n "$project_line" ]]; then
      # Parse project line: display_name|full_name|has_server|has_web
      IFS='|' read -r display_name full_name has_server has_web <<< "$project_line"
      
      # Build completion suggestions
      local missing_variants=""
      
      # Add 's' if not present and server component exists
      if [[ ! "$existing_variants" =~ s && "$has_server" == "s" ]]; then
        missing_variants+="s"
      fi
      
      # Add 'w' if not present and web component exists  
      if [[ ! "$existing_variants" =~ w && "$has_web" == "w" ]]; then
        missing_variants+="w"
      fi
      
      # Generate completions
      for (( i=0; i<${#missing_variants}; i++ )); do
        local variant="${missing_variants:$i:1}"
        projects+=("${base}[${existing_variants}${variant}")
      done
      
      # Add closing bracket option if we have some variants
      if [[ -n "$existing_variants" ]]; then
        projects+=("${base}[${existing_variants}]")
      fi
    fi
    
  # Handle project completion with current input
  elif [[ -n "$cur" ]]; then
    # Parse each project line and add matching completions
    while IFS='|' read -r display_name full_name has_server has_web; do
      if [[ -n "$display_name" ]]; then
        # Basic project name
        if [[ "$display_name" == "$cur"* ]]; then
          projects+=("$display_name")
        fi

        # Bracket notation (only if it matches and components exist)
        local variants=""
        if [[ "$has_server" == "s" ]]; then
          variants+="s"
        fi
        if [[ "$has_web" == "w" ]]; then
          variants+="w"
        fi
        if [[ -n "$variants" && "$display_name[$variants]" == "$cur"* ]]; then
          projects+=("$display_name[$variants]")
        fi
      fi
    done <<< "$_ALIASES_PROJECTS_CACHE"

  else
    # No current input - show projects with bracket notation only
    while IFS='|' read -r display_name full_name has_server has_web; do
      if [[ -n "$display_name" ]]; then
        # Add bracket notation if components are available, otherwise just project name
        local variants=""
        if [[ "$has_server" == "s" ]]; then
          variants+="s"
        fi
        if [[ "$has_web" == "w" ]]; then
          variants+="w"
        fi

        if [[ -n "$variants" ]]; then
          projects+=("$display_name[$variants]")
        else
          projects+=("$display_name")
        fi
      fi
    done <<< "$_ALIASES_PROJECTS_CACHE"
  fi
  
  # Remove duplicates from projects array using associative array for efficiency
  local unique_projects=()
  declare -A seen_map

  for project in "${projects[@]}"; do
    if [[ -z "${seen_map[$project]}" ]]; then
      unique_projects+=("$project")
      seen_map["$project"]=1
    fi
  done

  # Generate completions
  COMPREPLY=($(compgen -W "${unique_projects[*]}" -- "$cur"))

  # Handle special cases for bracket completions
  if [[ ${#COMPREPLY[@]} -eq 1 && ${COMPREPLY[0]} =~ \[([sw]+)$ ]]; then
    # If we have a single bracket completion without closing bracket, add it
    if [[ ! ${COMPREPLY[0]} =~ \] ]]; then
      COMPREPLY[0]="${COMPREPLY[0]}]"
    fi
  fi
  
  return 0
}

# Register completion for the 'c' command
complete -F _aliases_code_completion c

# Clear caches on new shell or when completion is reloaded
unset _ALIASES_PROJECTS_CACHE
