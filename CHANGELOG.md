## v2.2.2 (2026-06-15)

### Fix

- **cz**: replace invalid push=true with post_bump_hooks for auto push

## v2.2.1 (2026-06-15)

### Fix

- **cz**: enable auto-push and fix version_files pattern

## v2.2.0 (2026-06-15)

### Feat

- **update**: add aliases-cli update command

## v2.1.1 (2026-06-15)

### Perf

- fix 5 additional performance issues across shell and Python

## v2.1.0 (2026-06-15)

### Feat

- add AWS profile switching helper script with pickaws function
- add user and host replacements to config and implement in pwd command
- add mcv (mvn clean verify)

### Fix

- update existing config with new defaults during setup
- syncrotess prjupdate != prjbuild...
- promt + rm todo entirely

### Refactor

- streamline data file discovery for setup command

### Perf

- **prompt**: reduce PS1 to single subprocess call with directory cache

## v1.10.0 (2026-04-17)

### Feat

- aliases-cli compleation
- secretes cmd

## v1.9.0 (2026-04-15)

### Feat

- syncrotess ali

## v1.8.0 (2026-04-15)

### Feat

- add pwd formatter module; remove todo TUI

## v1.7.0 (2026-04-10)

### Feat

- add trigger-ci alias

## v1.6.0 (2025-12-05)

### Feat

- npm wrapper to enforce --ignore-scripts flag

## v1.5.0 (2025-10-29)

### Feat

- set host from outside
- add multiple workspace directories and ignore patterns support

### Fix

- test not mod accual config files

## v1.4.1 (2025-10-09)

### Fix

- only sync on todo and config cmds

## v1.4.0 (2025-10-08)

### Feat

- testing

## v1.3.0 (2025-10-08)

### Feat

- tui create mode enhancements + shared todo config

### Fix

- git identity missing

## v1.2.0 (2025-10-07)

### Feat

- npm-clean

## v1.1.1 (2025-10-07)

### Fix

- test CI/CD with branch protection enabled
- test CI/CD with branch protection enabled

## v1.1.0 (2025-10-07)

### Feat

- add automated CI/CD workflows
- config
- new stuff
- clear cmd not found hanlder
- clear
- todo v1.1
- todo v1
- git-reset-last
- conditional pnpm alias
- tab completion for c
- enhance code navigator with composite project support
- some clean up
- project env port selection based on user name

### Fix

- update all workflow actions to v4 and add permissions
- update upload-artifact to v4
- build
- c completion deduplication
- manual tab completion setup
- is server dir compared absolute to relative path
- shell wrapper to accually set the env in the shell
- call the alias cli bin directly when sourcing
- stuff göll :)
- project selection on shell init
- type in code alias
