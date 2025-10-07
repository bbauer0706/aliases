# Versioning Guide for aliases-cli

This project follows [Semantic Versioning 2.0.0](https://semver.org/).

## Version Format

**MAJOR.MINOR.PATCH** (e.g., `1.2.3`)

- **MAJOR** - Incompatible API changes or major rewrites
- **MINOR** - New features, backward-compatible
- **PATCH** - Bug fixes, backward-compatible

## Current Version

The current version is stored in:
- `VERSION` file (single source of truth)
- `src/main.cpp` (updated manually or via script)

## Creating a New Release

### 1. Determine Version Bump

**PATCH** (1.0.0 → 1.0.1):
- Bug fixes
- Documentation updates
- Performance improvements
- Refactoring without feature changes

**MINOR** (1.0.0 → 1.1.0):
- New features
- New commands or subcommands
- New configuration options
- Deprecations (but not removals)

**MAJOR** (1.0.0 → 2.0.0):
- Breaking changes to CLI interface
- Removal of features
- Major architecture changes
- Configuration format changes

### 2. Update Version

```bash
# Option 1: Use the version script (recommended)
./scripts/bump-version.sh patch   # 1.0.0 → 1.0.1
./scripts/bump-version.sh minor   # 1.0.0 → 1.1.0
./scripts/bump-version.sh major   # 1.0.0 → 2.0.0

# Option 2: Manual update
# Edit VERSION file
echo "1.1.0" > VERSION
# Update src/main.cpp
vim src/main.cpp  # Change VERSION constant
```

### 3. Update CHANGELOG

```bash
# Edit CHANGELOG.md
vim CHANGELOG.md

# Add entry:
## [1.1.0] - 2025-10-07
### Added
- New config sync feature with git/rsync/file/http support
- Configuration management commands

### Changed
- Improved error messages

### Fixed
- Fixed build issues with config module
```

### 4. Commit Version Changes

```bash
git add VERSION src/main.cpp CHANGELOG.md
git commit -m "chore: bump version to 1.1.0"
```

### 5. Create Git Tag

```bash
# Create annotated tag (recommended)
git tag -a v1.1.0 -m "Release version 1.1.0

New Features:
- Config sync with multiple methods
- Enhanced configuration management

Bug Fixes:
- Build system improvements

See CHANGELOG.md for full details."

# Push tag to remote
git push origin v1.1.0

# Or push all tags
git push --tags
```

### 6. Create GitHub Release

**Option 1: Via GitHub Web UI**
1. Go to https://github.com/bbauer0706/aliases/releases
2. Click "Draft a new release"
3. Select tag `v1.1.0`
4. Title: `v1.1.0 - Config Sync & Improvements`
5. Copy content from CHANGELOG.md
6. Attach binary: `build/aliases-cli`
7. Click "Publish release"

**Option 2: Via GitHub CLI**
```bash
# Install gh if not already
# sudo apt install gh

gh release create v1.1.0 \
  --title "v1.1.0 - Config Sync & Improvements" \
  --notes-file release-notes/v1.1.0.md \
  build/aliases-cli
```

## Version History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 2025-10-07 | Initial stable release with config sync |
| 0.9.0 | - | Beta with todo TUI |
| 0.1.0 | - | Initial C++ implementation |

## Tagging Convention

- **Release tags**: `v1.0.0`, `v1.1.0`, etc.
- **Pre-release tags**: `v1.0.0-rc1`, `v1.0.0-beta2`, etc.
- **Development tags**: `v1.0.0-dev`, etc.

### Pre-release Examples

```bash
# Release candidate
git tag -a v1.1.0-rc1 -m "Release candidate 1 for v1.1.0"

# Beta release
git tag -a v1.1.0-beta1 -m "Beta 1 for v1.1.0"

# Alpha release
git tag -a v1.1.0-alpha1 -m "Alpha 1 for v1.1.0"
```

## Commit Message Convention

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `chore`: Maintenance tasks
- `refactor`: Code refactoring
- `test`: Test changes
- `perf`: Performance improvements
- `style`: Code style changes
- `build`: Build system changes
- `ci`: CI/CD changes

**Examples:**
```bash
git commit -m "feat(config): add sync support with multiple methods"
git commit -m "fix(build): resolve config module linking issues"
git commit -m "docs: update all documentation for config sync"
git commit -m "chore: bump version to 1.1.0"
```

## Automated Versioning (Future)

Consider adding:
- GitHub Actions for automatic releases
- Changelog generation from commits
- Version bumping in CI/CD

Example GitHub Action (`.github/workflows/release.yml`):
```yaml
name: Release
on:
  push:
    tags:
      - 'v*'
jobs:
  release:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build
        run: ./build.sh
      - name: Create Release
        uses: softprops/action-gh-release@v1
        with:
          files: build/aliases-cli
```

## Best Practices

1. **Always update CHANGELOG.md** before tagging
2. **Use annotated tags** (`git tag -a`) not lightweight tags
3. **Write descriptive tag messages** with release highlights
4. **Test thoroughly** before creating releases
5. **Build and attach binaries** to GitHub releases
6. **Keep VERSION file** in sync with git tags
7. **Never delete or modify published tags**

## Checking Versions

```bash
# Show current version
./build/aliases-cli --version

# List all tags
git tag -l

# Show tag details
git show v1.0.0

# Compare versions
git diff v1.0.0 v1.1.0
```

## Rollback

If you need to rollback a release:

```bash
# Delete local tag
git tag -d v1.1.0

# Delete remote tag
git push origin :refs/tags/v1.1.0

# Or use
git push --delete origin v1.1.0

# Delete GitHub release via web UI or gh CLI
gh release delete v1.1.0
```

## See Also

- [Semantic Versioning](https://semver.org/)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Keep a Changelog](https://keepachangelog.com/)
