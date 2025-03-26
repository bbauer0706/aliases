#!/bin/bash
##############################################################################
#                             MAVEN SHORTCUTS                                #
##############################################################################

# Define mappings of shorthand commands to maven commands
declare -A mvn_commands=(
  [ci]="clean install"
  [cp]="clean package"
  [cid]="clean install -DskipTests"
  [cpd]="clean package -DskipTests"
  [t]="test"
  [tc]="test -Dtest="
  [tm]="test -Dtest="
  [run]="spring-boot:run"
)

# Basic Maven alias for backward compatibility
alias mcp='mvn clean package'

# Maven function with shortcuts
m() {
  # Help parameters
  if [[ $# -eq 0 || "$1" == "-h" || "$1" == "--help" ]]; then
    echo "mvn command shortcuts:"
    for cmd in "${!mvn_commands[@]}"; do
      echo "  $cmd => mvn ${mvn_commands[$cmd]}"
    done
    return
  fi

  local cmd="$1"
  shift # Remove the first argument

  # Check if it's a known shorthand command
  if [[ -n "${mvn_commands[$cmd]}" ]]; then
    # Special case for test class commands that need the parameter appended
    if [[ "$cmd" == "tc" || "$cmd" == "tm" ]]; then
      mvn ${mvn_commands[$cmd]}"$1"
    else
      mvn ${mvn_commands[$cmd]} "$@"
    fi
  else
    # Check if it's a standard Maven phase/goal
    if [[ "$cmd" =~ ^(clean|compile|test|package|verify|install|deploy|site)$ ]]; then
      mvn "$cmd" "$@"
    else
      # Unknown command, show error
      echo "Error: Unknown Maven shortcut or phase: '$cmd'"
      echo ""
      echo "Available shortcuts:"
      for cmd in "${!mvn_commands[@]}"; do
        echo "  $cmd => mvn ${mvn_commands[$cmd]}"
      done
      
      echo ""
      echo "Standard Maven phases:"
      echo "  clean, compile, test, package, verify, install, deploy, site"
      echo ""
    fi
  fi
}

# Tab completion for the 'm' function
_m_completion() {
  local cur=${COMP_WORDS[COMP_CWORD]}
  local prev=${COMP_WORDS[COMP_CWORD-1]}
  local mvn_shortcuts=()
  
  # First argument - command completion
  if [[ $COMP_CWORD -eq 1 ]]; then
    # Add all shorthand commands
    for cmd in "${!mvn_commands[@]}"; do
      mvn_shortcuts+=("$cmd")
    done
    
  elif [[ $COMP_CWORD -eq 2 ]]; then
    # Second argument handling for specific commands
    case "$prev" in
      tc|tm)
        # Try to find test classes if possible
        if [ -d "src/test/java" ]; then
          # This is a simplified approach to find test classes
          # For complex projects, a more sophisticated solution might be needed
          local test_classes=$(find src/test/java -name "*Test.java" -o -name "*Tests.java" | sed 's/.*\/\([^\/]*\)\.java$/\1/')
          COMPREPLY=($(compgen -W "$test_classes" -- "$cur"))
          return 0
        fi
        ;;
    esac
  fi
  
  # Set completion reply for general case
  COMPREPLY=($(compgen -W "${mvn_shortcuts[*]}" -- "$cur"))
  return 0
}

complete -F _m_completion m