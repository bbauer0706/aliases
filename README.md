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
git clone <repository-url> ~/aliases

# Run the setup script
~/aliases/setup.sh
```

### Update Existing Installation

If you already have the repository cloned:

```bash
# Pull latest changes
cd ~/aliases
git pull

# Run the setup script to apply any changes
./setup.sh
```

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

Edit the files in the `~/aliases` directory to customize or add your own aliases.
Each file is organized by category and sourced automatically.

## License

MIT
