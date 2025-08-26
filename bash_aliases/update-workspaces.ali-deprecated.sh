#!/bin/bash
##############################################################################
#                                                                            #
#                           ⚠️  DEPRECATED FILE  ⚠️                          #
#                                                                            #
#  This bash-based system has been replaced by the C++ implementation.      #
#  Use 'aliases-cli update' or the 'uw' alias instead.                      #
#                                                                            #
#  Performance: C++ version is 10x faster with better parallelization       #
#  Features: Better error handling, JSON config, progress reporting          #
#                                                                            #
#  This file is kept for historical reference only.                         #
#                                                                            #
##############################################################################

##############################################################################
#                                                                            #
#                          WORKSPACE UPDATE UTILITY                         #
#                                                                            #
##############################################################################

# ============================================================================
# CONFIGURATION
# ============================================================================

# Maximum number of parallel processes
MAX_PARALLEL_JOBS=4

# Color codes for output
declare -g COLOR_RESET="\033[0m"
declare -g COLOR_SUCCESS="\033[1;32m"  # Bright green
declare -g COLOR_ERROR="\033[1;31m"    # Bright red
declare -g COLOR_WARNING="\033[1;33m"  # Bright yellow
declare -g COLOR_INFO="\033[1;34m"     # Bright blue
declare -g COLOR_SKIPPED="\033[1;35m"  # Bright magenta

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

