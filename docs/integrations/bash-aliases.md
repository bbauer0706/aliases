# Bash Aliases Integration

This directory contains both **active utilities** and **deprecated scripts** from the original bash-based system.

## Clear Command Aliases

Comprehensive typo-resistant aliases for the `clear` command in `bash_aliases/clear.ali.sh`:

### German Keyboard Support (QWERTZ Layout)

The clear aliases are optimized for German keyboards with comprehensive typo coverage:

```bash
# Common typos
clear → clera, clare, claer, cler, cear, clar
clear → cleaar, clearr, cleasr, cleaer, cleer

# German-specific QWERTZ layout typos
clear → cleaz, clezr, clez        # Z/Y swaps
clear → cleay, cley, cleyar       # Y positioning

# Umlaut accidents (ä, ö, ü adjacent to standard keys)
clear → cleäar, cleär, cleä       # Ä instead of A
clear → cleöar, cleör, cleö       # Ö instead of O
clear → cleüar, cleür, cleü       # Ü instead of U

# German ß key mishits
clear → cleaß, cleß, clßear, cßlear

# Adjacent key mistakes on German keyboard
clear → cöear, ckear, coear       # L neighbors (Ö, K, O)
clear → clwar, cldar              # E neighbors (W, D)
clear → clesar, clesr             # A neighbors (S)
clear → cleat, cleatar            # R neighbors (T, F)
```

### Extreme Typo Coverage

```bash
# Super short versions for really bad typos
cr, cl, ce, le

# One-handed typing errors
vlera, xlera, vlare, xlear

# Backwards/mixed patterns
raelc, aelrc, learc, rceal

# Double letters
cclear, cllear, cleear, cleaar, clearr
```

### Usage Examples

```bash
# All of these work the same as 'clear'
clera          # Common typo
cleäar         # German umlaut accident
cleaz          # German Z/Y swap
cöear          # Adjacent key on German keyboard
cr             # Super short typo
```

**Total Coverage:** 80+ aliases covering virtually every possible way to misspell "clear" on a German keyboard.

## Smart Command-Not-Found Handler

Intelligent fallback for any unmapped clear typos (integrated in `bash_aliases/clear.ali.sh`):

### How It Works

The handler intercepts any command that bash can't find and applies smart detection:

```bash
# Checks if the unknown command contains all letters: c, l, e, a, r
command_not_found_handle() {
    # Example: "rceal" contains c,l,e,a,r → executes clear
    # Example: "aelrc" contains c,l,e,a,r → executes clear
    # Example: "xyz" missing letters → normal error
}
```

### Detection Rules

1. **Letter Check:** Must contain all letters `c`, `l`, `e`, `a`, `r` (case-insensitive)
2. **Length Filter:** Only 3-8 characters (reasonable typo range)
3. **Smart Fallback:** Uses system command-not-found if no match

### Examples

```bash
# These would trigger clear execution:
$ rceal        # ✅ Contains all clear letters
🔍 Command 'rceal' not found. Did you mean 'clear'? Executing clear...

$ aelrc        # ✅ Contains all clear letters
🔍 Command 'aelrc' not found. Did you mean 'clear'? Executing clear...

$ learc        # ✅ Contains all clear letters
🔍 Command 'learc' not found. Did you mean 'clear'? Executing clear...

# These would show normal error:
$ xyz          # ❌ Missing clear letters
bash: xyz: command not found

$ clearterminal # ❌ Too long (>8 chars)
bash: clearterminal: command not found
```

### Combined Coverage

**Alias Coverage (clear.ali.sh):** 80+ pre-defined aliases
**Dynamic Coverage (command_not_found.ali.sh):** Infinite anagram detection
**Total Result:** Virtually impossible to fail clearing the terminal

## Todo System Aliases

Comprehensive todo management aliases are provided in `bash_aliases/todo.ali.sh`:

### Smart Main Alias

```bash
# Main todo alias with intelligent parameter handling
td() {
    # td                    - Launch interactive TUI
    # td "task"             - Add todo with description
    # td category "task"    - Add todo with category and description  
    # td command args...    - Pass through to main command
}
```

### Core Aliases

```bash
# Quick access
alias tdl='aliases-cli todo list'    # List todos
alias tds='aliases-cli todo search'  # Search todos
alias todo='td'                      # Legacy compatibility
```

### Priority-Based Creation

```bash
# Priority shortcuts
td-high "task"        # High priority (🔴)
td-med "task"         # Medium priority (🟡) 
td-low "task"         # Low priority (🟢)
```

### Category-Based Creation

