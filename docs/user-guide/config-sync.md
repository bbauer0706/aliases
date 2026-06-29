# Configuration Sync

Sync `~/.config/aliases/config.json` across multiple machines using git,
rsync, a network file path, or an HTTP endpoint.

## Supported Methods

| Method | Push | Pull | Best For |
|--------|------|------|----------|
| `git` | ✅ | ✅ | Private repo, version history, most machines |
| `rsync` | ✅ | ✅ | Fast sync over SSH / network |
| `file` | ✅ | ✅ | Dropbox, Google Drive, NFS mount |
| `http` | ❌ | ✅ | Read-only team distribution |

## Quick Setup

### 1. Configure (once per machine)

```bash
# git (recommended)
aliases config sync setup git@github.com:you/aliases-config.git

# rsync
aliases config sync setup user@host:/path/to/config rsync

# file / Dropbox
aliases config sync setup ~/Dropbox/aliases-config file

# HTTP (read-only)
aliases config sync setup https://example.com/aliases-config http
```

### 2. First machine — push

```bash
aliases config sync push
```

### 3. Other machines — pull

```bash
aliases config sync pull
```

### 4. Enable auto-sync (optional)

```bash
aliases config set sync.auto_sync true
aliases config set sync.sync_interval 3600  # every hour
```

When enabled, a pull is triggered silently at startup if the interval has elapsed.

## Status

```bash
aliases config sync status
```

## What Is Synced

Only `config.json` is synced. The secrets name index
(`secrets_names.json`) and the keychain entries are intentionally
**never synced** — secrets are machine-local.

## Troubleshooting

**Git auth failures:** ensure your SSH key is loaded (`ssh-add ~/.ssh/id_ed25519`).

**Conflicts in git cache:** delete the local clone and re-pull:

```bash
rm -rf ~/.config/aliases/cache/sync
aliases config sync pull
```

**HTTP 404 / connection errors:** run `aliases config sync status` to confirm
the URL is correct, then try a manual pull to see the full error message.
