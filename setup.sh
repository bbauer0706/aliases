#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Function to print status messages
status() {
  echo -e "${BLUE}[STATUS]${NC} $1"
}

success() {
  echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warning() {
  echo -e "${YELLOW}[WARNING]${NC} $1"
}

error() {
  echo -e "${RED}[ERROR]${NC} $1"
  exit 1
}

# Function to calculate starting port based on username
calculate_user_port() {
  local username="$USER"
  # Use username hash to generate a base port
  local hash_val=$(echo "$username" | cksum | cut -d ' ' -f 1)
  # Generate a port rounded to thousands between 3000 and 9000
  local port=$(( 3000 + ($hash_val % 7) * 1000 ))
  echo "$port"
}

# Calculate user-specific starting port
USER_PORT=$(calculate_user_port)
status "Calculated user-specific starting port: $USER_PORT"

# Detect the actual location of the aliases repository
SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
ALIASES_DIR=$SCRIPT_DIR

status "Detected aliases repository at: $ALIASES_DIR"

# Create the template for .bash_aliases with a variable for the directory path
# This template includes a fix to prevent PROJECT_NAME being set to username
# when the shell starts outside of a workspace directory
BASH_ALIASES_TEMPLATE=$(cat << EOF
#!/bin/bash
##############################################################################
#                                                                            #
#                         CUSTOM BASH ALIASES                                #
#                                                                            #
#  This file is automatically managed by the aliases repository.             #
#  Make changes to the individual files in the aliases repository instead.   #
#                                                                            #
##############################################################################

# Define the aliases directory location
ALIASES_DIR="$ALIASES_DIR"

# Source all alias files from the aliases directory
if [ -d "\$ALIASES_DIR" ]; then
  for alias_file in "\$ALIASES_DIR"/*.ali.sh; do
    if [ -f "\$alias_file" ]; then
      source "\$alias_file"
    fi
  done
else
  echo -e "\033[0;31m[ERROR]\033[0m Aliases directory not found: \$ALIASES_DIR"
fi

# Initialize project environment only when appropriate
if [[ "\${PROJECT_ENV_LOADED}" != "true" ]]; then
  export PROJECT_ENV_LOADED=true
  
  # Only auto-run project_env if we're in a workspace directory
  if [[ "\$(pwd)" == "\$HOME/workspaces"/* ]]; then
    project_env -p $USER_PORT
  else
    # Set default values when not in a workspace to avoid PROJECT_NAME=username issue
    export PROFILE=dev
    export GQLHOST=\$(hostname)
    export WEBPORT=$USER_PORT
    export GQLPORT=\$((\$WEBPORT + 1))
    export SBPORT=\$((\$WEBPORT + 2))
    export NDEBUGPORT=\$((\$WEBPORT + 3))
    export GQLNUMBEROFMAXRETRIES=3
    export GQLSERVERPATH="/graphql"
    export GQLHTTPS=false
    export GQLINTROSPECTION=true
    export GQLTRANSFERMODE=plain
    # Don't set PROJECT_NAME when not in workspace to avoid username confusion
    echo -e "\033[0;33m[INFO]\033[0m Project environment initialized with defaults. Use 'fix_env' in a workspace project to set PROJECT_NAME."
  fi
fi
EOF
)

# Check if .bash_aliases exists
if [ ! -f "$HOME/.bash_aliases" ]; then
  status "Creating .bash_aliases file"
  
  # Create the .bash_aliases file with the template
  echo "$BASH_ALIASES_TEMPLATE" > "$HOME/.bash_aliases"

  success "Created .bash_aliases file"
else
  status ".bash_aliases already exists, checking if update is needed"
  
  # Define the expected variable definition to check for
  EXPECTED_DIR_VAR="ALIASES_DIR=\"$ALIASES_DIR\""
  
  # Check if the correct variable definition is in .bash_aliases
  if grep -q "$EXPECTED_DIR_VAR" "$HOME/.bash_aliases"; then
    success ".bash_aliases already contains the correct repository path"
  else
    warning ".bash_aliases needs to be updated with the correct repository path"
    
    # Ask user if they want to update the file
    read -p "Do you want to update your .bash_aliases to use the current repository path? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      # Backup the existing file
      cp "$HOME/.bash_aliases" "$HOME/.bash_aliases.bak"
      
      # Replace the file
      echo "$BASH_ALIASES_TEMPLATE" > "$HOME/.bash_aliases"
      
      success "Updated .bash_aliases with the correct path and created backup at .bash_aliases.bak"
    else
      warning "User chose not to update .bash_aliases - aliases may not work correctly"
    fi
  fi
fi

# Check if .bash_aliases is sourced from .bashrc
if ! grep -q "source ~/.bash_aliases" "$HOME/.bashrc"; then
  status "Adding source command to .bashrc"
  echo "" >> "$HOME/.bashrc"
  echo "# Source bash aliases" >> "$HOME/.bashrc"
  echo "if [ -f ~/.bash_aliases ]; then" >> "$HOME/.bashrc"
  echo "    source ~/.bash_aliases" >> "$HOME/.bashrc"
  echo "fi" >> "$HOME/.bashrc"
  success "Added source command to .bashrc"
else
  success ".bash_aliases is already sourced from .bashrc"
fi

success "Setup complete! Your bash aliases are now configured."
echo ""
echo -e "${YELLOW}Note:${NC} Changes to alias files will take effect in new terminal sessions"
echo -e "      or after running 'source ~/.bash_aliases'"
echo -e "      Aliases are sourced from: ${GREEN}$ALIASES_DIR${NC}"
echo ""
echo -e "${BLUE}Project Environment Features:${NC}"
echo -e "  • Auto-detects project names when in workspace directories"
echo -e "  • Prevents PROJECT_NAME=username issue when outside workspace"
echo -e "  • Use ${GREEN}fix_env${NC} or ${GREEN}refresh_project_env${NC} to update environment in any project"
echo -e "  • Enhanced ${GREEN}cd${NC} command auto-updates environment when moving between projects"
echo -e "  • Default starting port: ${GREEN}$USER_PORT${NC} (calculated from your username)"