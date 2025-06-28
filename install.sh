#!/bin/bash
set -e

# Orus Language Installer
# Usage: curl --proto '=https' --tlsv1.2 -sSf https://sh.orus.dev | sh

REPO="jordyorel/Orus-lang"
BINARY_NAME="orusc"
INSTALL_DIR="/usr/local/bin"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Detect OS and architecture
detect_platform() {
    local os
    local arch
    
    os=$(uname -s | tr '[:upper:]' '[:lower:]')
    arch=$(uname -m)
    
    case "$os" in
        linux|darwin)
            ;;
        *)
            print_error "Unsupported operating system: $os"
            exit 1
            ;;
    esac
    
    case "$arch" in
        x86_64|amd64)
            arch="x86_64"
            ;;
        arm64|aarch64)
            arch="arm64"
            ;;
        *)
            print_error "Unsupported architecture: $arch"
            exit 1
            ;;
    esac
    
    echo "${os}-${arch}"
}

# Get latest release tag
get_latest_release() {
    if command_exists curl; then
        curl -s "https://api.github.com/repos/${REPO}/releases" | grep '"tag_name"' | head -1 | cut -d'"' -f4
    elif command_exists wget; then
        wget -qO- "https://api.github.com/repos/${REPO}/releases" | grep '"tag_name"' | head -1 | cut -d'"' -f4
    else
        print_error "curl or wget is required"
        exit 1
    fi
}

# Download and install
install_orus() {
    local platform
    local latest_version
    local download_url
    local temp_dir
    
    print_status "Detecting platform..."
    platform=$(detect_platform)
    print_status "Platform detected: $platform"
    
    print_status "Getting latest release..."
    latest_version=$(get_latest_release)
    if [ -z "$latest_version" ]; then
        print_error "Could not determine latest version"
        exit 1
    fi
    print_status "Latest version: $latest_version"
    
    # For now, assume single binary works across platforms
    # In the future, you might have platform-specific binaries
    download_url="https://github.com/${REPO}/releases/download/${latest_version}/${BINARY_NAME}"
    
    print_status "Downloading Orus from $download_url..."
    
    temp_dir=$(mktemp -d)
    local binary_path="${temp_dir}/${BINARY_NAME}"
    
    if command_exists curl; then
        curl -L -o "$binary_path" "$download_url"
    elif command_exists wget; then
        wget -O "$binary_path" "$download_url"
    else
        print_error "curl or wget is required"
        exit 1
    fi
    
    # Verify download
    if [ ! -f "$binary_path" ] || [ ! -s "$binary_path" ]; then
        print_error "Download failed or file is empty"
        exit 1
    fi
    
    # Check if it's actually a binary (not an error page)
    if file "$binary_path" | grep -q "text"; then
        print_error "Downloaded file appears to be text, not a binary. Download may have failed."
        exit 1
    fi
    
    print_status "Making binary executable..."
    chmod +x "$binary_path"
    
    # Install binary
    if [ -w "$INSTALL_DIR" ]; then
        print_status "Installing to $INSTALL_DIR..."
        mv "$binary_path" "${INSTALL_DIR}/${BINARY_NAME}"
    else
        print_status "Installing to $INSTALL_DIR (requires sudo)..."
        sudo mv "$binary_path" "${INSTALL_DIR}/${BINARY_NAME}"
    fi
    
    # Cleanup
    rm -rf "$temp_dir"
    
    print_status "Verifying installation..."
    if command_exists "$BINARY_NAME"; then
        print_status "Orus installed successfully!"
        print_status "Version: $($BINARY_NAME --version 2>/dev/null || echo 'Unable to get version')"
        echo
        echo "Try running: $BINARY_NAME"
    else
        print_warning "Installation completed, but $BINARY_NAME not found in PATH"
        print_warning "You may need to add $INSTALL_DIR to your PATH or restart your shell"
    fi
}

# Check prerequisites
check_prerequisites() {
    if ! command_exists curl && ! command_exists wget; then
        print_error "Either curl or wget is required"
        exit 1
    fi
    
    if ! command_exists file; then
        print_warning "file command not found, skipping binary verification"
    fi
}

# Main installation flow
main() {
    echo "Orus Language Installer"
    echo "======================="
    echo
    
    check_prerequisites
    install_orus
}

# Run main function
main "$@"