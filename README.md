# Bash Aliases

A collection of useful bash aliases and functions to improve command-line productivity.

## Features

- Project navigation with VSCode (`c` command)
- Git shortcuts (`g` command)
- Maven shortcuts (`m` command)
- NPM shortcuts
- Easy installation and setup

## Installation

### New Installation

```bash
# Clone the repository
git clone <repository-url> <dir>

# Make the setup script executable if needed
chmod +x <dir>/setup.sh

# Run the setup script
<dir>/setup.sh
```

### Update Existing Installation

If you already have the repository cloned:

```bash
# Pull latest changes
cd <dir>/aliases
git pull

# Make sure the setup script is executable
chmod +x setup.sh

# Run the setup script to apply any changes
./setup.sh
```

## What the Setup Script Does

The setup script performs these essential tasks:

1. Creates or updates your `~/.bash_aliases` file to source all alias files
2. Ensures your `~/.bashrc` properly sources the `~/.bash_aliases` file
3. Preserves your existing aliases if you already have a `.bash_aliases` file

## Available Commands

### Project Navigation (`c`)

Navigate and open projects in VSCode:

- `c` - Open home directory
- `c project` - Open the specified project
- `c projects` - Open the server component of a project
- `c projectw` - Open the web component of a project
- `c project[sw]` - Open both server and web components

### Git Shortcuts (`g`)

- `g s` - git status
- `g a` - git add .
- `g c "msg"` - git commit -m "msg"
- `g ac "msg"` - git add . and commit
- `g p` - git pull
- `g ps` - git push
- See more with `g` command

### Maven Shortcuts (`m`)

- `m ci` - mvn clean install
- `m cp` - mvn clean package
- `m cid` - mvn clean install -DskipTests
- See more with `m` command

## Customization

Edit the files in the `<dir>/aliases` directory to customize or add your own aliases.
Each file is organized by category and sourced automatically.

### Adding Your Own Aliases

Create new `.ali.sh` files in the aliases directory to organize your aliases by category:

```bash
# Create a new aliases file
touch <dir>/aliases/my_custom.ali.sh

# Make it executable
chmod +x <dir>/aliases/my_custom.ali.sh

# Edit the file
code <dir>/aliases/my_custom.ali.sh
```

### Using Local Aliases

For machine-specific aliases that shouldn't be committed to version control, use the `.local.ali.sh` or `local.ali.sh` file pattern:

```bash
# Create a local aliases file
touch <dir>/aliases/my_custom.local.ali.sh

# Make it executable
chmod +x <dir>/aliases/my_custom.local.ali.sh

# Edit the file
code <dir>/aliases/my_custom.local.ali.sh
```

Files matching these patterns are automatically sourced but ignored by git, making them perfect for:
- Machine-specific paths
- Work-specific aliases
- Credentials or sensitive information
- Personal preferences

To apply changes immediately:

```bash
source ~/.bash_aliases
```

## Troubleshooting

- **Aliases not working**: Make sure all `.ali.sh` files are executable with `chmod +x <dir>/aliases/*.ali.sh`
- **Changes not applying**: Remember to run `source ~/.bash_aliases` after making changes
- **Script permission issues**: If you get "permission denied" errors, run `chmod +x <dir>/aliases/setup.sh`
