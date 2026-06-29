# Commands Reference

All commands are available as `aliases <command>` or via the short bash
wrappers set up by `aliases setup`.

---

## `setup`

Set up shell integration after installation.

```
aliases setup [OPTIONS]
```

| Option | Description |
|--------|-------------|
| `--force`, `-f` | Overwrite existing files without prompting |
| `--update`, `-u` | Update shell/alias files only; skip `.bash_aliases` and `.bashrc` |

Run once after `uv tool install`. Run again with `--update` after upgrading.

---

## `code` / `c`

Open projects in VS Code. Falls back to `code <args>` when no project matches.

```
aliases code [ARGS...]
aliases c    [ARGS...]   # short alias
c                [ARGS...]   # bash alias → aliases code
```

### Syntax

| Invocation | Effect |
|------------|--------|
| `c` | Open home directory |
| `c <project>` | Open project root |
| `c <project> s` | Open server component |
| `c <project> w` | Open web component |
| `c <project> server` | Same as `s` |
| `c <project> web` | Same as `w` |
| `c <project>s` | Suffix shorthand for server |
| `c <project>w` | Suffix shorthand for web |
| `c <project>sw` | Open server then web |
| `c <project>ws` | Open web then server |
| `c <project>[sw]` | Bracket notation — open both |
| `c proj1 proj2` | Open multiple projects |
| `c <path>` | Fallback: `code <path>` |

### Behavior

Project names can be display names (shortcuts) or full directory names.
Matching is case-insensitive. If no project matches, all args are passed
directly to `code`.

Config keys: `code.reuse_window`, `code.vscode_flags`, `code.fallback_behavior`.

---

## `env`

Discover the current project from `$PWD` and output shell export statements.

```
aliases env [OPTIONS]
```

Intended to be used via `eval`:

```bash
eval "$(aliases env)"
```

The bash wrapper `project_env` (installed by `setup`) does this for you.

### Options

| Option | Default | Description |
|--------|---------|-------------|
| `-e ENV` | `dev` | Environment profile (dev/staging/prod/…) |
| `-p PORT` | config `env.base_port` | Starting port to scan from |
| `-i true&#124;false` | `true` | GraphQL introspection |
| `-t MODE` | `plain` | Transfer mode (plain/compressed) |
| `-n` | — | No port offset: same port for WEB and GQL |
| `--show` | — | Print current env var values and exit |

### Exported Variables

| Variable | Description |
|----------|-------------|
| `PROJECT_NAME` | Detected project name |
| `PROFILE` | Environment profile |
| `WEBPORT` | Assigned web port |
| `GQLPORT` | Assigned GQL port (`WEBPORT + 1` unless `-n`) |
| `GQLNUMBEROFMAXRETRIES` | Fixed at 3 |
| `GQLINTROSPECTION` | `true` or `false` |
| `GQLTRANSFERMODE` | Transfer mode string |

### Port Assignment

Port offset is a stable hash of the project name (range 100–990, step 10).
The tool scans upward from `base_port + offset` until a free port is found.

Config keys: `env.base_port`, `env.default_env`.

---

## `config`

Read and write configuration. Config file: `~/.config/aliases/config.json`.

```
aliases config <subcommand>
```

### Subcommands

```
aliases config get <key>           # print value
aliases config set <key> <value>   # set value (type-coerced)
aliases config list [--plain]      # list all settings
aliases config reset [-y]          # reset to defaults
aliases config edit                # open in editor
aliases config path                # print config file path
```

```
aliases config sync setup <url> [method] [--config-path <path>]   # configure remote
aliases config sync pull                                            # fetch from remote
aliases config sync push                                            # push to remote
aliases config sync status                                          # show sync state
```

Keys use dot-notation: `general.editor`, `env.base_port`, etc.

See [Configuration Reference](configuration.md) for all keys.

---

## `secrets`

Manage secrets backed by the OS keychain (GNOME Keyring, macOS Keychain,
Windows Credential Manager).

```
aliases secrets <subcommand>
```

### Subcommands

```
aliases secrets set <name> [value]    # store (prompts if no value)
aliases secrets get <name>            # print value
aliases secrets list                  # list all names
aliases secrets delete <name> [-y]    # remove (also: remove, rm)
aliases secrets load [name...]        # output export statements for eval
```

The bash wrapper `secrets_load [names...]` (installed by `setup`) calls
`eval "$(aliases secrets load ...)"` to export secrets into the current shell.

Secret names must match `[A-Za-z0-9_-]`.

---

## `pwd`

Format `$PWD` using `prompt.path_replacements` rules from config.

```
aliases pwd [OPTIONS]
```

| Option | Description |
|--------|-------------|
| `--no-color` | Suppress ANSI color codes |
| `--ps1` | Wrap codes in `\001...\002` for readline safety |
| `--user-host` | Return formatted `user@host` string (with label replacements) |
| `--user-host-color` | Return only the user@host ANSI color code |
| `--full-prompt` | Return full `user@host:path` in a single call (used by `prompt.sh`) |

Used internally by `prompt.sh` to build the custom PS1.

---

## `update`

Check for a newer release on GitHub and self-update via `uv`.

```
aliases update [OPTIONS]
```

| Option | Description |
|--------|-------------|
| `--check` | Report only; exit 1 if an update is available, do not install |
| `--force` | Re-install even if already on the latest version |

### Behavior

Fetches the latest `vX.Y.Z` tag from the GitHub API, compares it to the
installed version, and runs:

```bash
uv tool install --force-reinstall git+https://github.com/bbauer0706/aliases
```

Requires `uv` on `$PATH`. Exits 1 with an error message if `uv` is missing
or GitHub is unreachable.

---

## `completion` (internal)

Data helpers for bash tab-completion. Not intended for direct use.

```
aliases completion projects           # pipe-delimited project list
aliases completion components <name>  # component suffixes for a project
aliases completion config-keys        # all dot-notation config keys
```
