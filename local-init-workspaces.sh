#!/bin/bash
#
# Workspace Initialization Script
# This script creates the ~/workspaces directory if needed,
# clones specified git repositories, and sets up bash aliases.

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

# Create workspaces directory if it doesn't exist
WORKSPACES_DIR="$HOME/workspaces"
if [ ! -d "$WORKSPACES_DIR" ]; then
  status "Creating workspaces directory at $WORKSPACES_DIR"
  mkdir -p "$WORKSPACES_DIR"
  success "Created workspaces directory"
else
  status "Workspaces directory already exists at $WORKSPACES_DIR"
fi

# Function to clone a repository
clone_repo() {
  local repo_url="$1"
  local target_dir="$2"
  
  if [ -d "$target_dir" ]; then
    warning "Directory already exists: $target_dir"
    read -p "Do you want to update the existing repository? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      status "Updating repository in $target_dir"
      (cd "$target_dir" && git pull) || error "Failed to update repository"
      success "Updated repository in $target_dir"
    else
      warning "Skipped repository: $target_dir"
    fi
  else
    status "Cloning repository: $repo_url"
    git clone "$repo_url" "$target_dir" || error "Failed to clone repository: $repo_url"
    success "Cloned repository to $target_dir"
  fi
}

# List of repositories to clone
status "Beginning repository setup"

# Add your repositories below by uncommenting and modifying these examples:
clone_repo "git@git.inform-software.com:gb20/road/ragnarok/urm20.git" "$WORKSPACES_DIR/urm20"
clone_repo "git@git.inform-software.com:gb20/road/ragnarok/otv20.git" "$WORKSPACES_DIR/otv20"
clone_repo "git@git.inform-software.com:gb20/road/ragnarok/dispatch20.git" "$WORKSPACES_DIR/dispatch20"
clone_repo "git@git.inform-software.com:gb20/road/ragnarok/library/ragnarokui.git" "$WORKSPACES_DIR/ragnarokui"
clone_repo "git@git.inform-software.com:gb20/road/ragnarok/library/ragnarok-java-tools.git" "$WORKSPACES_DIR/ragnarok-java-tools"
clone_repo "git@git.inform-software.com:gb20/road/ragnarok/deploy.git" "$WORKSPACES_DIR/deploy"
clone_repo "git@git.inform-software.com:gb20/road/ragnarok/containers/syncrotess-proxy.git" "$WORKSPACES_DIR/syncrotess-proxy"
clone_repo "git@git.inform-software.com:gb20/road/witos/witos-scripts.git" "$WORKSPACES_DIR/witos-scripts"



# Call setup.sh script
SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
status "Running aliases setup script"
"$SCRIPT_DIR/setup.sh" || error "Setup script failed"

success "Workspace initialization complete!"
echo ""
echo -e "${YELLOW}Note:${NC} All repositories have been cloned/updated in ${GREEN}$WORKSPACES_DIR${NC}"
echo -e "      Your bash aliases have been set up and will be available in new terminal sessions"
