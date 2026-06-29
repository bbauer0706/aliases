# Bash Aliases

`aliases setup` installs these alias files into
`~/.config/aliases/bash_aliases/`. They are sourced in alphabetical order
by `~/.bash_aliases`.

---

## `aws.ali.sh`

AWS profile switching helpers. Requires `aws-cli` and `fzf`.

| Function | Description |
|----------|-------------|
| `pickaws` | Interactively select an AWS profile with fzf, set `AWS_PROFILE`, and verify or refresh SSO credentials |

### `pickaws` Behaviour

1. Lists all profiles via `aws configure list-profiles` and lets you pick one with fzf
2. Exports `AWS_PROFILE` for the current shell session
3. Runs `aws sts get-caller-identity` to check credential validity
4. Prints `[OK] Aktiv` (green) if credentials are valid; otherwise prints `[WARN] SSO Login erforderlich` (yellow) and runs `aws sso login`

---

## `basic.ali.sh`

General navigation and utility aliases.

| Alias / Function | Command |
|-----------------|---------|
| `la` | `ls -alh` |
| `..` | `cd ..` |
| `...` | `cd ../..` |
| `~` | `cd ~` |
| `kp <port>` | `fuser -k <port>/tcp` — kill process on port |
| `tsc` | `npx tsc --noEmit` |
| `show_env` | `aliases env --show` |

---

## `git.ali.sh`

| Alias / Function | Description |
|-----------------|-------------|
| `reset-soft` | `git reset --soft HEAD~1` |
| `git-rm` | `git rm --cached` |
| `trigger-ci` | Empty commit + push to trigger CI |
| `rebase` | Safe rebase onto `origin/main` with pre-flight checks |

### `rebase` Safety Checks

Before rebasing, `rebase` verifies:

1. Inside a git repository
2. Not on `main` or `master`
3. No uncommitted changes
4. `origin` remote exists

Then fetches `origin/main`, rebases, and prompts before force-pushing
(uses `--force-with-lease` to avoid clobbering concurrent pushes).

---

## `npm.ali.sh`

Script shortcuts and npm wrappers.

| Alias | Expands To |
|-------|-----------|
| `dev` | `npm run dev` |
| `gen` | `npm run gen` |
| `build` | `npm run build` |
| `blocal` | `npm run build:local` |
| `bstatic` | `npm run build:static` |
| `vit` | `npm run vitest` |
| `jes` | `npm run test:watch` |
| `story` | `npm run start:storybook` |
| `npmi` | `npm i` |
| `lint` | `npm run lint` |
| `pipeline` | `npm run pipeline:dry-run` |
| `npm-clean` | Remove `.next`, `node_modules`, `package-lock.json`, then `npm i` |

If `pnpm` is installed, `npm` is aliased to `pnpm`. Otherwise an `npm()`
wrapper adds `--ignore-scripts` to install/update/ci/rebuild/add/remove
commands automatically.

---

## `maven.ali.sh`

| Alias | Command |
|-------|---------|
| `mcp` | `mvn clean package` |
| `mcv` | `mvn clean verify` |

---

## `clear.ali.sh`

A compact set of typo aliases for `clear` (common keyboard misfires).
Defined as a loop rather than 100+ individual lines.

Examples: `clera`, `cler`, `cleear`, `claer`, `cleare`, `clrea`, …

---

## `syncrotess.ali.sh`

Integration with the internal Syncrotess/Autobuild system.
Only useful on machines that have the `projectbuilds` scripts installed.

Searches for the scripts directory in this order:

1. `$PRJBUILDS_DIR`
2. `/nethome/svc-gb20-road/syncrotess/scripts`
3. `~/syncrotess/scripts`
4. `~/workspaces/projectbuilds`

If none found, all commands silently do nothing.

| Command | Description |
|---------|-------------|
| `prjsel <version>` | Select version (sets `$PRJBUILDS_VERSION`) |
| `prjserver [ver]` | Update server component |
| `prjsst [ver]` | Update interfaces |
| `prjwebtools [ver]` | Update web tools |
| `prjupdate [ver]` | Server + interfaces + webtools |
| `prjstart [ver]` | Start system |
| `prjstop [ver]` | Stop system |
| `prjtest [ver]` | Run test cases |
| `prjbuild <ver> <cmd>` | Raw wrapper |
| `prjscripts` | List available product scripts |
| `prjpull` | `git pull` the scripts repo |
| `relink <jar>` | Repoint `$INSTROOT/bin/<prefix>_Actual.jar` symlink |
