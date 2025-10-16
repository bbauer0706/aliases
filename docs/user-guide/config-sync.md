# Configuration Sync

Sync your aliases-cli configuration across multiple machines using simple HTTP fetch.

## Why Use Config Sync?

- **Consistency**: Keep settings identical across all your development machines
- **Easy Setup**: Just provide HTTP URLs to your config files
- **Team Distribution**: Share configurations across teams easily
- **Simple & Reliable**: No complex setup, just HTTP fetch with curl

## Quick Start

### 1. Host Your Config Files

Upload your `config.json` (and optionally `todos.json`) to a web server or Git repository:

**Option A: GitHub (Recommended)**
```bash
# 1. Create a private GitHub repo: github.com/yourusername/aliases-config
# 2. Push your config.json to the repo
# 3. Get raw file URL: https://raw.githubusercontent.com/yourusername/aliases-config/main/config.json
```

**Option B: Web Server**
```bash
# Upload config.json to your web server
# URL: https://example.com/configs/aliases/config.json
```

### 2. Setup Sync on Each Machine

```bash
# Setup with just config file
aliases-cli config sync setup https://raw.githubusercontent.com/you/aliases-config/main/config.json

# Or include todos file
aliases-cli config sync setup \
  https://raw.githubusercontent.com/you/aliases-config/main/config.json \
  https://raw.githubusercontent.com/you/aliases-config/main/todos.json
```

### 3. Pull Config

```bash
# Fetch latest configuration
aliases-cli config sync pull
```

### 4. Enable Auto-Sync (Optional)

Edit your config to enable automatic syncing:

```bash
aliases-cli config edit
```

Then modify the sync section:
```json
{
  "sync": {
    "enabled": true,
    "auto_sync": {
      "enabled": true,
      "interval": 3600
    },
    "config_file_url": "https://...",
    "todo_file_url": ""
  }
}
```

## Configuration

### Sync Settings

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `enabled` | boolean | `false` | Enable/disable config sync |
| `config_file_url` | string | `""` | HTTP URL to remote config.json |
| `todo_file_url` | string | `""` | HTTP URL to remote todos.json (optional) |
| `auto_sync.enabled` | boolean | `false` | Auto-sync on startup |
| `auto_sync.interval` | integer | `86400` | Seconds between syncs (default: 24 hours) |
| `last_sync` | integer | `0` | Unix timestamp of last sync (auto-managed) |

Example configuration:
```json
{
  "sync": {
    "enabled": true,
    "auto_sync": {
      "enabled": true,
      "interval": 3600
    },
    "last_sync": 0,
    "config_file_url": "https://raw.githubusercontent.com/user/config/main/config.json",
    "todo_file_url": "https://raw.githubusercontent.com/user/config/main/todos.json"
  }
}
```

## Common Workflows

### Daily Usage (Auto-Sync Enabled)

When auto-sync is enabled, config is automatically pulled on startup if the sync interval has passed:

```bash
# Just use aliases-cli normally
c myproject
```

Config is silently fetched in the background when needed.

### Manual Sync

```bash
# Check current sync status
aliases-cli config sync status

# Pull latest config
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
  Config file URL: https://raw.githubusercontent.com/user/repo/main/config.json
  Todo file URL: https://raw.githubusercontent.com/user/repo/main/todos.json
  Auto-sync enabled: yes
  Auto-sync interval: 3600 seconds
  Last sync: Tue Oct  7 09:15:32 2025
  Time since last sync: 1800 seconds
```

### Update Remote Config

To update the config that others will fetch:

```bash
# 1. Make your changes locally
aliases-cli config set general.editor vim

# 2. Copy your config to the remote location
cp ~/.config/aliases-cli/config.json /path/to/your/git/repo/
cd /path/to/your/git/repo
git add config.json
git commit -m "Update config"
git push

# 3. Others will get the update on next sync pull
```

## Hosting Options

### GitHub (Recommended)

**Advantages:**
- Free private repositories
- Easy to share within teams
- Version control included
- Raw file URLs are simple

**Setup:**
```bash
# 1. Create private repo: github.com/username/aliases-config
# 2. Upload config.json
# 3. Get raw URL:
#    https://raw.githubusercontent.com/username/aliases-config/main/config.json
# 4. Setup sync:
aliases-cli config sync setup https://raw.githubusercontent.com/username/aliases-config/main/config.json
```

### GitLab

Similar to GitHub:
```
https://gitlab.com/username/aliases-config/-/raw/main/config.json
```

### Your Own Web Server

