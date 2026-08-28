# Versioning

aliases follows [Semantic Versioning](https://semver.org/) (MAJOR.MINOR.PATCH).

## Automated Releases

Pushes to `main` are released by `.github/workflows/ci-release.yml` after tests
and lint pass. Commitizen derives the next version from conventional commits:

- `fix:` creates a patch release
- `feat:` creates a minor release
- `BREAKING CHANGE:` creates a major release

The workflow updates `pyproject.toml` and `CHANGELOG.md`, creates the version
commit and tag, pushes both, and creates a GitHub Release. Other commit types do
not create a release when they contain no releasable change.

The release job uses the `RELEASE_TOKEN` repository secret so its atomic version
commit and tag push can use the repository administrator ruleset bypass. The
token requires `repo` scope.

Preview the next release locally with `uv run cz bump --dry-run`.

## Version History

| Version | Changes |
|---------|---------|
| 2.0.0 | Full rewrite in Python; uv-installable; OS keychain secrets; `aliases setup` |
| 1.x | Original C++ implementation |

## Upgrade from 1.x

1. Remove old binary and install scripts if present
2. `uv tool install git+https://github.com/bbauer0706/aliases`
3. `aliases setup`

**Secrets:** the old AES-256-GCM `.secrets.enc` file is not migrated automatically.
Re-add your secrets with `aliases secrets set`.

## Version String

The package version is stored in `pyproject.toml` and exposed at runtime via
`importlib.metadata`. In a dev checkout without installed package metadata it
shows `dev`.
