##############################################################################
#                             MAVEN SHORTCUTS                                #
##############################################################################

# Basic Maven alias
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

  case "$1" in
    ci)  mvn clean install ;;
    cp)  mvn clean package ;;
    cid) mvn clean install -DskipTests ;;
    cpd) mvn clean package -DskipTests ;;
    t)   mvn test ;;
    tc)  mvn test -Dtest="$2" ;;
    tm)  mvn test -Dtest="$2" ;;
    run) mvn spring-boot:run ;;
    *)   mvn "$@" ;;
  esac
}