```bash
# Upload config.json to your server
scp ~/.config/aliases-cli/config.json user@server:/var/www/html/config/

# Setup sync
aliases-cli config sync setup https://your-server.com/config/config.json
```

### Internal File Server

```bash
# For organization-wide config distribution
aliases-cli config sync setup https://intranet.company.com/tools/aliases/config.json
```

## Migration from Old Sync Format

If you were using the old sync system with git/rsync/file methods, your config will be automatically migrated on next startup:

**Old format:**
```json
{
  "sync": {
    "enabled": true,
    "remote_url": "git@github.com:user/config.git",
    "method": "git",
    "auto_sync": true,
    "sync_interval": 86400
  }
}
```

**New format (auto-migrated):**
```json
{
  "sync": {
    "enabled": true,
    "auto_sync": {
      "enabled": true,
      "interval": 86400
    },
    "config_file_url": "",
    "todo_file_url": ""
  }
}
```

Note: `remote_url` will be moved to `config_file_url` if it's an HTTP URL.

## Troubleshooting

### Connection Issues

```bash
# Test URL manually
curl -f -s https://raw.githubusercontent.com/user/repo/main/config.json

# Check network connectivity
ping raw.githubusercontent.com
```

### Sync Fails

Check sync status:
```bash
aliases-cli config sync status
```

Try manual sync to see errors:
```bash
aliases-cli config sync pull
```

### Invalid JSON After Sync

If the fetched config is invalid:
```bash
# Validate downloaded config
python3 -m json.tool ~/.config/aliases-cli/config.json

# Restore from backup
cp ~/.config/aliases-cli/config.json.backup ~/.config/aliases-cli/config.json
```

### Rate Limiting

If using GitHub raw URLs, you might hit rate limits:
- Use authenticated requests (create a personal access token)
- Use longer sync intervals
- Host on your own server for unlimited access

## Best Practices

1. **Use Private Repositories**: Never expose your config publicly
2. **Enable Auto-Sync**: Keep configs synchronized automatically
3. **Reasonable Intervals**: Use 1-6 hour intervals for active use
4. **Test URLs First**: Verify URLs work with curl before setting up sync
5. **Keep Todos Local**: Only sync todos if needed (it's optional)
6. **Version Control**: Use Git hosting for automatic versioning

## Security Considerations

### What's Safe to Sync

- Editor preferences
- Color settings
- Port configurations
- Project mappings (if not sensitive)

### What to Keep Local

- API keys or tokens (use environment variables instead)
- Machine-specific paths
- Sensitive project information

### Repository Security

- Use private repositories only
- Enable two-factor authentication
- Use HTTPS URLs for GitHub (no SSH keys needed)
- Audit repository access regularly

## Examples

### Example 1: Personal Configuration

```bash
# Machine 1
aliases-cli config sync setup https://raw.githubusercontent.com/me/aliases/main/config.json

# Enable auto-sync in config.json
aliases-cli config edit
# Set auto_sync.enabled to true

# Pull config
aliases-cli config sync pull
```

### Example 2: Team Configuration

**Team lead hosts config on GitHub:**
```bash
# Create repo: github.com/company/aliases-cli-config
# Upload team config.json
```

**Each team member:**
```bash
# Setup sync with team config
aliases-cli config sync setup https://raw.githubusercontent.com/company/aliases-cli-config/main/config.json

# Enable hourly checks
aliases-cli config edit
# Set: "auto_sync": { "enabled": true, "interval": 3600 }

# Initial pull
aliases-cli config sync pull
```

### Example 3: Multiple Configs

```bash
# Work config
aliases-cli config sync setup https://company.com/configs/aliases-work.json

# Or personal config
aliases-cli config sync setup https://raw.githubusercontent.com/me/aliases/main/config.json

# Switch by re-running setup with different URL
```

## Comparison with Old Sync

| Feature | Old (Git/Rsync/File) | New (HTTP-Only) |
|---------|---------------------|-----------------|
| **Push** | ✅ Supported | ❌ Not supported (manual upload) |
| **Pull** | ✅ Supported | ✅ Supported |
| **Auto-sync** | ✅ Supported | ✅ Supported |
| **Setup** | Complex (SSH keys, git, rsync) | Simple (just a URL) |
| **Code** | ~220 lines | ~60 lines |
| **Best for** | Advanced users, two-way sync | Simple distribution, teams |

## See Also

- [Configuration Reference](../reference/configuration.md)
- [Commands Reference](../reference/commands.md)
- [Installation Guide](installation.md)
