# Configuration Sync

Sync your aliases-cli configuration across multiple machines using various storage backends.

## Why Use Config Sync?

- **Consistency**: Keep settings identical across all your development machines
- **Backup**: Automatically backup your configuration to remote storage
- **Easy Setup**: Configure once, sync everywhere
- **Multiple Methods**: Choose from git, rsync, file copy, or HTTP

## Quick Start

### 1. Setup Sync (One-Time Per Machine)

Choose your preferred sync method:

#### Git Repository (Recommended)

```bash
# Create a private git repo (GitHub, GitLab, etc.)
# Then setup sync
aliases-cli config sync setup git@github.com:yourusername/aliases-config.git git

# Or use HTTPS
aliases-cli config sync setup https://github.com:yourusername/aliases-config.git git
```

#### Network Drive / NFS

```bash
aliases-cli config sync setup /mnt/network-drive/aliases-config file
```

#### Rsync

```bash
aliases-cli config sync setup user@server:/path/to/config rsync
```

#### HTTP Endpoint (Read-Only)

```bash
aliases-cli config sync setup https://example.com/my-config http
```

### 2. First Machine: Push Your Config

```bash
# Push your current configuration to remote
aliases-cli config sync push
```

### 3. Other Machines: Pull Config

```bash
# Pull configuration from remote
aliases-cli config sync pull
```

### 4. Enable Auto-Sync (Optional)

```bash
# Enable automatic syncing on startup
aliases-cli config set sync.auto_sync true

# Set sync interval (default: 24 hours / 86400 seconds)
aliases-cli config set sync.sync_interval 3600  # 1 hour
```

## Sync Methods Comparison

| Method | Push | Pull | Auto-Sync | Best For |
|--------|------|------|-----------|----------|
| **Git** | ✅ | ✅ | ✅ | Version control, multiple machines, conflict resolution |
| **Rsync** | ✅ | ✅ | ✅ | Network drives, fast sync, direct file access |
| **File** | ✅ | ✅ | ✅ | Local network shares, Dropbox/Google Drive |
| **HTTP** | ❌ | ✅ | ✅ | Read-only config distribution, CI/CD |

## Detailed Setup Guides

### Git Method (Recommended)

**Advantages:**
- Version history
- Conflict resolution
- Easy collaboration
- Works anywhere

**Setup:**

```bash
# 1. Create a private GitHub/GitLab repo
#    (e.g., https://github.com/username/aliases-config)

# 2. Setup sync
aliases-cli config sync setup git@github.com:username/aliases-config.git git

# 3. Push your current config
aliases-cli config sync push

# 4. On other machines, pull the config
aliases-cli config sync pull

# 5. Enable auto-sync
aliases-cli config set sync.auto_sync true
```

**What Gets Synced:**
- `config.json` - All settings including project mappings

**What Doesn't Get Synced:**
- `todos.json` - Kept local (task lists are machine-specific)
- `cache/` - Temporary files

### Rsync Method

**Advantages:**
- Fast incremental syncing
- Works over SSH
- No git needed

**Setup:**

```bash
# 1. Setup rsync sync
aliases-cli config sync setup user@server:/path/to/config rsync

# 2. Push config
aliases-cli config sync push

# 3. Pull from other machines
aliases-cli config sync pull
```

### File Copy Method

**Advantages:**
- Simple file system copies
- Works with Dropbox, Google Drive, OneDrive
- No external tools needed

**Setup:**

```bash
# Using a network drive
aliases-cli config sync setup /mnt/network/aliases-config file

# Using Dropbox
aliases-cli config sync setup ~/Dropbox/aliases-config file

# Using Google Drive
aliases-cli config sync setup ~/Google\ Drive/aliases-config file
```

### HTTP Method (Read-Only)

**Advantages:**
- Centralized config distribution
- Good for teams/organizations
- Read-only prevents accidental changes

**Setup:**

```bash
# Host config.json on a web server
# Then pull from machines
aliases-cli config sync setup https://config.example.com/aliases http
aliases-cli config sync pull
```

**Note:** HTTP method only supports pull, not push.

## Common Workflows

### Daily Usage (Auto-Sync Enabled)

When auto-sync is enabled, config is automatically pulled on startup if the sync interval has passed:

```bash
# Just use aliases-cli normally
c myproject
```

Auto-sync runs silently in the background.

### Manual Sync After Changes

