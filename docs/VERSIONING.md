# Versioning

aliases-cli follows [Semantic Versioning](https://semver.org/) (MAJOR.MINOR.PATCH).

## Version History

| Version | Changes |
|---------|---------|
| 2.0.0 | Full rewrite in Python; uv-installable; OS keychain secrets; `aliases-cli setup` |
| 1.x | Original C++ implementation |

## Upgrade from 1.x

1. Remove old binary and install scripts if present
2. `uv tool install git+https://github.com/bbauer0706/aliases`
3. `aliases-cli setup`

**Secrets:** the old AES-256-GCM `.secrets.enc` file is not migrated automatically.
Re-add your secrets with `aliases-cli secrets set`.

## Version String

The version is derived from the latest git tag at build time
(via `importlib.metadata`). In a dev checkout it shows `dev`.
