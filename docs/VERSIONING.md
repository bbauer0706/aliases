# Versioning Guide for aliases-cli

This project follows [Semantic Versioning 2.0.0](https://semver.org/) with automated releases.

## Version Format

**MAJOR.MINOR.PATCH** (e.g., `1.2.3`)

- **MAJOR** - Incompatible API changes or major rewrites
- **MINOR** - New features, backward-compatible
- **PATCH** - Bug fixes, backward-compatible

## How Versioning Works

**Version storage:**
- Versions exist ONLY in git tags (e.g., `v1.2.0`)
- No VERSION file in the codebase
- Binary version is extracted from git tags during build

**Automated releases:**
- CI/CD analyzes commit messages using conventional commits
- Automatically determines version bump type
- Creates git tag and GitHub release
- No manual version commits needed

## Creating a Release

### Automatic Release (Recommended)

Simply commit with conventional commit messages and push to main:

```bash
# New feature → Minor version bump (1.0.0 → 1.1.0)
git commit -m "feat: add config sync support"
git push origin main

# Bug fix → Patch version bump (1.0.0 → 1.0.1)
git commit -m "fix: resolve authentication timeout"
git push origin main

# Breaking change → Major version bump (1.0.0 → 2.0.0)
git commit -m "feat!: redesign configuration structure

BREAKING CHANGE: Config format has changed"
git push origin main
```

The CI/CD workflow will:
1. Analyze your commits
2. Determine the version bump
3. Create a git tag (e.g., v1.1.0)
4. Build the binary
5. Create a GitHub release

### Manual Tag Release

If you prefer manual control:

```bash
# Create and push a tag manually
git tag -a v1.1.0 -m "Release version 1.1.0"
git push origin v1.1.0

# The release workflow will build and create a GitHub release
```

## Commit Message Convention

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

### Types and Version Bumps

| Commit Type | Version Bump | Example |
|-------------|--------------|---------|
| `feat:` | Minor | `feat: add sync support` |
| `fix:` | Patch | `fix: resolve timeout` |
| `docs:` | Patch | `docs: update README` |
| `chore:` | Patch | `chore: update deps` |
| `refactor:` | Patch | `refactor: improve structure` |
| `perf:` | Patch | `perf: optimize search` |
| `test:` | Patch | `test: add unit tests` |
| `feat!:` or `BREAKING CHANGE:` | Major | `feat!: redesign API` |

### Examples

```bash
# Feature (minor bump)
git commit -m "feat(config): add HTTP sync support"

# Bug fix (patch bump)
git commit -m "fix(build): resolve linking issues"

# Breaking change (major bump)
git commit -m "feat!: change config format"

# Or with body
git commit -m "feat: new API

BREAKING CHANGE: Old endpoints removed"

# Documentation (patch bump)
git commit -m "docs: update versioning guide"
```

## Version Detection

The binary gets its version from git tags during build:

```bash
# Build extracts version from git
./build.sh

# If no tags exist, version shows "dev"
./build/aliases-cli --version
# Output: aliases-cli version dev

# With tags
git tag v1.2.0
./build.sh
./build/aliases-cli --version
# Output: aliases-cli version 1.2.0
```

## Checking Versions

```bash
# Show current version from binary
./build/aliases-cli --version

# List all tags
git tag -l

# Show tag details
git show v1.0.0

# Get latest tag
git describe --tags --abbrev=0

# Compare versions
git diff v1.0.0 v1.1.0
```

## Rollback

If you need to rollback a release:

```bash
# Delete local tag
git tag -d v1.1.0

# Delete remote tag
git push --delete origin v1.1.0

# Delete GitHub release (via web UI or gh CLI)
gh release delete v1.1.0
```

## Workflows

See `.github/workflows/` for available workflows:

- **ci-cd.yml** - Automatic releases on push to main
- **release.yml** - Triggered by manual tag push

For detailed workflow documentation, see `.github/workflows/README.md`

## Best Practices

1. **Use conventional commits** for automatic versioning
2. **Let CI/CD handle releases** - no manual version commits
3. **Write clear commit messages** describing the "why"
4. **Test locally** before pushing to main
5. **Use annotated tags** if creating manual tags
6. **Never force-push tags** once published

## Migration from Manual Versioning

**Old way (deprecated):**
```bash
# Manually update VERSION file
echo "1.1.0" > VERSION
# Update version in source code
sed -i 's/VERSION = "[^"]*"/VERSION = "1.1.0"/' src/main.cpp
# Update changelog
vim CHANGELOG.md
# Commit, tag, and release manually
git commit -m "chore: bump version to 1.1.0"
git tag -a v1.1.0 -m "Release v1.1.0"
git push origin main && git push origin v1.1.0
```

**New way (automated):**
```bash
git commit -m "feat: new feature"
git push origin main
# Done! CI/CD handles the rest ✨
```

## See Also

- [Semantic Versioning](https://semver.org/)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [GitHub Actions Documentation](https://docs.github.com/actions)
- [Workflow README](.github/workflows/README.md)