_load_project_mappings() {
  local workspace_dir="$HOME/workspaces"
  
  # Discover all projects in the workspace directory
  declare -gA full_paths=()
  for dir in "$workspace_dir"/*; do
    if [ -d "$dir" ]; then
      local full_name=$(basename "$dir")
      full_paths[$full_name]="$dir"
    fi
  done
  
  # Initialize empty mapping arrays
  declare -gA full_to_short=()
  declare -gA server_paths=()
  declare -gA web_paths=()
  
  # Default paths for server and web components
  declare -ga default_server_paths=("java/serverJava" "serverJava")
  declare -ga default_web_paths=("webapp" "webApp" "web")
  
  # Load local mappings if they exist
  local script_dir="$(dirname "${BASH_SOURCE[0]}")"
  local local_mappings_file="$script_dir/mappings.local.sh"
  
  if [[ -f "$local_mappings_file" ]]; then    
    # Initialize empty local mapping arrays
    declare -gA local_full_to_short=()
    declare -gA local_server_paths=()
    declare -gA local_web_paths=()
    
    # Source the local mappings file
    source "$local_mappings_file"
    
    # Merge local mappings with default mappings
    for key in "${!local_full_to_short[@]}"; do
      full_to_short[$key]="${local_full_to_short[$key]}"
    done
    
    for key in "${!local_server_paths[@]}"; do
      server_paths[$key]="${local_server_paths[$key]}"
    done
    
    for key in "${!local_web_paths[@]}"; do
      web_paths[$key]="${local_web_paths[$key]}"
    done
  fi
}

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

_get_project_path() {
  local name="$1"
  local full_name=$(_get_full_project_name "$name")
  if [[ -n "$full_name" ]]; then
    echo "${full_paths[$full_name]}"
  else
    echo ""
  fi
}

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

_log() {
  local level="$1"
  local project="$2"
  local message="$3"
  local timestamp=$(date '+%H:%M:%S')
  
  case "$level" in
    "SUCCESS")
      echo -e "[$timestamp] ${COLOR_SUCCESS}✓${COLOR_RESET} $project: $message"
      ;;
    "ERROR")
      echo -e "[$timestamp] ${COLOR_ERROR}✗${COLOR_RESET} $project: $message"
      ;;
    "WARNING")
      echo -e "[$timestamp] ${COLOR_WARNING}⚠${COLOR_RESET} $project: $message"
      ;;
    "INFO")
      echo -e "[$timestamp] ${COLOR_INFO}ℹ${COLOR_RESET} $project: $message"
      ;;
    "SKIPPED")
      echo -e "[$timestamp] ${COLOR_SKIPPED}⊘${COLOR_RESET} $project: $message"
      ;;
  esac
}

_has_working_changes() {
  local project_path="$1"
  
  # Check if it's a git repository
  if [[ ! -d "$project_path/.git" ]]; then
    return 1  # Not a git repo, consider as no changes
  fi
  
  cd "$project_path" || return 1
  
  # Check for unstaged changes, staged changes, or untracked files
  if [[ -n "$(git status --porcelain)" ]]; then
    return 0  # Has working changes
  else
    return 1  # No working changes
  fi
}

_get_current_branch() {
  local project_path="$1"
  cd "$project_path" || return 1
  git rev-parse --abbrev-ref HEAD 2>/dev/null
}

_update_project_component() {
  local project_name="$1"
  local component_type="$2"  # "main", "server", or "web"
  local component_path="$3"
  
  local project_path=$(_get_project_path "$project_name")
  local full_path="$project_path"
  
  if [[ "$component_type" != "main" ]]; then
    full_path="$project_path/$component_path"
    project_name="${project_name}-${component_type}"
  fi
  
  # Check if directory exists
  if [[ ! -d "$full_path" ]]; then
    _log "ERROR" "$project_name" "Directory does not exist: $full_path"
    return 1
  fi
  
  # Check if it's a git repository
  if [[ ! -d "$full_path/.git" ]]; then
    _log "SKIPPED" "$project_name" "Not a git repository"
    return 0
  fi
  
  # Check for working changes
  if _has_working_changes "$full_path"; then
    _log "SKIPPED" "$project_name" "Has uncommitted changes"
    return 0
  fi
  
  cd "$full_path" || {
    _log "ERROR" "$project_name" "Cannot change to directory"
    return 1
  }
  
  # Get current branch
  local original_branch=$(_get_current_branch "$full_path")
  if [[ -z "$original_branch" ]]; then
    _log "ERROR" "$project_name" "Cannot determine current branch"
    return 1
  fi
  
  _log "INFO" "$project_name" "Starting update (current branch: $original_branch)"
  
  # Switch to main if not already on main
  local switched_branch=false
  if [[ "$original_branch" != "main" && "$original_branch" != "master" ]]; then
    _log "INFO" "$project_name" "Switching to main branch"
    if git checkout main 2>/dev/null || git checkout master 2>/dev/null; then
      switched_branch=true
    else
      _log "ERROR" "$project_name" "Failed to switch to main/master branch"
      return 1
    fi
  fi
  
  # Pull latest changes
  _log "INFO" "$project_name" "Pulling latest changes"
  if ! git pull --ff-only; then
    _log "ERROR" "$project_name" "Failed to pull changes"
    # Try to switch back to original branch if we switched
    if [[ "$switched_branch" == "true" ]]; then
      git checkout "$original_branch" 2>/dev/null
    fi
    return 1
  fi
  
  # Update packages based on component type
  local update_success=true
  if [[ "$component_type" == "server" || ( "$component_type" == "main" && -f "pom.xml" ) ]]; then
    _log "INFO" "$project_name" "Updating Maven dependencies"
    if ! mvn dependency:resolve dependency:resolve-sources -q; then
      _log "WARNING" "$project_name" "Maven dependency update failed"
      update_success=false
    fi
  elif [[ "$component_type" == "web" || ( "$component_type" == "main" && -f "package.json" ) ]]; then
    _log "INFO" "$project_name" "Running npm install"
    if ! npm install --silent; then
      _log "WARNING" "$project_name" "npm install failed"
      update_success=false
    fi
  elif [[ "$component_type" == "main" ]]; then
    # Check for both Maven and npm in main project
    if [[ -f "pom.xml" ]]; then
      _log "INFO" "$project_name" "Updating Maven dependencies"
      if ! mvn dependency:resolve dependency:resolve-sources -q; then
        _log "WARNING" "$project_name" "Maven dependency update failed"
        update_success=false
      fi
    fi
    if [[ -f "package.json" ]]; then
      _log "INFO" "$project_name" "Running npm install"
      if ! npm install --silent; then
        _log "WARNING" "$project_name" "npm install failed"
        update_success=false
      fi
    fi
  fi
  
  # Switch back to original branch if we switched
  if [[ "$switched_branch" == "true" ]]; then
    _log "INFO" "$project_name" "Switching back to $original_branch"
    if ! git checkout "$original_branch"; then
      _log "ERROR" "$project_name" "Failed to switch back to $original_branch"
      return 1
    fi
  fi
  
  if [[ "$update_success" == "true" ]]; then
    _log "SUCCESS" "$project_name" "Update completed successfully"
  else
    _log "WARNING" "$project_name" "Update completed with warnings"
  fi
  
  return 0
}

# ============================================================================
# MAIN FUNCTION (renamed to avoid conflict with C++ version)
# ============================================================================

uw_bash() {
  # Help parameters
  if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    echo "Workspace update utility:"
    echo "  uw                    - Update all projects"
    echo "  uw <project>          - Update specific project"
    echo "  uw <project>s         - Update server component only"
    echo "  uw <project>w         - Update web component only"
    echo "  uw <proj1> <proj2>... - Update multiple specific projects"
    echo "  uw -j <num>           - Set max parallel jobs (default: $MAX_PARALLEL_JOBS)"
    echo ""
    echo "The script will:"
    echo "  1. Skip projects with uncommitted changes"
    echo "  2. Switch to main branch (if not already on main)"
    echo "  3. Pull latest changes"
    echo "  4. Update packages (Maven for server, npm for web)"
    echo "  5. Switch back to original branch"
    return
  fi
  
  # Load project mappings
  _load_project_mappings
  
  # Parse arguments
  local projects_to_update=()
  local parallel_jobs="$MAX_PARALLEL_JOBS"
  
  while [[ $# -gt 0 ]]; do
    case "$1" in
      -j|--jobs)
        parallel_jobs="$2"
        shift 2
        ;;
      *)
        projects_to_update+=("$1")
        shift
        ;;
    esac
  done
  
  # If no projects specified, update all
  if [[ ${#projects_to_update[@]} -eq 0 ]]; then
    for full_name in "${!full_paths[@]}"; do
      projects_to_update+=("$full_name")
    done
  fi
  
  echo "Starting workspace update with $parallel_jobs parallel jobs..."
  echo "Projects to process: ${#projects_to_update[@]}"
  echo "=============================================="
  
  # Create a temporary directory for job management
  local temp_dir=$(mktemp -d)
  local job_count=0
  
  # Function to wait for a job slot to become available
  wait_for_slot() {
    while [[ $(jobs -r | wc -l) -ge $parallel_jobs ]]; do
      sleep 0.1
    done
  }
  
  # Process each project
  for project in "${projects_to_update[@]}"; do
    # Parse project specification (project, projects, projectw)
    local base_project="$project"
    local suffix=""
    
    # Check for 's' or 'w' suffix
    if [[ "$project" =~ (.+)([sw])$ ]]; then
      base_project="${BASH_REMATCH[1]}"
      suffix="${BASH_REMATCH[2]}"
    fi
    
    # Validate project exists
    local project_path=$(_get_project_path "$base_project")
    if [[ -z "$project_path" ]]; then
      _log "ERROR" "$project" "Unknown project"
      continue
    fi
    
    # Determine what to update
    if [[ -z "$suffix" ]]; then
      # Update main project and components
      wait_for_slot
      {
        _update_project_component "$base_project" "main" ""
        
        # Also update server component if it exists
        local server_path=$(_get_component_path "$base_project" "server")
        if [[ -n "$server_path" ]]; then
          _update_project_component "$base_project" "server" "$server_path"
        fi
        
        # Also update web component if it exists
        local web_path=$(_get_component_path "$base_project" "web")
        if [[ -n "$web_path" ]]; then
          _update_project_component "$base_project" "web" "$web_path"
        fi
      } &
    elif [[ "$suffix" == "s" ]]; then
      # Update server component only
      local server_path=$(_get_component_path "$base_project" "server")
      if [[ -n "$server_path" ]]; then
        wait_for_slot
        _update_project_component "$base_project" "server" "$server_path" &
      else
        _log "ERROR" "$project" "No server component found"
      fi
    elif [[ "$suffix" == "w" ]]; then
      # Update web component only
      local web_path=$(_get_component_path "$base_project" "web")
      if [[ -n "$web_path" ]]; then
        wait_for_slot
        _update_project_component "$base_project" "web" "$web_path" &
      else
        _log "ERROR" "$project" "No web component found"
      fi
    fi
    
    ((job_count++))
  done
  
  # Wait for all background jobs to complete
  wait
  
  # Cleanup
  rm -rf "$temp_dir"
  
  echo "=============================================="
  echo "Workspace update completed!"
}

# ============================================================================
# TAB COMPLETION
# ============================================================================

_uw_completion() {
  local cur=${COMP_WORDS[COMP_CWORD]}
  local prev=${COMP_WORDS[COMP_CWORD-1]}
  
  # Handle -j flag
  if [[ "$prev" == "-j" || "$prev" == "--jobs" ]]; then
    COMPREPLY=($(compgen -W "1 2 3 4 5 6 8 10" -- "$cur"))
    return 0
  fi
  
  # Load mappings for completion
  _load_project_mappings
  
  local projects=()
  
  # Add flags
  if [[ "$cur" == -* ]]; then
    projects+=("-j" "--jobs" "-h" "--help")
  else
    # Generate project completions similar to 'c' command
    declare -A display_to_full
    declare -a all_displays
    
    for full_name in "${!full_paths[@]}"; do
      local display="$full_name"
      if [[ -n "${full_to_short[$full_name]}" ]]; then
        display="${full_to_short[$full_name]}"
      fi
      display_to_full[$display]="$full_name"
      all_displays+=("$display")
    done
    
    # Get matching projects for the current input
    for display in "${all_displays[@]}"; do
      if [[ "$display" == "$cur"* ]]; then
        projects+=("$display")
        
        # Add component-specific completions
        local server_path=$(_get_component_path "$display" "server")
        if [[ -n "$server_path" ]]; then
          projects+=("${display}s")
        fi
        
        local web_path=$(_get_component_path "$display" "web")
        if [[ -n "$web_path" ]]; then
          projects+=("${display}w")
        fi
      fi
    done
  fi
  
  COMPREPLY=($(compgen -W "${projects[*]}" -- "$cur"))
  return 0
}

complete -F _uw_completion uw
