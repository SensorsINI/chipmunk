# Chipmunk Tools v6.3.0 Release

**Release Date:** November 21, 2025

This release brings critical bug fixes for keyboard input handling and comprehensive macOS Apple Silicon support with cross-platform improvements.

## 🎉 What's New

### ✨ Key Features

#### **macOS Apple Silicon Support (Verified)**
- **Full native support for Apple Silicon (M1/M2/M3)** with clean builds
- **Cross-platform shell support**: Automatic PATH setup now supports both **bash** and **zsh**
  - Interactive prompt after build to add `chipmunk/bin` to your PATH
  - Detects your shell (`.bashrc` for bash, `.zshrc` for zsh)
  - Smart updates: won't duplicate entries if already present
- **Improved uninstall**: `make uninstall` now handles both `.bashrc` and `.zshrc`
- **Platform-specific compilation**: Fixed struct size mismatches using `-D__alpha__` flag
  - Resolves "Can't find gate 'TIME'" error on macOS
  - Ensures 256-byte alignment for binary `.gate` files
- **Clang compatibility**: Converted GNU C nested functions to standard C static functions
  - No more compilation errors on macOS/Clang
  - Maintains full Linux/GCC compatibility

#### **Fixed Keyboard Input Handling**
- **Fixed Delete key (forward delete) on native Linux**
  - Previously worked on WSL but was broken on native Linux
  - Now correctly deletes the character AT the cursor position (forward delete)
  - Backspace correctly deletes character BEFORE cursor (backward delete)
- **Fixed Ctrl-U behavior**
  - Now deletes from start of line to character before cursor (standard shell behavior)
  - Works consistently in both console window and `:` command prompt
- **Comprehensive keyboard architecture documentation**
  - Added detailed comments in `log.c` and `mylib.c` explaining key mapping
  - Documents which functions handle input in which contexts
  - Prevents future regressions

### 📦 What's Included

This release includes:
- **Pre-built binaries** (via GitHub Actions CI):
  - Linux x86_64
  - macOS Intel (x86_64)
  - macOS Apple Silicon (ARM64)
- All configuration files and gate libraries
- Interactive tutorial circuits (now in `lessons/` directory)
- Complete documentation (README, CHANGELOG, DEVELOPER.md)
- Source code with cross-platform build system

## 🐛 Bug Fixes

### Keyboard Input
- **Delete key regression on native Linux**: Fixed forward delete functionality in console input
- **Ctrl-U behavior**: Now correctly deletes from cursor to start of line (matches shell behavior)
- **Key event handling**: Comprehensive fix for X11 keyboard event mapping
  - `0x07` (BEL): Backspace - delete char BEFORE cursor
  - `0x05` (ENQ): Delete - delete char AT cursor (forward delete)
  - `0x15` (NAK): Ctrl-U - delete from start to cursor
  - `0x03` (ETX): Ctrl-C/ESC - exit input mode
  - `0x08` (BS): Left arrow - move cursor left
  - `0x1C` (FS): Right arrow - move cursor right

### Cross-Platform Compilation
- **Nested function errors on macOS**: Converted 4 instances of GNU C nested functions to standard C static functions
  - `psys/src/mylib.c`: 3 debug logging helpers
  - `log/src/log.c`: 1 coordinate conversion helper
- **Compiler warning flags**: Updated Makefiles to use platform-appropriate flags
  - GCC-specific flags only on Linux
  - Clang-compatible flags on macOS
- **Struct size mismatch on macOS**: Added `-D__alpha__` to ensure 256-byte `filerec` union alignment

### File Path Handling
- **Save/load path resolution**: Fixed to work relative to launch directory (not current directory)
- **`.lgf` extension**: Made optional for file loading (user-friendly)
- **Buffer overflow**: Fixed in `savepage()` path resolution

## 📚 Documentation Improvements

### New Documentation
- **DEVELOPER.md**: Comprehensive cross-platform development guide
  - Explains nested functions vs. static functions
  - Documents compiler differences (GCC vs. Clang)
  - Provides coding guidelines for maintainability
- **Keyboard architecture comments**: Detailed inline documentation in source
  - Key mapping table in `mylib.c`
  - Input function responsibilities in `log.c`
  - Context-specific behavior (console vs. main window)

### Updated Documentation
- **README.md**: Enhanced with cross-platform build instructions
- **HELP.md**: Added Chipmunk-specific environment variables section
- **CHANGELOG.md**: Updated with all v6.3.0 changes

## 🚀 Quick Start

### Option 1: Use Pre-built Binaries (Recommended)

**For macOS (including Apple Silicon):**
```bash
# Download from releases page
wget https://github.com/sensorsINI/chipmunk/releases/download/v6.3.0/chipmunk-macos-arm64.tar.gz
# OR for Intel Macs:
wget https://github.com/sensorsINI/chipmunk/releases/download/v6.3.0/chipmunk-macos-intel.tar.gz

# Extract and run
tar -xzf chipmunk-macos-*.tar.gz
cd chipmunk-macos-*/bin
./analog
```

