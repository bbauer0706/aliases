# Release Checklist

## Automated Release (Recommended)

### Pre-Release
- [ ] All features/fixes are committed
- [ ] Tests pass (if applicable)
- [ ] Documentation is updated
- [ ] Commit messages follow conventional commits format

### Release Process

**Just push to main:**
```bash
# Commit with conventional commit message
git commit -m "feat: add new feature"
git push origin main

# CI/CD automatically:
# ✅ Analyzes commits
# ✅ Determines version bump
# ✅ Creates git tag (e.g., v1.1.0)
# ✅ Builds binary
# ✅ Creates GitHub release
```

### Post-Release
- [ ] Check GitHub Actions completed successfully
- [ ] Verify tag exists: `git tag -l`
- [ ] Check GitHub release page
- [ ] Test binary download from release

---

## Manual Tag Release

If you prefer manual control over when releases happen:

### 1. Determine Version

Based on changes since last tag:
- **Patch** (1.0.0 → 1.0.1): Bug fixes, docs
- **Minor** (1.0.0 → 1.1.0): New features
- **Major** (1.0.0 → 2.0.0): Breaking changes

### 2. Create and Push Tag

```bash
# Create annotated tag
git tag -a v1.1.0 -m "Release version 1.1.0

New Features:
- Feature 1
- Feature 2

Bug Fixes:
- Fix 1"

# Push tag
git push origin v1.1.0
```

### 3. Verify Release

The release workflow will automatically:
- Build the binary
- Create GitHub release
- Attach binary to release

---

## Commit Message Examples

### Features (Minor Bump)
```bash
git commit -m "feat(config): add HTTP sync support"
git commit -m "feat: add search functionality"
```

### Bug Fixes (Patch Bump)
```bash
git commit -m "fix(build): resolve linking issues"
git commit -m "fix: correct timeout handling"
```

### Breaking Changes (Major Bump)
```bash
git commit -m "feat!: redesign configuration structure

BREAKING CHANGE: Config format has changed.
Users need to migrate their config files."
```

### Documentation (Patch Bump)
```bash
git commit -m "docs: update versioning guide"
git commit -m "docs(readme): add installation section"
```

---

## Version Bump Reference

| Commit Type | Version Bump | Example |
|-------------|--------------|---------|
| `fix:`, `docs:`, `chore:` | Patch | 1.0.0 → 1.0.1 |
| `feat:` | Minor | 1.0.0 → 1.1.0 |
| `feat!:` or `BREAKING CHANGE:` | Major | 1.0.0 → 2.0.0 |

---

## Useful Commands

```bash
# Check current version
./build/aliases-cli --version

# List all tags
git tag -l

# Show latest tag
git describe --tags --abbrev=0

# View tag details
git show v1.0.0

# Delete local tag (if mistake)
git tag -d v1.0.0

# Delete remote tag
git push --delete origin v1.0.0

# Delete GitHub release
gh release delete v1.0.0

# View releases
gh release list
```

---

## Troubleshooting

### CI/CD workflow didn't trigger
- Check commit is on `main` branch
- Verify workflow file exists: `.github/workflows/ci-cd.yml`
- Check GitHub Actions tab for errors

### Wrong version bump
- Check commit message format
- Ensure using conventional commits
- Review "Determine version bump" step in workflow logs

### Release failed
- Check GitHub Actions logs
- Verify GITHUB_TOKEN permissions
- Ensure tag doesn't already exist

### Need to rollback
```bash
# Delete tag locally and remotely
git tag -d v1.1.0
git push --delete origin v1.1.0

# Delete GitHub release
gh release delete v1.1.0
```

---

## See Also

- [VERSIONING.md](./VERSIONING.md) - Full versioning guide
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Semantic Versioning](https://semver.org/)
