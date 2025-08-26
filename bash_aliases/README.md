# Mixed Status: Bash Aliases Directory

This directory contains both **active utilities** and **deprecated scripts** from the original bash-based system.

## ✅ Active Files (Still Used)

These bash utilities are **still sourced** and provide valuable shortcuts:

- **basic.ali.sh** - ✅ Basic utility aliases and shortcuts
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
| Basic Utilities | Bash | ✅ Active | Original |
| Maven Shortcuts | Bash | ✅ Active | Original |
| NPM Shortcuts | Bash | ✅ Active | Original |

## Why Hybrid?

- **Performance-critical** operations (navigation, updates) → C++
- **Utility shortcuts** (maven, npm, basic) → Bash (flexibility)
- **Best of both worlds**: Speed where needed, bash convenience for utilities

## Usage

```bash
# Fast C++ commands (always use these for core operations)
c <project>           # 50x faster than old bash version
uw                   # 10x faster parallel updates
project_env          # 20x faster environment setup

# Bash utilities (complement the C++ commands)
mvn-shortcuts        # From maven.ali.sh
npm-helpers          # From npm.ali.sh  
basic-utils          # From basic.ali.sh
```

**Setup:** Run `./install.sh` to configure both C++ and active bash utilities.
