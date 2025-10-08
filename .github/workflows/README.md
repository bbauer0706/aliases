# GitHub Actions Workflows

This directory contains automated CI/CD workflows for aliases-cli.

## Available Workflows

### 1. CI/CD Pipeline (`ci-cd.yml`) - **PRIMARY**

**Trigger:** Every push to `main` or `develop` branch
**What it does:**
- ✅ Builds and tests the binary
- 🔍 Analyzes commit messages (conventional commits)
- 📊 Determines version bump type automatically
- 🏷️ Creates version tag (e.g., v1.2.0)
- 📦 Creates GitHub release with binary and release notes

**Conventional Commit Detection:**
- `feat:` or `feat!:` → Minor version bump (1.0.0 → 1.1.0)
- `fix:`, `docs:`, `chore:` → Patch version bump (1.0.0 → 1.0.1)
- `BREAKING CHANGE:` or `!` → Major version bump (1.0.0 → 2.0.0)

**Example:**
```bash
git commit -m "feat: add config sync support"
git push origin main
# Automatically creates v1.1.0 release
```

### 2. Manual Release (`manual-release.yml`)

**Trigger:** GitHub Actions tab → Run workflow  
**What it does:**
- Choose version bump type (patch/minor/major)
- Manually trigger a release
- Optionally mark as pre-release

**Use when:**
- You want control over the version bump
- Creating beta/RC releases
- Need to release without specific commit patterns

**How to use:**
1. Go to Actions tab
2. Select "Manual Release"
3. Click "Run workflow"
4. Choose version bump type
5. Submit

### 3. Hotfix Release (`hotfix.yml`)

**Trigger:** GitHub Actions tab → Run workflow  
**What it does:**
- Quick patch version bump
- Immediate release
- Includes hotfix description in release notes

**Use when:**
- Emergency bug fixes
- Security patches
- Critical issues

**How to use:**
1. Fix the bug and commit
2. Go to Actions tab
3. Select "Hotfix Release"
4. Enter hotfix description
5. Run workflow

### 4. Release (Tag-triggered) (`release.yml`)

**Trigger:** Manual tag push  
**What it does:**
- Builds binary when you manually create a tag
- Creates GitHub release

**Use when:**
- You manage versions manually
- Legacy workflow compatibility

## Workflow Comparison

| Workflow | Trigger | Version Control | Best For |
|----------|---------|-----------------|----------|
| **CI/CD** (Recommended) | Push to main | Automatic | Normal development |
| Manual Release | GitHub UI | Manual choice | Controlled releases |
| Hotfix | GitHub UI | Automatic patch | Emergency fixes |
| Tag Release | Manual tag | Manual | Legacy compatibility |

## Complete Development Flow

### Scenario 1: New Feature (Automatic)

```bash
# 1. Develop feature
git checkout -b feature/new-sync
# ... make changes ...

# 2. Commit with conventional commit
git commit -m "feat(config): add HTTP sync support"

# 3. Merge to main
git checkout main
git merge feature/new-sync

# 4. Push
git push origin main

# ✨ CI/CD workflow automatically:
#    - Detects "feat:" = minor version
#    - Bumps 1.0.0 → 1.1.0
#    - Builds binary
#    - Creates tag v1.1.0
#    - Creates GitHub release
```

### Scenario 2: Bug Fix (Automatic)

```bash
# 1. Fix bug
git commit -m "fix: resolve authentication timeout"
git push origin main

# ✨ Automatically creates v1.0.1 release
```

### Scenario 3: Breaking Change (Automatic)

```bash
# 1. Make breaking change
git commit -m "feat!: redesign configuration structure

BREAKING CHANGE: Config format has changed"
git push origin main

# ✨ Automatically creates v2.0.0 release
```

### Scenario 4: Controlled Release (Manual)

```bash
# 1. Make changes and push
git commit -m "Various improvements"
git push origin main

# 2. Go to GitHub Actions
# 3. Run "Manual Release" workflow
# 4. Choose "minor" or "patch"
# 5. Release created
```

### Scenario 5: Emergency Hotfix

```bash
# 1. Fix critical bug
git commit -m "fix: critical security vulnerability"
git push origin main

# 2. Go to GitHub Actions
# 3. Run "Hotfix Release" workflow
# 4. Enter: "Security patch for CVE-2025-XXXX"
# 5. Immediate patch release created
```

## Conventional Commits Standard

Use these commit message prefixes:

| Prefix | Version Bump | Example |
|--------|--------------|---------|
| `feat:` | Minor | `feat: add sync support` |
| `feat!:` | Major | `feat!: redesign API` |
| `fix:` | Patch | `fix: resolve timeout issue` |
| `docs:` | Patch | `docs: update README` |
| `chore:` | Patch | `chore: update dependencies` |
| `refactor:` | Patch | `refactor: improve code structure` |
| `perf:` | Patch | `perf: optimize search algorithm` |
| `test:` | Patch | `test: add unit tests` |

**Breaking Changes:**
Add `!` after type or `BREAKING CHANGE:` in commit body:
```bash
git commit -m "feat!: change config format"
# or
git commit -m "feat: new API

BREAKING CHANGE: Old API endpoints removed"
```

## Skip CI

To push without triggering workflows:
```bash
git commit -m "docs: minor typo fix [skip ci]"
git push origin main
```

## GitHub Actions Secrets

No secrets needed! Uses built-in `GITHUB_TOKEN`.

## Monitoring Workflows

- **View runs:** Repository → Actions tab
- **Check status:** Each commit shows ✅ or ❌
- **Download artifacts:** Click workflow run → Artifacts section
- **Cancel workflow:** Click running workflow → Cancel

## Troubleshooting

### Workflow didn't trigger
- Check commit is on `main` branch
- Verify workflow file syntax
- Check Actions are enabled (Settings → Actions)

### Version bump incorrect
- Check commit message format
- Use conventional commits prefix
- Review "Determine version bump" step logs

### Release creation failed
- Check GITHUB_TOKEN permissions
- Ensure tag doesn't already exist
- Review workflow logs for errors

### Want to change version manually
- Use "Manual Release" workflow
- Manually create and push a git tag (see Tag Release workflow)

## Best Practices

1. **Use conventional commits** for automatic versioning
2. **Let CI/CD handle releases** on main branch
3. **Use Manual Release** for controlled releases
4. **Use Hotfix** only for emergencies
5. **Review changelog** in releases
6. **Test locally** before pushing to main

## Migration from Manual Versioning

Old way (deprecated):
```bash
# Manual version bumping and release creation
echo "1.1.0" > VERSION
sed -i 's/VERSION = "[^"]*"/VERSION = "1.1.0"/' src/main.cpp
vim CHANGELOG.md
git commit -m "chore: bump version"
git tag -a v1.1.0 -m "Release v1.1.0"
git push origin v1.1.0
```

New way (automated):
```bash
git commit -m "feat: new feature"
git push origin main
# Done! CI/CD handles versioning ✨
```

## Files Modified by Workflows

Workflows automatically create:
- Git tags - Version tags (e.g., v1.2.0)
- GitHub Releases - Release pages with binaries

**Note:** Version is stored only in git tags. The binary gets its version from the tag during build time.

## Additional Resources

- [Conventional Commits](https://www.conventionalcommits.org/)
- [Semantic Versioning](https://semver.org/)
- [GitHub Actions Documentation](https://docs.github.com/actions)

---

**Setup Complete!** Push to main and watch the magic happen. 🚀
