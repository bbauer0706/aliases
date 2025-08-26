#!/bin/bash
# Template for local mappings for code.ali.sh and update-workspaces.ali.sh
# Copy this file to mappings.local.sh and customize as needed

# Additional project shortcuts
# These mappings are used by both the 'c' (code navigation) and 'uw' (update workspaces) commands
declare -A local_full_to_short=(
  # Add your project mappings here
  # Example: [project-name]="shortcut"
)

# Custom server paths
# Define custom paths for server components (relative to project root)
# Server components are typically Spring Boot projects and will be updated with Maven
declare -A local_server_paths=(
  # Add your server paths here
  # Example: [project-name]="path/to/server"
)

# Custom web paths  
# Define custom paths for web components (relative to project root)
# Web components are typically npm projects and will be updated with npm install
declare -A local_web_paths=(
  # Add your web paths here
  # Example: [project-name]="path/to/web"
)
