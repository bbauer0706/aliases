#!/bin/bash
##############################################################################
#                      SETUP SCRIPT FOR C++ ALIASES-CLI                     #
##############################################################################

# Parse command line arguments
FORCE_MODE=false
while [[ $# -gt 0 ]]; do
    case $1 in
        -f|--force)
            FORCE_MODE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [-f|--force] [-h|--help]"
            echo ""
            echo "Options:"
            echo "  -f, --force    Force installation by automatically:"
            echo "                 - Killing running aliases-cli processes"
            echo "                 - Auto-accepting .bash_aliases updates"
            echo "                 - Overriding permission checks"
            echo "  -h, --help     Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
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

# Function to kill running aliases-cli processes in force mode
kill_running_processes() {
    local processes=$(pgrep -f "aliases-cli" 2>/dev/null || true)
    if [[ -n "$processes" ]]; then
        if [[ "$FORCE_MODE" == "true" ]]; then
            status "Force mode: Killing running aliases-cli processes..."
            pkill -f "aliases-cli" 2>/dev/null || true
            sleep 1
            # Double-check and force kill if needed
            local remaining=$(pgrep -f "aliases-cli" 2>/dev/null || true)
            if [[ -n "$remaining" ]]; then
                warning "Some processes still running, force killing..."
                pkill -9 -f "aliases-cli" 2>/dev/null || true
                sleep 1
            fi
            success "Terminated running aliases-cli processes"
            return 0
        else
            warning "Found running aliases-cli processes (PIDs: $processes)"
            warning "Please close running instances or use --force flag"
            return 1
        fi
    fi
    return 0
}

# Get script directory - detect the actual location of the aliases repository
SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
ALIASES_DIR=$SCRIPT_DIR

status "Detected aliases repository at: $ALIASES_DIR"
if [[ "$FORCE_MODE" == "true" ]]; then
    status "Running in FORCE MODE - will auto-accept prompts and handle conflicts"
fi

# 1. Try to build the C++ version if build.sh exists
if [[ -f "$ALIASES_DIR/build.sh" ]]; then
    status "Found build.sh, attempting to build C++ version..."
    
    # Check if we have a C++ compiler
    if command -v g++ >/dev/null 2>&1; then
        cd "$ALIASES_DIR"
        
        # Try to build
        if ./build.sh >/dev/null 2>&1; then
            success "Successfully built new C++ version"
            
            # Update the distributed binary if build succeeded
            if [[ -f "$ALIASES_DIR/build/aliases-cli" ]]; then
                if ! cp "$ALIASES_DIR/build/aliases-cli" "$ALIASES_DIR/aliases-cli" 2>/dev/null; then
                    status "Binary copy failed, checking for running processes..."
                    if kill_running_processes; then
                        status "Retrying binary copy after process termination..."
                        if cp "$ALIASES_DIR/build/aliases-cli" "$ALIASES_DIR/aliases-cli" 2>/dev/null; then
                            success "Updated distributed binary with new build"
                        else
                            error "Failed to update distributed binary even after terminating processes - check file permissions"
                        fi
                    else
                        error "Failed to update distributed binary - aliases-cli is currently in use. Please close all running instances or use --force flag."
                    fi
                else
                    success "Updated distributed binary with new build"
                fi
            fi
        else
            warning "Build failed, using existing binary"
        fi
    else
        warning "No C++ compiler found (g++), using existing binary"
    fi
else
    status "No build.sh found, using existing binary"
fi

# 2. Check if binary exists and is working
BINARY_PATH=""
if [[ -f "$ALIASES_DIR/build/aliases-cli" ]]; then
    BINARY_PATH="$ALIASES_DIR/build/aliases-cli"
    status "Using built binary: build/aliases-cli"
elif [[ -f "$ALIASES_DIR/aliases-cli" ]]; then
    BINARY_PATH="$ALIASES_DIR/aliases-cli"
    status "Using distributed binary: aliases-cli"
else
    error "No aliases-cli binary found! Expected: build/aliases-cli or aliases-cli"
fi

# Test binary functionality
status "Testing binary functionality..."
if ! "$BINARY_PATH" --version > /dev/null 2>&1; then
    error "Binary test failed! The aliases-cli binary is not working."
fi
success "Binary test passed"

# 3. Update any existing local installations with the new binary
LOCAL_BIN="$HOME/.local/bin"
if [[ -f "$LOCAL_BIN/aliases-cli" ]]; then
    status "Found existing local installation, updating with new binary..."
    if ! mkdir -p "$LOCAL_BIN"; then
        error "Failed to create directory $LOCAL_BIN"
    fi
    if ! cp "$BINARY_PATH" "$LOCAL_BIN/aliases-cli" 2>/dev/null; then
        status "Local binary copy failed, checking for running processes..."
        if kill_running_processes; then
            status "Retrying local binary copy after process termination..."
            if ! cp "$BINARY_PATH" "$LOCAL_BIN/aliases-cli"; then
                error "Failed to copy binary to $LOCAL_BIN/aliases-cli even after terminating processes - check permissions"
            fi
        else
            error "Failed to copy binary to $LOCAL_BIN/aliases-cli - file is in use. Please close all running instances or use --force flag."
        fi
    fi
    if ! chmod +x "$LOCAL_BIN/aliases-cli"; then
        error "Failed to make $LOCAL_BIN/aliases-cli executable - check permissions"
    fi
    success "Updated local binary at $LOCAL_BIN/aliases-cli"
fi

# Create the template for .bash_aliases with C++ aliases-cli integration
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

##############################################################################
#                        C++ ALIASES-CLI INTEGRATION                        #
##############################################################################

# Fast C++ workspace management aliases
alias aliases-cli='\$ALIASES_DIR/aliases-cli'
alias c='\$ALIASES_DIR/aliases-cli code'
alias uw='\$ALIASES_DIR/aliases-cli update'

##############################################################################

##############################################################################

# Source utility bash files (only .ali.sh files, excluding deprecated .ali-deprecated.sh)
if [ -d "\$ALIASES_DIR/bash_aliases" ]; then
  for alias_file in "\$ALIASES_DIR/bash_aliases"/*.ali.sh; do
    if [ -f "\$alias_file" ]; then
      source "\$alias_file"
    fi
  done
fi

# Mark that C++ aliases are active (no legacy navigation/update sourcing)
export USE_CPP_ALIASES=true

# Load bash completion for aliases-cli
if [ -f "\$ALIASES_DIR/bash_completion/aliases-completion.sh" ]; then
  source "\$ALIASES_DIR/bash_completion/aliases-completion.sh"
fi

# Load bash integration for project environment
if [ -f "\$ALIASES_DIR/bash_integration/project-env.sh" ]; then
  source "\$ALIASES_DIR/bash_integration/project-env.sh"
fi

# Run auto-setup for new terminals
if type auto_setup_new_terminal >/dev/null 2>&1; then
  auto_setup_new_terminal
fi
EOF
)

# Check if .bash_aliases exists
if [ ! -f "$HOME/.bash_aliases" ]; then
  status "Creating .bash_aliases file"
  
  # Create the .bash_aliases file with the template
  if ! echo "$BASH_ALIASES_TEMPLATE" > "$HOME/.bash_aliases"; then
    error "Failed to create .bash_aliases file - check permissions for $HOME"
  fi

  success "Created .bash_aliases file with C++ aliases-cli integration"
else
  status ".bash_aliases already exists, checking if update is needed"
  
  # Define the expected variable definition to check for
  EXPECTED_DIR_VAR="ALIASES_DIR=\"$ALIASES_DIR\""
  
  # Check if the correct variable definition is in .bash_aliases
  if grep -q "$EXPECTED_DIR_VAR" "$HOME/.bash_aliases"; then
    # Check if C++ integration exists
    if grep -q "# C++ ALIASES-CLI INTEGRATION" "$HOME/.bash_aliases"; then
      success ".bash_aliases already contains C++ aliases-cli integration"
    else
      warning ".bash_aliases needs C++ integration added"
      
      # Ask user if they want to update the file (auto-accept in force mode)
      if [[ "$FORCE_MODE" == "true" ]]; then
        status "Force mode: Auto-accepting .bash_aliases update with C++ integration"
        REPLY="Y"
      else
        read -p "Do you want to update your .bash_aliases with C++ aliases-cli integration? (y/n): " -n 1 -r
        echo
      fi
      if [[ $REPLY =~ ^[Yy]$ ]]; then
        # Backup the existing file
        if ! cp "$HOME/.bash_aliases" "$HOME/.bash_aliases.bak"; then
          error "Failed to create backup of .bash_aliases - check permissions"
        fi
        
        # Replace the file
        if ! echo "$BASH_ALIASES_TEMPLATE" > "$HOME/.bash_aliases"; then
          error "Failed to update .bash_aliases - check permissions. Backup is at .bash_aliases.bak"
        fi
        
        success "Updated .bash_aliases with C++ integration and created backup at .bash_aliases.bak"
      else
        warning "User chose not to update .bash_aliases - C++ features may not be available"
      fi
    fi
  else
    warning ".bash_aliases needs to be updated with the correct repository path"
    
    # Ask user if they want to update the file (auto-accept in force mode)
    if [[ "$FORCE_MODE" == "true" ]]; then
      status "Force mode: Auto-accepting .bash_aliases update with correct repository path"
      REPLY="Y"
    else
      read -p "Do you want to update your .bash_aliases to use the current repository path? (y/n): " -n 1 -r
      echo
    fi
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      # Backup the existing file
      if ! cp "$HOME/.bash_aliases" "$HOME/.bash_aliases.bak"; then
        error "Failed to create backup of .bash_aliases - check permissions"
      fi
      
      # Replace the file
      if ! echo "$BASH_ALIASES_TEMPLATE" > "$HOME/.bash_aliases"; then
        error "Failed to update .bash_aliases - check permissions. Backup is at .bash_aliases.bak"
      fi
      
      success "Updated .bash_aliases with correct path and C++ integration, created backup at .bash_aliases.bak"
    else
      warning "User chose not to update .bash_aliases - aliases may not work correctly"
    fi
  fi
fi

# Check if .bash_aliases is sourced from .bashrc
if ! grep -q "source ~/.bash_aliases" "$HOME/.bashrc" && ! grep -q ". ~/.bash_aliases" "$HOME/.bashrc"; then
  status "Adding source command to .bashrc"
  {
    echo ""
    echo "# Source bash aliases"
    echo "if [ -f ~/.bash_aliases ]; then"
    echo "    source ~/.bash_aliases"
    echo "fi"
  } >> "$HOME/.bashrc" || error "Failed to update .bashrc - check permissions for $HOME/.bashrc"
  success "Added source command to .bashrc"
else
  success ".bash_aliases is already sourced from .bashrc"
fi

# Check for and remove duplicate C++ integration from .bashrc
if grep -q "# C++ ALIASES-CLI INTEGRATION" "$HOME/.bashrc"; then
  status "Found duplicate C++ integration in .bashrc, removing..."
  
  # Create a temporary file without the C++ integration section
  awk '
    /^##############################################################################$/ && getline line && line ~ /^#.*C\+\+ ALIASES-CLI INTEGRATION.*#$/ {
      # Found start of C++ section, skip until we find the closing line
      while (getline > 0) {
        if ($0 ~ /^##############################################################################$/) {
          break
        }
      }
      next
    }
    { print }
  ' "$HOME/.bashrc" > "$HOME/.bashrc.tmp" && mv "$HOME/.bashrc.tmp" "$HOME/.bashrc"
  
  success "Removed duplicate C++ integration from .bashrc"
fi

success "Setup complete! Your bash aliases are now configured with C++ aliases-cli integration."

# Log force mode summary if used
if [[ "$FORCE_MODE" == "true" ]]; then
    echo ""
    echo -e "${BLUE}Force Mode Summary:${NC}"
    if pgrep -f "aliases-cli" >/dev/null 2>&1; then
        echo -e "  ${GREEN}✓${NC} Terminated running aliases-cli processes"
    fi
    if [[ -f "$HOME/.bash_aliases.bak" ]]; then
        echo -e "  ${GREEN}✓${NC} Auto-accepted .bash_aliases updates (backup created)"
    fi
    echo -e "  ${GREEN}✓${NC} Completed installation without user prompts"
fi

echo ""
echo -e "${YELLOW}Note:${NC} Changes will take effect in new terminal sessions or after running 'source ~/.bash_aliases'"
echo -e "      Aliases are sourced from: ${GREEN}$ALIASES_DIR${NC}"
echo -e "      Binary location: ${GREEN}$BINARY_PATH${NC}"

# Show build status if build was attempted
if [[ -f "$ALIASES_DIR/build.sh" ]] && command -v g++ >/dev/null 2>&1; then
    if [[ -f "$ALIASES_DIR/build/aliases-cli" ]]; then
        echo -e "      Build status: ${GREEN}Successfully built from source${NC}"
    else
        echo -e "      Build status: ${YELLOW}Using distributed binary (build failed)${NC}"
    fi
elif [[ -f "$ALIASES_DIR/build.sh" ]]; then
    echo -e "      Build status: ${YELLOW}Using distributed binary (no compiler)${NC}"
fi

# Check for configuration
if [[ ! -f "$ALIASES_DIR/mappings.json" ]]; then
    echo ""
    echo -e "${YELLOW}Configuration:${NC}"
    echo -e "  ${BLUE}First-time setup:${NC} cp mappings.template.json mappings.json"
    echo -e "  ${BLUE}Then edit:${NC} code mappings.json  # Add your project shortcuts"
    echo -e "  ${BLUE}Note:${NC} mappings.json is git-ignored (safe for local config)"
fi

echo ""
echo -e "${YELLOW}File Structure:${NC}"
echo -e "  • ${GREEN}*.ali.sh${NC} files in bash_aliases/ are automatically sourced"
echo -e "  • ${RED}*.ali-deprecated.sh${NC} files are ignored (replaced by C++)"
echo -e "  • Add new bash utilities with ${GREEN}.ali.sh${NC} extension for auto-sourcing"

echo ""
echo -e "${BLUE}Available Commands:${NC}"
echo -e "  • ${GREEN}c <project>${NC}    - Navigate to project (50x faster than bash)"
echo -e "  • ${GREEN}uw${NC}             - Update all workspaces"
echo -e "  • ${GREEN}project_env${NC}    - Setup project environment"
echo -e "  • ${GREEN}fix_env${NC}        - Refresh environment (legacy compatibility)"
echo -e "  • Plus bash utilities: basic, maven, npm shortcuts"
