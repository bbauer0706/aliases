# Release Checklist

Use this checklist when creating a new release.

## Pre-Release

- [ ] All features/fixes are committed and pushed
- [ ] Tests pass (if applicable)
- [ ] Documentation is updated
- [ ] No uncommitted changes

## Release Process

### 1. Bump Version

```bash
# Choose appropriate bump type
./scripts/bump-version.sh patch   # Bug fixes (1.0.0 → 1.0.1)
./scripts/bump-version.sh minor   # New features (1.0.0 → 1.1.0)
./scripts/bump-version.sh major   # Breaking changes (1.0.0 → 2.0.0)
```

This updates:
- `VERSION` file
- `src/main.cpp` VERSION constant

### 2. Update CHANGELOG.md

```bash
vim CHANGELOG.md
```

Add entry for new version:
```markdown
## [1.1.0] - 2025-10-07

### Added
- New feature 1
- New feature 2

### Changed
- Changed behavior 1

### Fixed
- Bug fix 1
- Bug fix 2
```

### 3. Review Changes

```bash
git diff VERSION src/main.cpp CHANGELOG.md
```

### 4. Commit Version Bump

```bash
git add VERSION src/main.cpp CHANGELOG.md
git commit -m "chore: bump version to 1.1.0"
git push origin main
```

### 5. Create Release

```bash
# Option A: Use automated script (recommended)
./scripts/create-release.sh

# Option B: Manual process
# Build binary
./build.sh

# Create and push tag
git tag -a v1.1.0 -m "Release version 1.1.0

[Copy content from CHANGELOG.md]
"
git push origin v1.1.0

# Create GitHub release
gh release create v1.1.0 \
  --title "v1.1.0" \
  --notes-file CHANGELOG.md \
  build/aliases-cli
```

## Post-Release

- [ ] Verify tag exists: `git tag -l`
- [ ] Check GitHub release: https://github.com/bbauer0706/aliases/releases
- [ ] Test download: `curl -L https://github.com/bbauer0706/aliases/releases/download/v1.1.0/aliases-cli-1.1.0-linux-x86_64 -o test-binary`
- [ ] Announce release (if public)
- [ ] Update any dependent projects

## Quick Commands

```bash
# List all tags
git tag -l

# Show tag details
git show v1.0.0

# Delete local tag (if mistake)
git tag -d v1.0.0

# Delete remote tag
git push --delete origin v1.0.0

# Delete GitHub release
gh release delete v1.0.0

# View current version
./build/aliases-cli --version
cat VERSION
```

## Commit Message Examples

```bash
# Feature
git commit -m "feat(config): add sync support with multiple methods"

# Bug fix
git commit -m "fix(build): resolve config module linking issues"

# Documentation
git commit -m "docs: update versioning guide"

# Version bump
git commit -m "chore: bump version to 1.1.0"

# Breaking change
git commit -m "feat!: redesign configuration structure

BREAKING CHANGE: Configuration format has changed.
Users need to migrate their config files."
```

## Semantic Versioning Quick Reference

| Change Type | Version Bump | Example |
|-------------|--------------|---------|
| Bug fixes, docs, patches | PATCH | 1.0.0 → 1.0.1 |
| New features (backward-compatible) | MINOR | 1.0.0 → 1.1.0 |
| Breaking changes | MAJOR | 1.0.0 → 2.0.0 |

## Troubleshooting

**Tag already exists:**
```bash
git tag -d v1.1.0
git push --delete origin v1.1.0
```

**Wrong version committed:**
```bash
# Reset to previous commit
git reset --hard HEAD~1

# Re-run version bump script
./scripts/bump-version.sh patch
```

**GitHub release failed:**
```bash
# Install GitHub CLI
sudo apt install gh

# Login
gh auth login

# Retry release
gh release create v1.1.0 --title "v1.1.0" --notes-file CHANGELOG.md build/aliases-cli
```
