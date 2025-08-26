#!/bin/bash
##############################################################################
#                      SETUP SCRIPT FOR C++ ALIASES-CLI                     #
##############################################################################

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

# Get script directory - detect the actual location of the aliases repository
SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
ALIASES_DIR=$SCRIPT_DIR

status "Detected aliases repository at: $ALIASES_DIR"

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
                cp "$ALIASES_DIR/build/aliases-cli" "$ALIASES_DIR/aliases-cli"
                success "Updated distributed binary with new build"
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
    mkdir -p "$LOCAL_BIN"
    cp "$BINARY_PATH" "$LOCAL_BIN/aliases-cli"
    chmod +x "$LOCAL_BIN/aliases-cli"
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

# Add aliases-cli to PATH if not already there
if [[ ":$PATH:" != *":\$ALIASES_DIR:"* ]]; then
    export PATH="\$ALIASES_DIR:\$PATH"
fi

##############################################################################
#                        C++ ALIASES-CLI INTEGRATION                        #
##############################################################################

# Fast C++ workspace management aliases
alias c='aliases-cli code'
alias uw='aliases-cli update'

# Project environment setup (enhanced)
project_env() {
    # Run the C++ version and capture output
    local output=\$(aliases-cli env "\$@" 2>/dev/null)
    
    if [[ \$? -eq 0 && -n "\$output" ]]; then
        # Extract and eval the export statements
        echo "\$output" | grep '^export ' | while read -r line; do
            eval "\$line"
        done
        
        # Show the success message
        echo "\$output" | grep -v '^export '
    else
        echo "Project environment setup failed or not in a project directory"
        return 1
    fi
}

# Legacy compatibility functions (call C++ version internally)
refresh_project_env() {
    echo "Refreshing project environment for current directory..."
    if project_env -p \${WEBPORT:-8000}; then
        show_env_vars
    fi
}

show_env_vars() {
    echo "Current Project Environment Variables:"
    echo "------------------------------------"
    
    local env_vars=(
        "PROJECT_NAME" "PROFILE" "GQLHOST" "WEBPORT" "GQLPORT" 
        "SBPORT" "NDEBUGPORT" "GQLNUMBEROFMAXRETRIES" "GQLSERVERPATH"
        "GQLHTTPS" "GQLINTROSPECTION" "GQLTRANSFERMODE"
    )
    
    for var in "\${env_vars[@]}"; do
        echo "\$var: \${!var:-Not set}"
    done
    
    echo "------------------------------------"
}

# Convenient aliases for project environment management
alias fix_env='refresh_project_env'
alias fix_project='refresh_project_env'  
alias project_fix='refresh_project_env'

# Auto-setup for new terminals (only in workspace directories)
auto_setup_new_terminal() {
    if [[ "\$(pwd)" == "\$HOME/workspaces"/* ]]; then
        if [[ -z "\$PROJECT_NAME" ]]; then
            project_env -p \${WEBPORT:-8000} -t plain 2>/dev/null || true
        fi
    fi
}

# Run auto-setup for new terminals
auto_setup_new_terminal

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
EOF
)

# Check if .bash_aliases exists
if [ ! -f "$HOME/.bash_aliases" ]; then
  status "Creating .bash_aliases file"
  
  # Create the .bash_aliases file with the template
  echo "$BASH_ALIASES_TEMPLATE" > "$HOME/.bash_aliases"

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
      
      # Ask user if they want to update the file
      read -p "Do you want to update your .bash_aliases with C++ aliases-cli integration? (y/n): " -n 1 -r
      echo
      if [[ $REPLY =~ ^[Yy]$ ]]; then
        # Backup the existing file
        cp "$HOME/.bash_aliases" "$HOME/.bash_aliases.bak"
        
        # Replace the file
        echo "$BASH_ALIASES_TEMPLATE" > "$HOME/.bash_aliases"
        
        success "Updated .bash_aliases with C++ integration and created backup at .bash_aliases.bak"
      else
        warning "User chose not to update .bash_aliases - C++ features may not be available"
      fi
    fi
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
      
      success "Updated .bash_aliases with correct path and C++ integration, created backup at .bash_aliases.bak"
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

success "Setup complete! Your bash aliases are now configured with C++ aliases-cli integration."
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