**For Linux:**
```bash
wget https://github.com/sensorsINI/chipmunk/releases/download/v6.3.0/chipmunk-linux-x86_64.tar.gz
tar -xzf chipmunk-linux-x86_64.tar.gz
cd chipmunk-linux-x86_64/bin
./analog
```

### Option 2: Build from Source

**macOS (including Apple Silicon):**
```bash
# Install dependencies
brew install --cask xquartz  # Provides X11, fonts, headers
brew install gcc             # C compiler

# Clone and build
git clone https://github.com/sensorsINI/chipmunk.git
cd chipmunk
make

# After build, you'll be prompted to add to PATH
# For zsh users (default on macOS): adds to ~/.zshrc
# For bash users: adds to ~/.bashrc

# Run
./bin/analog
```

**Linux (Ubuntu/Debian/WSL2):**
```bash
# Install dependencies
sudo apt-get install gcc make libx11-dev xfonts-base xfonts-75dpi xfonts-100dpi
xset fp rehash

# Clone and build
git clone https://github.com/sensorsINI/chipmunk.git
cd chipmunk
make

# After build, you'll be prompted to add to PATH
# Adds to ~/.bashrc for bash users

# Run
./bin/analog
```

## 📋 System Requirements

### All Platforms
- **X11 Window System**: 
  - Linux: Built-in or via X.org
  - macOS: XQuartz (download from https://www.xquartz.org/)
- **X11 fonts**: `6x10` and `8x13` (included with XQuartz on macOS)

### macOS-Specific
- **OS**: macOS 10.15 (Catalina) or later
- **Architecture**: Intel (x86_64) or Apple Silicon (ARM64)
- **XQuartz**: Version 2.8.0 or later
- **Compiler**: GCC via Homebrew (Clang not sufficient for all features)

### Linux-Specific
- **OS**: Ubuntu 20.04+, Debian 11+, or compatible (including WSL2)
- **Compiler**: GCC (ANSI C compatible)
- **Libraries**: X11 development libraries (libX11)

## 🔧 Technical Details

### Platform Support Matrix

| Platform | Architecture | Build Status | Notes |
|----------|-------------|--------------|-------|
| Linux | x86_64 | ✅ Verified | Native and WSL2 |
| macOS | ARM64 (M1/M2/M3) | ✅ Verified | Requires XQuartz |
| macOS | Intel (x86_64) | ✅ Verified | Requires XQuartz |

### Build System Improvements
- **Automatic platform detection**: Makefiles detect OS and configure appropriately
- **XQuartz detection**: Automatically finds XQuartz at `/opt/X11` or `/usr/X11R6`
- **Shell detection**: PATH setup script detects bash vs. zsh
- **CI/CD**: GitHub Actions builds all platforms automatically on release

### Code Quality
- **Standard C compliance**: All code now uses C89/C99 standard features
- **No GNU extensions**: Eliminated nested functions for cross-platform compatibility
- **Cleaner builds**: Platform-specific warning flag suppression
- **Comprehensive comments**: Inline documentation for maintainability

## 📝 Full Changelog

See [CHANGELOG.md](CHANGELOG.md) for complete details of all changes since v6.2.0.

### Key Commits in This Release
- `7065f31`: Fix cross-platform compilation: nested functions to static functions
- `4d0d14f`: Fix Delete key and Ctrl-U in console input with comprehensive documentation
- `ef849ed`: fix(macos): resolve struct size mismatch causing gate loading failures
- `346c642`: Fix save command to save files relative to launch directory
- `98487d0`: Auto path setup with shell detection

## 🙏 Credits

**Original Authors:**
- Dave Gillespie
- John Lazzaro
- Rick Koshi
- Glenn Gribble
- Adam Greenblatt
- Maryann Maher

**Version 6.3.0 Contributors:**
- Tobi Delbruck (Sensors Group, UZH-ETH Zurich) - Maintainer, keyboard fixes, release management
- Tarek Allam Jr (@tallamjr) - macOS support, struct alignment fix, CI/CD setup

**Special Thanks:**
- Community testing and feedback on Linux and macOS platforms

## 🔗 Links

- **GitHub Repository**: https://github.com/sensorsINI/chipmunk
- **Original Chipmunk**: https://john-lazzaro.github.io/chipmunk/
- **Issue Tracker**: https://github.com/sensorsINI/chipmunk/issues
- **Release Page**: https://github.com/sensorsINI/chipmunk/releases/tag/v6.3.0

---

**Note**: This is a community-maintained fork focused on modern platform support. For the original Chipmunk distribution, visit [https://john-lazzaro.github.io/chipmunk/](https://john-lazzaro.github.io/chipmunk/)

