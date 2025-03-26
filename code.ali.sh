#!/bin/bash
##############################################################################
#                                                                            #
#                          PROJECT NAVIGATION - CODE                         #
#                                                                            #
##############################################################################

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

_define_mappings_c() {
  local workspace_dir="$HOME/workspaces"
  
  # First, discover all projects in the workspace directory
  declare -gA full_paths=()
  for dir in "$workspace_dir"/*; do
    if [ -d "$dir" ]; then
      local full_name=$(basename "$dir")
      full_paths[$full_name]="$dir"
    fi
  done
  
  # Define full name to shortcut mapping
  declare -gA full_to_short=(
    [urm20]="urm"
    [otv20]="otv"
    [ragnarokui]="rak"
    [dispatch20]="dip"
  )
  
  # Custom component paths for specific projects (using full names)
  declare -gA server_paths=(
    [urm20]="serverJava"
  )
  
  declare -gA web_paths=(
    [urm20]="urm2"
  )
  
  # Color codes for display
  declare -g COLOR_RESET="\033[0m"
  declare -g COLOR_SERVER="\033[1;32m"  # Bright green/lime for server
  declare -g COLOR_WEB="\033[1;34m"     # Bright blue for web
  
  # Default paths for server and web components
  declare -ga default_server_paths=("java/serverJava" "serverJava")
  declare -ga default_web_paths=("webapp" "webApp" "web")
}

# Initialize mappings
_define_mappings_c

# Get full project name from either short or full name
_get_full_project_name() {
  local name="$1"
  
  # Check all full path names for a match
  for full_name in "${!full_paths[@]}"; do
    # Check if this is the full name directly
    if [[ "$full_name" == "$name" ]]; then
      echo "$full_name"
      return
    fi
    
    # Check if the short form matches
    if [[ -n "${full_to_short[$full_name]}" && "${full_to_short[$full_name]}" == "$name" ]]; then
      echo "$full_name"
      return
    fi
  done
  
  # No match found
  echo ""
}

# Get project path from either short or full name
_get_project_path() {
  local name="$1"
  local full_name=$(_get_full_project_name "$name")
  if [[ -n "$full_name" ]]; then
    echo "${full_paths[$full_name]}"
  else
    echo ""
  fi
}

# Get preferred display name (short if available, otherwise full)
_get_display_name() {
  local full_name="$1"
  if [[ -n "${full_to_short[$full_name]}" ]]; then
    echo "${full_to_short[$full_name]}"
  else
    echo "$full_name"
  fi
}

# Helper function to get the component path for a project
_get_component_path() {
  local name="$1"  # Either short or full project name
  local component_type="$2" # "server" or "web"
  local full_name=$(_get_full_project_name "$name")
  
  # If full name isn't found, exit early
  if [[ -z "$full_name" ]]; then
    echo ""
    return
  fi
  
  local project_path="${full_paths[$full_name]}"
  
  # Check if this project has a custom path defined
  if [[ "$component_type" == "server" && -n "${server_paths[$full_name]}" ]]; then
    echo "${server_paths[$full_name]}"
    return
  elif [[ "$component_type" == "web" && -n "${web_paths[$full_name]}" ]]; then
    echo "${web_paths[$full_name]}"
    return
  fi
  
  # Try default paths
  if [[ "$component_type" == "server" ]]; then
    for path in "${default_server_paths[@]}"; do
      if [[ -d "$project_path/$path" ]]; then
        echo "$path"
        return
      fi
    done
  elif [[ "$component_type" == "web" ]]; then
    for path in "${default_web_paths[@]}"; do
      if [[ -d "$project_path/$path" ]]; then
        echo "$path"
        return
      fi
    done
  fi
  
  # Return empty if no path found
  echo ""
}

# Check if a component exists for a project
_has_component() {
  local name="$1"
  local component_type="$2"
  local component_path=$(_get_component_path "$name" "$component_type")
  local project_path=$(_get_project_path "$name")
  
  if [[ -n "$component_path" && -d "$project_path/$component_path" ]]; then
    echo "true"
  else
    echo "false"
  fi
}

# ============================================================================
# MAIN FUNCTION
# ============================================================================

c() {
  # Help parameters
  if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    echo "VS Code project navigation:"
    echo "  c                - Open home directory"
    echo "  c <project>      - Open project"
    echo "  c <project>s     - Open server component of project"
    echo "  c <project>w     - Open web component of project"
    echo "  c <project>[sw]  - Open both server and web components"
    echo "  c <proj1> <proj2> ... - Open multiple projects"
    return
  fi

  # No parameters, open home directory
  if [[ $# -eq 0 ]]; then
    code "$HOME"
    return
  fi
  
  # Multiple parameters, open each project
  if [[ $# -gt 1 ]]; then
    for param in "$@"; do
      c "$param"
    done
    return
  fi

  # Check for pattern proj[sw]
  if [[ "$1" =~ (.+)\[(.*)\]$ ]]; then
    local proj="${BASH_REMATCH[1]}"
    local variants="${BASH_REMATCH[2]}"
    
    # Handle each variant separately
    for (( i=0; i<${#variants}; i++ )); do
      local variant="${variants:$i:1}"
      c "${proj}${variant}"
    done
    return
  fi

  # Extract base and suffix
  local input="$1"
  local suffix="${input: -1}"
  local base="${input%?}"
  
  # First, check if the exact input matches a project
  if [[ -n "$(_get_project_path "$input")" ]]; then
    # If full input is a valid project, use it as is
    suffix=""
    base="$input"
  elif [[ "$suffix" == "s" || "$suffix" == "w" ]]; then
    # Check if base is a valid project when removing the suffix
    if [[ -z "$(_get_project_path "$base")" ]]; then
      # Base is not a valid project, treat the whole input as a project
      suffix=""
      base="$input"
    fi
    # Otherwise, keep the split as is (base + suffix)
  else
    # Neither 's' nor 'w' suffix, treat whole string as base
    suffix=""
    base="$input"
  fi
  
  # Get the project path
  local project_path=$(_get_project_path "$base")
  
  if [[ -z "$project_path" ]]; then
    echo "Unknown project: $base"
    echo "Available projects: "
    
    # Get a sorted list of display names (preferring short names)
    declare -A display_names
    declare -a sorted_display_names
    
    for full_name in "${!full_paths[@]}"; do
      local display_name=$(_get_display_name "$full_name")
      display_names[$display_name]="$full_name"
      sorted_display_names+=("$display_name")
    done
    
    # Sort the display names
    IFS=$'\n' sorted_display_names=($(sort <<<"${sorted_display_names[*]}"))
    unset IFS
    
    for display_name in "${sorted_display_names[@]}"; do
      local full_name="${display_names[$display_name]}"
      
      # Only show the full name in parentheses if using a shortcut
      if [[ "$display_name" != "$full_name" ]]; then
        echo -n "  $display_name ($full_name) "
      else
        echo -n "  $display_name "
      fi
        
      # Check server component
      if [[ $(_has_component "$display_name" "server") == "true" ]]; then
        echo -e -n "| ${COLOR_SERVER}${display_name}s${COLOR_RESET} "
      fi
      
      # Check web component
      if [[ $(_has_component "$display_name" "web") == "true" ]]; then
        echo -e -n "| ${COLOR_WEB}${display_name}w${COLOR_RESET} "
      fi
      
      echo ""
    done
    
    return
  fi
  
  # Handle different cases
  if [[ -z "$suffix" ]]; then
    # Open the main project
    echo -e "Opening project: $base ($project_path)"
    code "$project_path"
  elif [[ "$suffix" == "s" ]]; then
    # Open server component
    local server_path=$(_get_component_path "$base" "server")
    
    if [[ -n "$server_path" ]]; then
      echo -e "Opening server component: $base (${COLOR_SERVER}${project_path}/${server_path}${COLOR_RESET})"
      code "${project_path}/${server_path}"
    else
      echo "No server component found for project $base"
    fi
  elif [[ "$suffix" == "w" ]]; then
    # Open web component
    local web_path=$(_get_component_path "$base" "web")
    
    if [[ -n "$web_path" ]]; then
      echo -e "Opening web component: $base (${COLOR_WEB}${project_path}/${web_path}${COLOR_RESET})"
      code "${project_path}/${web_path}"
    else
      echo "No web component found for project $base"
    fi
  fi
}

# ============================================================================
# TAB COMPLETION
# ============================================================================

# Tab completion for the 'c' function
_c_completion() {
  local cur=${COMP_WORDS[COMP_CWORD]}
  local projects=()
  
  # Check for pattern proj[
  if [[ "$cur" =~ (.+)\[([sw]*)$ ]]; then
    # Handle auto-completion inside brackets
    local base="${BASH_REMATCH[1]}"
    local existing_variants="${BASH_REMATCH[2]}"
    local project_path=$(_get_project_path "$base")
    
    # Only proceed if base exists
    if [[ -n "$project_path" ]]; then
      local missing_variants=""
      
      # Check if 's' is missing and possible
      if [[ ! "$existing_variants" =~ s && $(_has_component "$base" "server") == "true" ]]; then
        missing_variants+="s"
      fi
      
      # Check if 'w' is missing and possible
      if [[ ! "$existing_variants" =~ w && $(_has_component "$base" "web") == "true" ]]; then
        missing_variants+="w"
      fi
      
      # Suggest completions for remaining variants
      for (( i=0; i<${#missing_variants}; i++ )); do
        local variant="${missing_variants:$i:1}"
        projects+=("${base}[${existing_variants}${variant}")
      done
      
      # Add closing bracket if some variants are selected
      if [[ -n "$existing_variants" ]]; then
        projects+=("${base}[${existing_variants}]")
      fi
    fi
  elif [[ -n "$cur" ]]; then
    # Generate list of display names (short names when available)
    declare -A display_to_full
    declare -a all_displays
    
    for full_name in "${!full_paths[@]}"; do
      local display=$(_get_display_name "$full_name")
      display_to_full[$display]="$full_name"
      all_displays+=("$display")
    done
    
    # Get matching projects for the current input
    for display in "${all_displays[@]}"; do
      # Check if this display name matches the current input
      if [[ "$display" == "$cur"* ]]; then
        local full_name="${display_to_full[$display]}"
        local variants=""
        local has_server=false
        local has_web=false
        
        # Check for server and web components
        if [[ $(_has_component "$display" "server") == "true" ]]; then
          variants+="s"
          has_server=true
        fi
        
        if [[ $(_has_component "$display" "web") == "true" ]]; then
          variants+="w"
          has_web=true
        fi
        
        # Only add base project if it's a partial match (not exact match)
        if [[ "$display" != "$cur" ]]; then
          projects+=("$display")
        fi
        
        # Add individual component completions if they exist
        if [[ "$has_server" == "true" ]]; then
          projects+=("${display}s")
        fi
        
        if [[ "$has_web" == "true" ]]; then
          projects+=("${display}w")
        fi
      fi
        
      # Also add individual component completions that match directly
      if [[ "${display}s" == "$cur"* && $(_has_component "$display" "server") == "true" ]]; then
        projects+=("${display}s")
      fi
        
      if [[ "${display}w" == "$cur"* && $(_has_component "$display" "web") == "true" ]]; then
        projects+=("${display}w")
      fi
    done
  else
    # No input - show all display names with bracket notation for variants
    declare -a all_displays
    
    for full_name in "${!full_paths[@]}"; do
      local display=$(_get_display_name "$full_name")
      all_displays+=("$display")
    done
    
    # Process each display name
    for display in "${all_displays[@]}"; do
      local variants=""
      
      # Check for server component
      if [[ $(_has_component "$display" "server") == "true" ]]; then
        variants+="s"
      fi
        
      # Check for web component
      if [[ $(_has_component "$display" "web") == "true" ]]; then
        variants+="w"
      fi
      
      # If we have variants, show the compact format with brackets
      if [[ -n "$variants" ]]; then
        projects+=("$display[$variants]")
      else
        projects+=("$display")
      fi
    done
  fi
  
  COMPREPLY=($(compgen -W "${projects[*]}" -- "$cur"))
  
  # Handle bracket completions (for manually typed bracket notation)
  if [[ ${#COMPREPLY[@]} -eq 1 && ${COMPREPLY[0]} =~ \[([sw]+)$ ]]; then
    # Add the closing bracket
    COMPREPLY[0]="${COMPREPLY[0]}]"
  fi
  
  return 0
}

complete -F _c_completion c