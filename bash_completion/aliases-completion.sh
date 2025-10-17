#!/bin/bash
##############################################################################
#                                                                            #
#                    BASH COMPLETION FOR ALIASES-CLI                        #
#                                                                            #
##############################################################################

# Tab completion for the 'c' command (aliases-cli code)
_aliases_code_completion() {
  local cur cword

  # Try to use _init_completion if available, otherwise fallback
  if type -t _init_completion >/dev/null; then
    _init_completion || return
  else
    # Manual initialization for systems without bash-completion package
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
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
      IFS='|' read -r display_name _ has_server has_web <<< "$project_line"

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
    while IFS='|' read -r display_name _ has_server has_web; do
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
        if [[ -n "$variants" && "${display_name}[${variants}]" == "$cur"* ]]; then
          projects+=("${display_name}[${variants}]")
        fi
      fi
    done <<< "$_ALIASES_PROJECTS_CACHE"

  else
    # No current input - show projects with bracket notation only
    while IFS='|' read -r display_name _ has_server has_web; do
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
          projects+=("${display_name}[${variants}]")
        else
          projects+=("${display_name}")
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
  mapfile -t COMPREPLY < <(compgen -W "${unique_projects[*]}" -- "$cur")

  # Handle special cases for bracket completions
  if [[ ${#COMPREPLY[@]} -eq 1 && ${COMPREPLY[0]} =~ \[([sw]+)$ ]]; then
    # If we have a single bracket completion without closing bracket, add it
    if [[ ! ${COMPREPLY[0]} =~ \] ]]; then
      COMPREPLY[0]="${COMPREPLY[0]}]"
    fi
  fi

  return 0
}

# Register completion for the 'c' command only (as requested)
complete -F _aliases_code_completion c

# Clear cache on new shell or when completion is reloaded
unset _ALIASES_PROJECTS_CACHE
