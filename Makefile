# Makefile wrapper for aliases-cli project
# This provides convenient make targets that call the build script

.PHONY: all build debug clean install test help release

# Default target
all: release

# Release build (default)
release:
	@echo "Building aliases-cli in Release mode..."
	@./build.sh

# Debug build
debug:
	@echo "Building aliases-cli in Debug mode..."
	@./build.sh --debug

# Clean build
clean:
	@echo "Cleaning build directory..."
	@./build.sh --clean

# Clean and build
rebuild: clean release

# Install to system
install:
	@echo "Building and installing aliases-cli..."
	@./build.sh --install

# Fast build (using all CPU cores)
fast:
	@echo "Fast build using all available cores..."
	@./build.sh --jobs $(shell nproc)

# Development build (debug)
dev:
	@echo "Development build (debug)..."
	@./build.sh --debug

# Show help
help:
	@echo "Available targets:"
	@echo "  all/release  - Build in Release mode (default)"
	@echo "  debug        - Build in Debug mode"
	@echo "  clean        - Clean build directory"
	@echo "  rebuild      - Clean and build"
	@echo "  install      - Build and install to /usr/local/bin"
	@echo "  fast         - Fast build using all CPU cores"
	@echo "  dev          - Development build (debug)"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "For more options, run: ./build.sh --help"

# Test the binary
test: all
	@echo "Testing aliases-cli..."
	@./build/aliases-cli --version
	@./build/aliases-cli --help