```bash
# After making changes locally
aliases-cli config set general.editor vim

# Push to remote
aliases-cli config sync push

# On other machines
aliases-cli config sync pull
```

### Check Sync Status

```bash
aliases-cli config sync status
```

Output:
```
Sync Configuration:
  Enabled: yes
  Remote URL: git@github.com:user/aliases-config.git
  Method: git
  Auto-sync: yes
  Sync interval: 86400 seconds
  Last sync: Tue Oct  7 09:15:32 2025
  Time since last sync: 3600 seconds
```

### Disable Sync

```bash
aliases-cli config set sync.enabled false
```

### Change Sync URL

```bash
# Setup with new URL
aliases-cli config sync setup new-url@server:/path git
```

## Configuration Options

All sync settings are in the `sync` section of `config.json`:

```json
{
  "sync": {
    "enabled": true,
    "remote_url": "git@github.com:user/aliases-config.git",
    "auto_sync": true,
    "sync_interval": 86400,
    "last_sync": 1696676132,
    "method": "git"
  }
}
```

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `enabled` | boolean | `false` | Enable/disable config sync |
| `remote_url` | string | `""` | Remote storage location |
| `auto_sync` | boolean | `false` | Auto-sync on startup |
| `sync_interval` | integer | `86400` | Seconds between auto-syncs (24 hours) |
| `last_sync` | integer | `0` | Unix timestamp of last sync |
| `method` | string | `"git"` | Sync method: `git`, `rsync`, `file`, `http` |

## Troubleshooting

### Git Authentication Issues

```bash
# Use SSH keys (recommended)
ssh-add ~/.ssh/id_rsa

# Or use HTTPS with credential caching
git config --global credential.helper cache
```

### Sync Fails Silently

Check sync status:
```bash
aliases-cli config sync status
```

Try manual sync to see errors:
```bash
aliases-cli config sync pull
```

### Conflicts in Git Repo

```bash
# Reset local git cache and re-sync
rm -rf ~/.config/aliases-cli/cache/sync
aliases-cli config sync pull
```

### Different Configs for Different Machines

Config sync allows machine-specific settings to coexist. Consider keeping different workspace directories per machine while syncing other settings, or use sync selectively with git branches for machine-specific configs.

### Disable Auto-Sync Temporarily

```bash
aliases-cli config set sync.auto_sync false
```

## Best Practices

1. **Use Git for Most Cases**: Provides version history and conflict resolution
2. **Private Repos Only**: Never use public repos for your config
3. **Enable Auto-Sync**: Keep configs in sync automatically
4. **Short Sync Intervals**: Use 1-4 hour intervals for active development
5. **Manual Push After Big Changes**: Don't rely only on auto-sync for important changes
6. **Keep Todos Local**: The system intentionally doesn't sync todos.json

## Security Considerations

### What's Safe to Sync

- Editor preferences
- Color settings
- Port configurations
- Project mappings (if not sensitive)

### What to Keep Local

- API keys or tokens (use environment variables instead)
- Machine-specific paths that vary
- Sensitive project information

### Repository Security

- Use private repositories only
- Enable two-factor authentication
- Use SSH keys with passphrases
- Audit repository access regularly

## Examples

### Example 1: Two Development Machines

**Machine 1 (Work):**
```bash
aliases-cli config sync setup git@github.com:me/aliases-config.git
aliases-cli config sync push
aliases-cli config set sync.auto_sync true
```

**Machine 2 (Home):**
```bash
aliases-cli config sync setup git@github.com:me/aliases-config.git
aliases-cli config sync pull
aliases-cli config set sync.auto_sync true
```

Now both machines stay in sync automatically!

### Example 2: Team Configuration

**Team Lead:**
```bash
# Setup HTTP distribution
# Host config.json at https://team.example.com/config/

# Each team member:
aliases-cli config sync setup https://team.example.com/config http
aliases-cli config sync pull
aliases-cli config set sync.sync_interval 3600  # Check hourly
aliases-cli config set sync.auto_sync true
```

### Example 3: Dropbox Sync

```bash
# All machines with Dropbox
aliases-cli config sync setup ~/Dropbox/aliases-config file
aliases-cli config set sync.auto_sync true
aliases-cli config set sync.sync_interval 1800  # 30 minutes

# Dropbox handles the actual syncing
```

## See Also

- [Configuration Reference](../reference/configuration.md)
- [Commands Reference](../reference/commands.md)
- [Installation Guide](installation.md)
