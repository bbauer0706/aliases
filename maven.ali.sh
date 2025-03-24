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
  # No parameters, show help
  if [[ $# -eq 0 ]]; then
    echo "Maven shortcuts:"
    echo "  m ci      - mvn clean install"
    echo "  m cp      - mvn clean package"
    echo "  m cid     - mvn clean install -DskipTests"
    echo "  m cpd     - mvn clean package -DskipTests"
    echo "  m t       - mvn test"
    echo "  m tc      - mvn test -Dtest=ClassName"
    echo "  m tm      - mvn test -Dtest=ClassName#methodName"
    echo "  m run     - mvn spring-boot:run"
    return
  fi

  local cmd="$1"
  shift # Remove the first argument

  case "$cmd" in
    ci)  mvn clean install ;;
    cp)  mvn clean package ;;
    cid) mvn clean install -DskipTests ;;
    cpd) mvn clean package -DskipTests ;;
    t)   mvn test ;;
    tc)  mvn test -Dtest="$1" ;;
    tm)  mvn test -Dtest="$1" ;;
    run) mvn spring-boot:run ;;
    *)   mvn "$cmd" "$@" ;;
  esac
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