# aliases-cli Documentation

Developer workspace management for your shell.

## Contents

### User Guide
- [Installation](user-guide/installation.md) — install with uv, first-time setup, updating
- [Config Sync](user-guide/config-sync.md) — sync config across machines

### Reference
- [Commands](reference/commands.md) — full command reference
- [Configuration](reference/configuration.md) — all config keys and defaults

### Development
- [Architecture](development/architecture.md) — module design and data flow
- [Building](development/building.md) — development setup and running locally
- [Testing](development/testing.md) — running and writing tests

### Integrations
- [Bash Integration](integrations/bash-integration.md) — shell functions and prompt setup
- [Bash Aliases](integrations/bash-aliases.md) — bundled alias files

## Quick Start

```bash
uv tool install git+https://github.com/bbauer0706/aliases
aliases-cli setup
```

Restart your shell (or `source ~/.bash_aliases`) and you are ready.
