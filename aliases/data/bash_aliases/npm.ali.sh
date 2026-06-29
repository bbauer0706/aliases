#!/usr/bin/env bash
# npm / pnpm shortcuts and wrappers.

# ── Script shortcuts ────────────────────────────────────────────────────────
alias dev='npm run dev'
alias gen='npm run gen'
alias build='npm run build'
alias blocal='npm run build:local'
alias bstatic='npm run build:static'
alias vit='npm run vitest'
alias jes='npm run test:watch'
alias story='npm run start:storybook'
alias npmi='npm i'
alias lint='npm run lint'
alias pipeline='npm run pipeline:dry-run'

# ── Clean install ───────────────────────────────────────────────────────────
# Removes .next, node_modules, and package-lock.json then reinstalls.
npm-clean() {
    [[ -d ".next" ]]           && echo "  Removing .next…"           && rm -rf .next
    [[ -d "node_modules" ]]    && echo "  Removing node_modules…"    && rm -rf node_modules
    [[ -f "package-lock.json" ]] && echo "  Removing package-lock.json…" && rm -f package-lock.json
    npm i
}

# ── pnpm preference ─────────────────────────────────────────────────────────
# If pnpm is installed, transparently use it instead of npm.
if command -v pnpm &>/dev/null; then
    alias npm='pnpm'
fi

# ── Hardened npm wrapper ─────────────────────────────────────────────────────
# Automatically adds --ignore-scripts to lifecycle-relevant commands.
# Only active when the alias above is NOT pointing npm → pnpm.
if ! command -v pnpm &>/dev/null; then
    npm() {
        local cmd="$1"
        shift
        case "$cmd" in
            install|i|update|up|ci|rebuild|add|remove|rm|link|dedupe|pack)
                command npm "$cmd" --ignore-scripts "$@"
                ;;
            *)
                command npm "$cmd" "$@"
                ;;
        esac
    }
fi
