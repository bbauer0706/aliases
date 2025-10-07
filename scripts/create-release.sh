#!/bin/bash
##############################################################################
#                          CREATE RELEASE SCRIPT                             #
##############################################################################
#
# Usage: ./scripts/create-release.sh [VERSION]
#
# This script:
# 1. Verifies version is committed
# 2. Builds the binary
# 3. Creates git tag
# 4. Pushes tag
# 5. Creates GitHub release (if gh CLI available)
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

# Get version
if [ $# -eq 0 ]; then
    # Read from VERSION file
    VERSION=$(cat VERSION | tr -d '[:space:]')
else
    VERSION=$1
fi

# Remove 'v' prefix if present
VERSION=${VERSION#v}

print_info "Creating release for version: $VERSION"

# Validate version format
if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    print_error "Invalid version format: $VERSION"
    print_error "Expected format: MAJOR.MINOR.PATCH (e.g., 1.0.0)"
    exit 1
fi

# Check for uncommitted changes
if ! git diff-index --quiet HEAD --; then
    print_error "You have uncommitted changes. Please commit them first."
    exit 1
fi

# Verify VERSION file matches
VERSION_FILE_CONTENT=$(cat VERSION | tr -d '[:space:]')
if [ "$VERSION_FILE_CONTENT" != "$VERSION" ]; then
    print_error "VERSION file ($VERSION_FILE_CONTENT) doesn't match specified version ($VERSION)"
    exit 1
fi

# Check if tag already exists
if git rev-parse "v$VERSION" >/dev/null 2>&1; then
    print_error "Tag v$VERSION already exists!"
    print_info "Delete it first with: git tag -d v$VERSION && git push --delete origin v$VERSION"
    exit 1
fi

# Build binary
print_info "Building binary..."
./build.sh

if [ ! -f "build/aliases-cli" ]; then
    print_error "Binary build failed - build/aliases-cli not found"
    exit 1
fi

print_success "Binary built successfully"

# Get binary size
BINARY_SIZE=$(du -h build/aliases-cli | cut -f1)
print_info "Binary size: $BINARY_SIZE"

# Extract changelog for this version
print_info "Extracting changelog..."
RELEASE_NOTES=$(mktemp)

if [ -f "CHANGELOG.md" ]; then
    # Extract section for this version
    sed -n "/## \[$VERSION\]/,/## \[/p" CHANGELOG.md | sed '$d' > "$RELEASE_NOTES"
    
    if [ ! -s "$RELEASE_NOTES" ]; then
        print_warning "No changelog entry found for version $VERSION"
        echo "Release $VERSION" > "$RELEASE_NOTES"
        echo "" >> "$RELEASE_NOTES"
        echo "See CHANGELOG.md for details." >> "$RELEASE_NOTES"
    fi
else
    print_warning "CHANGELOG.md not found"
    echo "Release $VERSION" > "$RELEASE_NOTES"
fi

# Show release notes
echo ""
print_info "Release notes:"
echo "----------------------------------------"
cat "$RELEASE_NOTES"
echo "----------------------------------------"
echo ""

# Confirm
read -p "Create release v$VERSION? (y/N) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    print_info "Aborted."
    rm "$RELEASE_NOTES"
    exit 0
fi

# Create annotated tag
print_info "Creating git tag v$VERSION..."
git tag -a "v$VERSION" -F "$RELEASE_NOTES"
print_success "Tag created"

# Push tag
print_info "Pushing tag to remote..."
git push origin "v$VERSION"
print_success "Tag pushed"

# Create GitHub release if gh CLI is available
if command -v gh &> /dev/null; then
    print_info "Creating GitHub release..."
    
    gh release create "v$VERSION" \
        --title "v$VERSION" \
        --notes-file "$RELEASE_NOTES" \
        build/aliases-cli#aliases-cli-$VERSION-linux-x86_64
    
    print_success "GitHub release created!"
    echo ""
    print_info "View release at: https://github.com/bbauer0706/aliases/releases/tag/v$VERSION"
else
    print_warning "GitHub CLI (gh) not found - skipping GitHub release creation"
    print_info "Create release manually at: https://github.com/bbauer0706/aliases/releases/new?tag=v$VERSION"
    print_info "Attach binary: build/aliases-cli"
fi

# Cleanup
rm "$RELEASE_NOTES"

echo ""
print_success "Release v$VERSION created successfully!"
echo ""
print_info "Next steps:"
echo "  - Verify release: https://github.com/bbauer0706/aliases/releases"
echo "  - Test installation: curl -o aliases-cli https://github.com/bbauer0706/aliases/releases/download/v$VERSION/aliases-cli-$VERSION-linux-x86_64"
echo "  - Announce release (if public)"
