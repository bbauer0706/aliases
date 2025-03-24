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

  success "Created .bash_aliases file"
else
  status ".bash_aliases already exists, checking format"
  
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

success "Setup complete! Your bash aliases are now configured."
echo ""
echo -e "${YELLOW}Note:${NC} Changes to alias files will take effect in new terminal sessions"
echo -e "      or after running 'source ~/.bash_aliases'"