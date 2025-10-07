# Versioning System Overview

## What Was Created

Your project now has a complete versioning and release system:

### Core Files
- **`VERSION`** - Single source of truth for version number (1.0.0)
- **`VERSIONING.md`** - Complete guide with best practices
- **`CHANGELOG.md`** - Pre-filled release history
- **`docs/RELEASE-CHECKLIST.md`** - Step-by-step release guide

### Automation Scripts
- **`scripts/bump-version.sh`** - Automates version bumping
- **`scripts/create-release.sh`** - Automates release creation
- **`.github/workflows/release.yml`** - GitHub Actions automation

## Quick Reference

### Version Bump Types

| Type | When to Use | Example |
|------|-------------|---------|
| **patch** | Bug fixes, docs | 1.0.0 → 1.0.1 |
| **minor** | New features | 1.0.0 → 1.1.0 |
| **major** | Breaking changes | 1.0.0 → 2.0.0 |

### Common Workflows

#### 1. Simple Bug Fix Release
```bash
# Fix the bug, commit normally
git add src/file.cpp
git commit -m "fix: resolve authentication issue"
git push

# Bump patch version
./scripts/bump-version.sh patch

# Update changelog
vim CHANGELOG.md  # Add fix details

# Commit and release
git add VERSION src/main.cpp CHANGELOG.md
git commit -m "chore: bump version to 1.0.1"
git push
./scripts/create-release.sh
```

#### 2. New Feature Release
```bash
# Develop feature, commit normally
git add src/new-feature.cpp
git commit -m "feat(todo): add search functionality"
git push

# Bump minor version
./scripts/bump-version.sh minor

# Update changelog
vim CHANGELOG.md  # Add feature details

# Commit and release
git add VERSION src/main.cpp CHANGELOG.md
git commit -m "chore: bump version to 1.1.0"
git push
./scripts/create-release.sh
```

#### 3. Emergency Hotfix
```bash
# Fix critical bug
git add src/critical-fix.cpp
git commit -m "fix!: resolve security vulnerability"
git push

# Quick release
./scripts/bump-version.sh patch
vim CHANGELOG.md
git add VERSION src/main.cpp CHANGELOG.md
git commit -m "chore: bump version to 1.0.1 (hotfix)"
git push
./scripts/create-release.sh
```

## Commit Message Standards

Following [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

### Types
- `feat` - New feature
- `fix` - Bug fix
- `docs` - Documentation
- `chore` - Maintenance
- `refactor` - Code restructuring
- `test` - Tests
- `perf` - Performance
- `style` - Formatting
- `build` - Build system
- `ci` - CI/CD

### Examples
```bash
feat(config): add sync support with git/rsync/file/http
fix(build): resolve linking issues with config module
docs: update versioning guide
chore: bump version to 1.1.0
refactor(todo): improve TUI performance
test(config): add unit tests for sync module
```

## GitHub Integration

### Manual Release (Using Scripts)
```bash
./scripts/create-release.sh
```
- Builds binary
- Creates tag
- Pushes to GitHub
- Creates release with binary

### Automated Release (GitHub Actions)
```bash
git tag -a v1.1.0 -m "Release v1.1.0"
git push origin v1.1.0
```
- GitHub Actions triggered automatically
- Builds on GitHub servers
- Creates release
- Attaches binary

### GitHub CLI Integration
If you have `gh` CLI installed:
```bash
# View releases
gh release list

# View specific release
gh release view v1.0.0

# Download release asset
gh release download v1.0.0

# Delete release
gh release delete v1.0.0
```

## Best Practices

1. **Always update CHANGELOG.md** before releasing
2. **Use annotated tags** (`git tag -a`) not lightweight
3. **Test before releasing** - build and test binary
4. **Keep VERSION in sync** with git tags
5. **Write clear commit messages** following conventions
6. **Review changes** before tagging
7. **Never force-push tags** once published
8. **Document breaking changes** clearly

## Troubleshooting

### Version Mismatch
```bash
# If VERSION file doesn't match code
cat VERSION
grep VERSION src/main.cpp
./scripts/bump-version.sh patch  # Re-sync
```

### Tag Already Exists
```bash
# Delete and recreate
git tag -d v1.0.0
git push --delete origin v1.0.0
# Now create new tag
```

### Release Creation Failed
```bash
# Check if gh CLI is installed
which gh
# Install if needed
sudo apt install gh
gh auth login

# Retry release
./scripts/create-release.sh
```

### GitHub Actions Not Running
- Ensure `.github/workflows/release.yml` is committed
- Check Actions tab on GitHub
- Verify GitHub token permissions
- Check workflow syntax

## Version History Example

Your CHANGELOG.md now tracks:
- v1.0.0 - Initial stable release with config sync
- v0.9.0 - Todo system implementation
- v0.5.0 - Core navigation features
- v0.1.0 - Initial C++ implementation

## Files to Track in Git

**Always commit:**
- `VERSION`
- `CHANGELOG.md`
- `src/main.cpp` (when VERSION changes)
- `VERSIONING.md`
- `scripts/*.sh`
- `.github/workflows/*.yml`

**Don't commit:**
- `build/` directory
- Compiled binaries
- Temporary files

## Integration with Development Workflow

```
Feature Development → Commit → Push
         ↓
   Ready to Release?
         ↓
   Bump Version → Update CHANGELOG
         ↓
   Commit Version → Run create-release.sh
         ↓
   Tag Created → GitHub Release
         ↓
   (Optional) GitHub Actions Builds
```

## Next Steps

1. **Commit versioning files:**
   ```bash
   git add VERSION VERSIONING.md CHANGELOG.md
   git add scripts/ .github/ docs/RELEASE-CHECKLIST.md
   git commit -m "chore: add versioning system"
   git push
   ```

2. **Create first release:**
   ```bash
   ./scripts/create-release.sh 1.0.0
   ```

3. **Verify on GitHub:**
   - Check: https://github.com/bbauer0706/aliases/releases
   - Should see v1.0.0 with binary

4. **Update README (optional):**
   Add installation instructions for releases

## Support

- Read `VERSIONING.md` for detailed guide
- Check `docs/RELEASE-CHECKLIST.md` for step-by-step
- Follow conventional commits standard
- Keep CHANGELOG.md updated

---

**System Ready!** You can now create professional releases with proper versioning. 🎉
