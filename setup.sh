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

# Determine if this is an initial setup or a post-clone setup
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
EXPECTED_ALIASES_DIR="$HOME/aliases"

# Check if we're running from the expected aliases directory
if [ "$SCRIPT_DIR" != "$EXPECTED_ALIASES_DIR" ]; then
  # This is likely a post-clone setup, so we need to create a symlink
  status "Detected script running from non-standard location: $SCRIPT_DIR"
  status "Will create symlink from $SCRIPT_DIR to $EXPECTED_ALIASES_DIR"
  
  # Check if target directory already exists
  if [ -d "$EXPECTED_ALIASES_DIR" ]; then
    warning "Target directory $EXPECTED_ALIASES_DIR already exists."
    read -p "Do you want to replace it? This will remove all files in that directory. (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      status "Removing existing aliases directory"
      rm -rf "$EXPECTED_ALIASES_DIR"
    else
      error "Setup aborted by user"
    fi
  fi
  
  # Create symlink
  status "Creating symlink from $SCRIPT_DIR to $EXPECTED_ALIASES_DIR"
  ln -s "$SCRIPT_DIR" "$EXPECTED_ALIASES_DIR"
  
  if [ $? -ne 0 ]; then
    error "Failed to create symlink"
  fi
  
  success "Created symlink to aliases directory"
  ALIASES_DIR="$EXPECTED_ALIASES_DIR"
else
  # Running from the expected location
  ALIASES_DIR="$SCRIPT_DIR"
  status "Running from expected aliases directory: $ALIASES_DIR"
fi

# Check if .bash_aliases exists
if [ ! -f "$HOME/.bash_aliases" ]; then
  status "Creating .bash_aliases file"
  
  # Create the .bash_aliases file with a nice header
  cat > "$HOME/.bash_aliases" << 'EOF'
#!/bin/bash
##############################################################################
#                                                                            #
#                         CUSTOM BASH ALIASES                                #
#                                                                            #
#  This file is automatically managed by the aliases repository.             #
#  Make changes to the individual files in ~/aliases/ instead.               #
#                                                                            #
##############################################################################

# Source all alias files from the aliases directory
if [ -d "$HOME/aliases" ]; then
  for alias_file in "$HOME/aliases"/*.ali.sh; do
    if [ -f "$alias_file" ]; then
      source "$alias_file"
    fi
  done
fi
EOF
# Basic aliases (add your simple aliases here)
alias la='ls -A'ed .bash_aliases file"
alias ll='ls -l'
  status ".bash_aliases already exists, checking format"
# Kill process on port
  # Check if sourcing of alias files is already in .bash_aliases
  if ! grep -q "Source all alias files from the aliases directory" "$HOME/.bash_aliases"; then
    warning ".bash_aliases doesn't include the standard sourcing code"
    
    # Ask user if they want to update the file
    read -p "Do you want to update your .bash_aliases to source the alias files? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      # Backup the existing file
      cp "$HOME/.bash_aliases" "$HOME/.bash_aliases.bak"
      
      # Create temporary file with new content
      TMP_FILE=$(mktemp)
      cat > "$TMP_FILE" << 'EOF'
#!/bin/bash
##############################################################################
#                                                                            #
#                         CUSTOM BASH ALIASES                                #
#                                                                            #
#  This file is automatically managed by the aliases repository.             #
#  Make changes to the individual files in ~/aliases/ instead.               #
#                                                                            #
##############################################################################

# Source all alias files from the aliases directory
if [ -d "$HOME/aliases" ]; then
  for alias_file in "$HOME/aliases"/*.ali.sh; do
    if [ -f "$alias_file" ]; then
      source "$alias_file"
    fi
  done
fi

# Original content of .bash_aliases follows:
EOF
      
      # Append the original content
      cat "$HOME/.bash_aliases" >> "$TMP_FILE"
      
      # Replace the file
      mv "$TMP_FILE" "$HOME/.bash_aliases"
      
      success "Updated .bash_aliases and created backup at .bash_aliases.bak"
    else
      warning "User chose not to update .bash_aliases"
    fi
  else
    success ".bash_aliases already has the correct format"
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

# Check for missing standard alias files
ALIAS_FILES=("code.ali.sh" "git.ali.sh" "maven.ali.sh" "npm.ali.sh")

for file in "${ALIAS_FILES[@]}"; do
  if [ ! -f "$ALIASES_DIR/$file" ]; then
    status "Creating template for $file"
    
    # Create the header for each file
    cat > "$ALIASES_DIR/$file" << EOF
#!/bin/bash
##############################################################################
#                                                                            #
#                            ${file%.ali.sh} SHORTCUTS                                #
#                                                                            #
##############################################################################
" "npm.ali.sh")
# Add your ${file%.ali.sh} aliases and functions here
for file in "${ALIAS_FILES[@]}"; do
  if [ ! -f "$ALIASES_DIR/$file" ]; then
    status "Creating template for $file"
    # If this is a git repo, add the file
    # Create the header for each fileen
    cat > "$ALIASES_DIR/$file" << EOFe"
#!/bin/bash
##############################################################################
#                                                                            #
#                            ${file%.ali.sh} SHORTCUTS                                #
#                                                                            #
##############################################################################
# If this is a git repo, commit any new files
# Add your ${file%.ali.sh} aliases and functions hereASES_DIR" status --porcelain)" ]; then
  status "Committing new files to git repository"
EOFit -C "$ALIASES_DIR" commit -m "Added or updated standard alias files"
  success "Committed new files to git repository"
    # If this is a git repo, add the file
    if [ -d "$ALIASES_DIR/.git" ]; then
      git -C "$ALIASES_DIR" add "$file"ses are now configured."
    fi"
     -e "${BLUE}Next steps:${NC}"
    success "Created $file"al session or run 'source ~/.bash_aliases'"
  fi "2. Review and customize your alias files: cd ~/aliases && ls"
done ""
echo -e "${YELLOW}Note:${NC} Changes to alias files will take effect in new terminal sessions"
# If this is a git repo, commit any new filessh_aliases'"
if [ -d "$ALIASES_DIR/.git" ] && [ -n "$(git -C "$ALIASES_DIR" status --porcelain)" ]; then
  status "Committing new files to git repository"
  git -C "$ALIASES_DIR" commit -m "Added or updated standard alias files"
  success "Committed new files to git repository"fisuccess "Setup complete! Your bash aliases are now configured."echo ""echo -e "${BLUE}Next steps:${NC}"echo "1. Start a new terminal session or run 'source ~/.bash_aliases'"echo "2. Review and customize your alias files: cd ~/aliases && ls"echo ""echo -e "${YELLOW}Note:${NC} Changes to alias files will take effect in new terminal sessions"echo -e "      or after running 'source ~/.bash_aliases'"# Set the script as executablechmod +x "$ALIASES_DIR/setup.sh"