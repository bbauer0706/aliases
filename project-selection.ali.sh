#!/bin/bash
##############################################################################
#                         AUTO PROJECT SELECTION                             #
##############################################################################

show_help() {
    echo "Usage: project_env [OPTIONS]"
    echo "Sets up environment variables for project development."
    echo
    echo "Options:"
    echo "  -e ENV      Set environment profile (dev, prod, etc). Default: dev"
    echo "  -s FLAG     Enable/disable HTTPS (true/false). Default: false"
    echo "  -p PORT     Starting port number to check availability. Default: 3000"
    echo "  -i FLAG     Enable/disable GraphQL introspection (true/false). Default: true"
    echo "  -t MODE     Set transfer mode (plain, compressed, etc). Default: plain"
    echo "  -n          No port offset - use same port for WEB and GQL services"
    echo "  -h, --help  Display this help message and exit"
    echo
    echo "Examples:"
    echo "  project_env -e prod -s true -p 4000 -t compressed"
    echo "  project_env -e dev -p 3000 -n"
}

# Function to check if a port is available
is_port_available() {
    local port=$1
    if command -v lsof >/dev/null 2>&1; then
        lsof -i:"$port" >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            return 1  # Port is in use
        else
            return 0  # Port is available
        fi
    elif command -v ss >/dev/null 2>&1; then
        ss -tuln | grep ":$port\s" >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            return 1  # Port is in use
        else
            return 0  # Port is available
        fi
    else
        # If neither lsof nor ss is available, assume port is available
        return 0
    fi
}

# Function to get project name from current directory
get_project_name() {
    local current_dir=$(pwd)
    local workspace_dir="$HOME/workspaces"
    
    # Check if current directory is under the workspace directory
    if [[ "$current_dir" == "$workspace_dir"/* ]]; then
        # Extract the part of the path after workspaces/
        local remaining_path="${current_dir#$workspace_dir/}"
        # Extract the first directory name (the project name)
        local project_name="${remaining_path%%/*}"
        echo "$project_name" 
        return 0
    else
        # Not in workspace dir, use current directory name
        local current_dir_name=$(basename "$current_dir")
        echo "$current_dir_name"
        return 0
    fi
}

# Function to get port offset for a project
get_project_offset() {
    local project_name="$1"
    
    # Use the project name's hash to generate an offset
    # This ensures the same project always gets the same offset
    local hash_val=$(echo "$project_name" | cksum | cut -d ' ' -f 1)
    # Expanded offset range between 100 and 990 with increments of 10
    local offset=$(( 100 + ($hash_val % 90) * 10 ))
    
    echo "$offset"
}

# Function to display current environment variables
show_env_vars() {
    echo "Current Project Environment Variables:"
    echo "------------------------------------"
    
    # Project info
    echo "PROJECT_NAME: ${PROJECT_NAME:-Not set}"
    echo "PROFILE: ${PROFILE:-Not set}"
    
    # Host and ports
    echo "GQLHOST: ${GQLHOST:-Not set}"
    echo "WEBPORT: ${WEBPORT:-Not set}"
    echo "GQLPORT: ${GQLPORT:-Not set}"
    echo "SBPORT: ${SBPORT:-Not set}"
    echo "NDEBUGPORT: ${NDEBUGPORT:-Not set}"
    
    # GraphQL settings
    echo "GQLNUMBEROFMAXRETRIES: ${GQLNUMBEROFMAXRETRIES:-Not set}"
    echo "GQLSERVERPATH: ${GQLSERVERPATH:-Not set}"
    echo "GQLHTTPS: ${GQLHTTPS:-Not set}"
    echo "GQLINTROSPECTION: ${GQLINTROSPECTION:-Not set}"
    echo "GQLTRANSFERMODE: ${GQLTRANSFERMODE:-Not set}"
    
    echo "------------------------------------"
}

# Function to set up project environment variables
project_env() {
    local profile="dev"
    local use_https="false"
    local starting_port=3000
    local introspection="true"
    local transfer_mode="plain"
    local project_specific_offset=0
    local no_port_offset=false
    
    # Check if running from a project directory
    local project_name=$(get_project_name)
    if [ -n "$project_name" ]; then
        # We're in a project directory, get the specific offset
        project_specific_offset=$(get_project_offset "$project_name")
        export PROJECT_NAME="$project_name"
    fi
    
    # Check for help flag first
    for arg in "$@"; do
        if [ "$arg" == "--help" ]; then
            show_help
            return 0
        fi
    done
    
    # Parse command line arguments
    OPTIND=1
    while getopts ":e:s:p:i:t:nh" opt; do
        case $opt in
            e) profile="$OPTARG" ;;
            s) use_https="$OPTARG" ;; 
            p) starting_port="$OPTARG" ;;
            i) introspection="$OPTARG" ;;
            t) transfer_mode="$OPTARG" ;;
            n) no_port_offset=true ;;
            h) show_help; return 0 ;;  
            \?) echo "Invalid option: -$OPTARG" >&2
                show_help
                return 1 ;;
            :) echo "Option -$OPTARG requires an argument." >&2
                show_help
                return 1 ;;
        esac
    done
    
    export PROFILE=$profile
    
    # Find available port starting from specified port with project offset
    local port=$((starting_port + project_specific_offset))
    while ! is_port_available $port; do
        ((port++))
    done

    # Check hostname for setting GQLHOST
    CURRENT_HOSTNAME=$(hostname)
    export GQLHOST=$CURRENT_HOSTNAME
    
    export WEBPORT=$port
    if [ "$no_port_offset" = true ]; then
        export GQLPORT=$port 
    else
        export GQLPORT=$(($port+1)) 
    fi
    export SBPORT=$(($port+2))
    export NDEBUGPORT=$(($port+3))
    export GQLNUMBEROFMAXRETRIES=3
    export GQLSERVERPATH="/graphql"
    export GQLHTTPS=$use_https
    export GQLINTROSPECTION=$introspection
    export GQLTRANSFERMODE=$transfer_mode
}

# Function to automatically update project environment when changing directories
auto_update_project_env() {
    local current_dir=$(pwd)
    local workspace_dir="$HOME/workspaces"
    
    # Only auto-update if we're in a workspace directory
    if [[ "$current_dir" == "$workspace_dir"/* ]]; then
        local new_project_name=$(get_project_name)
        
        # Only update if the project name has changed
        if [ "$new_project_name" != "$PROJECT_NAME" ]; then
            echo "Auto-updating environment for project: $new_project_name"
            project_env -p ${WEBPORT:-8000}
        fi
    fi
}

# Enhanced cd function that updates project environment
pcd() {
    if [ $# -eq 0 ]; then
        # No arguments, go to home
        cd
    else
        # Change to specified directory
        cd "$@"
    fi
    
    # Auto-update project environment if successful
    if [ $? -eq 0 ]; then
        auto_update_project_env
    fi
}

# Alias for the enhanced cd
alias cd='pcd'

# Function to manually refresh project environment in current directory
refresh_project_env() {
    echo "Refreshing project environment for current directory..."
    local current_dir=$(pwd)
    echo "Current directory: $current_dir"
    
    # Clear PROJECT_NAME to force refresh
    unset PROJECT_NAME
    
    # Run project_env with current port or default
    project_env -p ${WEBPORT:-8000}
    
    echo "Environment refreshed!"
    show_env_vars
}

# Convenient aliases for project environment management
alias fix_env='refresh_project_env'
alias fix_project='refresh_project_env'  
alias project_fix='refresh_project_env'
