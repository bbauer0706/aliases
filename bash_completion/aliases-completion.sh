#!/bin/bash
##############################################################################
#                                                                            #
#                    BASH COMPLETION FOR ALIASES-CLI                        #
#                                                                            #
##############################################################################

# Tab completion for the 'c' command (aliases-cli code)
_aliases_code_completion() {
  local cur prev words cword
  _init_completion || return

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
    
  # Handle composite project completion: dipws, dipsw, etc.
  elif [[ -n "$cur" ]]; then
    # Parse each project line and add completions
    while IFS='|' read -r display_name full_name has_server has_web; do
      if [[ -n "$display_name" ]]; then
        # Basic project name completion - only if project name starts with current input
        if [[ "$display_name" == "$cur"* ]]; then
          projects+=("$display_name")
          
          # Add component completions
          if [[ "$has_server" == "s" ]]; then
            projects+=("${display_name}s")
          fi
          if [[ "$has_web" == "w" ]]; then
            projects+=("${display_name}w")
          fi
          
          # Add bracket notation if components are available
          local variants=""
          if [[ "$has_server" == "s" ]]; then
            variants+="s"
          fi
          if [[ "$has_web" == "w" ]]; then
            variants+="w"
          fi
          
          if [[ -n "$variants" ]]; then
            projects+=("$display_name[$variants]")
          fi
        fi
        
        # Handle composite patterns (dipws, dipsw, etc.) - only for exact matches
        if [[ "$cur" == "${display_name}"* && ${#cur} -gt ${#display_name} ]]; then
          local suffix="${cur#$display_name}"
          
          # Check if suffix is valid combination of s and w
          if [[ "$suffix" =~ ^[sw]+$ ]]; then
            local valid=true
            
            # Validate each suffix character
            for (( i=0; i<${#suffix}; i++ )); do
              local char="${suffix:$i:1}"
              if [[ "$char" == "s" && "$has_server" != "s" ]]; then
                valid=false
                break
              elif [[ "$char" == "w" && "$has_web" != "w" ]]; then
                valid=false
                break
              fi
            done
            
            if [[ "$valid" == "true" ]]; then
              projects+=("$cur")
            fi
          fi
        fi
      fi
    done <<< "$_ALIASES_PROJECTS_CACHE"
    
  else
    # No current input - show all available projects with their variants
    while IFS='|' read -r display_name full_name has_server has_web; do
      if [[ -n "$display_name" ]]; then
        projects+=("$display_name")
        
        # Add component completions
        if [[ "$has_server" == "s" ]]; then
          projects+=("${display_name}s")
        fi
        if [[ "$has_web" == "w" ]]; then
          projects+=("${display_name}w")
        fi
        
        # Add bracket notation if components are available
        local variants=""
        if [[ "$has_server" == "s" ]]; then
          variants+="s"
        fi
        if [[ "$has_web" == "w" ]]; then
          variants+="w"
        fi
        
        if [[ -n "$variants" ]]; then
          projects+=("$display_name[$variants]")
        fi
      fi
    done <<< "$_ALIASES_PROJECTS_CACHE"
  fi
  
  # Generate completions
  COMPREPLY=($(compgen -W "${projects[*]}" -- "$cur"))
  
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
_ALIASES_PROJECTS_CACHE=""