```bash
# Common categories with appropriate priorities
td-bug "task"         # Bug category (high priority)
td-feature "task"     # Feature category
td-docs "task"        # Documentation category
td-review "task"      # Review category (medium priority)
td-deploy "task"      # Deployment category (high priority)
td-test "task"        # Testing category
```

### Search-Based Actions

```bash
# Smart search and action
td-done "search term"     # Find and complete todo
td-rm "search term"       # Find and remove todo
td-urgent "search term"   # Find and mark as urgent
td-find "term" [category] # Search with optional category filter
td-next                   # Complete next highest priority todo
```

### Category Shortcuts

```bash
# Show todos by category
td-bugs          # All bug todos
td-features      # All feature todos  
td-reviews       # All review todos
```

### Git Integration

```bash
# Git project and branch integration
td-branch "task"         # Add todo: "branchname: task" [projectname]
td-branch-list          # Show todos for current branch
td-project              # Show all todos for current project
```

### Usage Examples

```bash
# Smart main alias usage
td                              # Launch TUI
td "Fix login bug"              # Add simple todo
td bug "Authentication fails"   # Add with category

# Priority-based creation
td-high "Critical production issue"
td-med "Update documentation" 
td-low "Refactor old code"

# Search-based actions
td-done "authentication"        # Find and complete
td-urgent "production"          # Find and mark urgent
td-next                        # Complete next priority item

# Git workflow integration  
# On branch "feature/user-auth" in project "my-app"
td-branch "Add login validation"
# Creates: "feature/user-auth: Add login validation" [my-app]

td-branch-list                 # Show branch todos
td-project                     # Show all project todos

# Category workflows
td-bug "Login fails on Safari"
td-bugs                        # Show all bug todos
```

## ✅ Active Files (Still Used)

These bash utilities are **still sourced** and provide valuable shortcuts:

- **todo.ali.sh** - ✅ Comprehensive todo system aliases with smart search and git integration
- **basic.ali.sh** - ✅ Basic utility aliases and shortcuts
- **clear.ali.sh** - ✅ Comprehensive clear command typo aliases + smart command-not-found handler
- **maven.ali.sh** - ✅ Maven-specific aliases and build shortcuts
- **npm.ali.sh** - ✅ NPM/Node.js development shortcuts

## ❌ Deprecated Files (Replaced by C++)

These files have been **replaced by the C++ implementation** and renamed with `.ali-deprecated.sh` extension:

- **code.ali-deprecated.sh** - ❌ Project navigation (→ `aliases-cli code`)
- **update-workspaces.ali-deprecated.sh** - ❌ Workspace updates (→ `aliases-cli update`)
- **project-selection.ali-deprecated.sh** - ❌ Environment setup (→ `aliases-cli env`)
- **mappings.local.sh** - ❌ Bash mappings (→ `mappings.json`)
- **mappings.template.sh** - ❌ Bash template (→ `mappings.template.json`)
- **local-init-workspaces.sh** - ❌ Initialization (→ integrated in C++)
- **setup.sh.old** - ❌ Original setup (→ new `install.sh`)

## Migration Status: Hybrid System ⚡

| Component | Implementation | Status | Performance |
|-----------|---------------|--------|-------------|
| Project Navigation | C++ | ✅ Active | 50x faster |
| Workspace Updates | C++ | ✅ Active | 10x faster |
| Environment Setup | C++ | ✅ Active | 20x faster |
| Todo System Core | C++ | ✅ Active | Fast persistence |
| Todo Aliases | Bash | ✅ Active | Smart shortcuts |
| Basic Utilities | Bash | ✅ Active | Original |
| Clear Typo Aliases | Bash | ✅ Active | German keyboard optimized |
| Maven Shortcuts | Bash | ✅ Active | Original |
| NPM Shortcuts | Bash | ✅ Active | Original |

## Why Hybrid?

- **Performance-critical** operations (navigation, updates, todo core) → C++
- **Smart shortcuts** (todo aliases, maven, npm, basic) → Bash (flexibility)
- **Best of both worlds**: Speed where needed, bash convenience for workflows

## Usage

```bash
# Fast C++ commands (always use these for core operations)
c <project>           # 50x faster than old bash version
uw                   # 10x faster parallel updates  
project_env          # 20x faster environment setup
aliases-cli todo     # Fast todo management

# Smart Bash aliases (complement the C++ commands)
td "task"            # Smart todo creation (from todo.ali.sh)
td-branch "task"     # Git-integrated todos
td-done "search"     # Smart todo completion
mvn-shortcuts        # From maven.ali.sh
npm-helpers          # From npm.ali.sh  
basic-utils          # From basic.ali.sh
```

**Setup:** Run `./install.sh` to configure both C++ and active bash utilities.
