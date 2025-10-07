#!/bin/bash
##############################################################################
#                           VERSION BUMP SCRIPT                              #
##############################################################################
#
# Usage: ./scripts/bump-version.sh [major|minor|patch]
#
# This script:
# 1. Reads current version from VERSION file
# 2. Bumps version according to semantic versioning
# 3. Updates VERSION file and src/main.cpp
# 4. Shows git commands to commit and tag (doesn't execute them)
#

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check arguments
if [ $# -ne 1 ]; then
    print_error "Usage: $0 [major|minor|patch]"
    exit 1
fi

BUMP_TYPE=$1

if [[ ! "$BUMP_TYPE" =~ ^(major|minor|patch)$ ]]; then
    print_error "Invalid bump type: $BUMP_TYPE"
    print_error "Must be one of: major, minor, patch"
    exit 1
fi

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

VERSION_FILE="$PROJECT_ROOT/VERSION"
MAIN_CPP="$PROJECT_ROOT/src/main.cpp"

# Check if VERSION file exists
if [ ! -f "$VERSION_FILE" ]; then
    print_error "VERSION file not found at $VERSION_FILE"
    exit 1
fi

# Check if main.cpp exists
if [ ! -f "$MAIN_CPP" ]; then
    print_error "main.cpp not found at $MAIN_CPP"
    exit 1
fi

# Read current version
CURRENT_VERSION=$(cat "$VERSION_FILE" | tr -d '[:space:]')
print_info "Current version: $CURRENT_VERSION"

# Parse version
if [[ ! "$CURRENT_VERSION" =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)$ ]]; then
    print_error "Invalid version format in VERSION file: $CURRENT_VERSION"
    print_error "Expected format: MAJOR.MINOR.PATCH (e.g., 1.0.0)"
    exit 1
fi

MAJOR="${BASH_REMATCH[1]}"
MINOR="${BASH_REMATCH[2]}"
PATCH="${BASH_REMATCH[3]}"

# Bump version
case $BUMP_TYPE in
    major)
        MAJOR=$((MAJOR + 1))
        MINOR=0
        PATCH=0
        ;;
    minor)
        MINOR=$((MINOR + 1))
        PATCH=0
        ;;
    patch)
        PATCH=$((PATCH + 1))
        ;;
esac

NEW_VERSION="$MAJOR.$MINOR.$PATCH"
print_info "New version: $NEW_VERSION"

# Check for uncommitted changes
if ! git diff-index --quiet HEAD --; then
    print_warning "You have uncommitted changes. Consider committing them first."
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_info "Aborted."
        exit 0
    fi
fi

# Update VERSION file
print_info "Updating VERSION file..."
echo "$NEW_VERSION" > "$VERSION_FILE"

# Update main.cpp
print_info "Updating src/main.cpp..."
sed -i "s/constexpr const char\* VERSION = \"[^\"]*\";/constexpr const char* VERSION = \"$NEW_VERSION\";/" "$MAIN_CPP"

# Verify changes
if grep -q "VERSION = \"$NEW_VERSION\"" "$MAIN_CPP"; then
    print_success "Updated src/main.cpp successfully"
else
    print_error "Failed to update src/main.cpp"
    exit 1
fi

if [ "$(cat $VERSION_FILE | tr -d '[:space:]')" == "$NEW_VERSION" ]; then
    print_success "Updated VERSION file successfully"
else
    print_error "Failed to update VERSION file"
    exit 1
fi

print_success "Version bumped: $CURRENT_VERSION → $NEW_VERSION"

# Show git commands
echo ""
print_info "Next steps:"
echo ""
echo "1. Update CHANGELOG.md with changes for v$NEW_VERSION"
echo "   vim CHANGELOG.md"
echo ""
echo "2. Review changes:"
echo "   git diff VERSION src/main.cpp CHANGELOG.md"
echo ""
echo "3. Commit version bump:"
echo "   git add VERSION src/main.cpp CHANGELOG.md"
echo "   git commit -m \"chore: bump version to $NEW_VERSION\""
echo ""
echo "4. Create git tag:"
echo "   git tag -a v$NEW_VERSION -m \"Release version $NEW_VERSION\""
echo ""
echo "5. Push changes:"
echo "   git push origin main"
echo "   git push origin v$NEW_VERSION"
echo ""
echo "6. Create GitHub release:"
echo "   gh release create v$NEW_VERSION --title \"v$NEW_VERSION\" --notes-file CHANGELOG.md build/aliases-cli"
echo ""
print_info "Or use the release script:"
echo "   ./scripts/create-release.sh $NEW_VERSION"
