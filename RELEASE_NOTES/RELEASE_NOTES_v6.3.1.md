# Chipmunk Tools v6.3.1 Release

**Release Date:** November 23, 2025

This release brings improvements to mouse cursor handling, zoom fit functionality, and build system reliability.

## 🎉 What's New

### ✨ Key Features

#### **Enhanced Mouse Cursor System**
- **Distinct X11 cursors for all editing modes** - Each editing mode now has a unique, semantically meaningful cursor:
  - **Move mode**: Four-way arrow (⟷) - `XC_fleur`
  - **Copy mode**: Plus sign (+) - `XC_plus`
  - **Delete mode**: Large X - `XC_X_cursor`
  - **Probe mode**: Arrow with question mark - `XC_question_arrow`
  - **Box mode**: Crosshair - `XC_crosshair`
  - **Rotate mode**: Circular arrows (↻) - `XC_exchange`
  - **Mirror H mode**: Horizontal double arrow (↔) - `XC_sb_h_double_arrow`
  - **Mirror V mode**: Vertical double arrow (↕) - `XC_sb_v_double_arrow`
- **Improved cursor visibility**: Increased default cursor scale to 2x for better visibility on high-DPI displays
- **Fixed cursor scaling bit order**: Corrected bitmap cursor scaling implementation
- **Eliminated cursor flickering**: Removed software/hardware cursor conflicts that caused visual glitches
- **Fixed memory corruption**: Extended cursors array from 4 to 9 elements to prevent array bounds violations
- **Application startup fix**: Application now starts with normal arrow cursor instead of rotation cursor

#### **Fixed Zoom Fit (F key) Functionality**
- **Fixed integer division bug**: Ensures minimum zoom of 1 when view is smaller than object size
  - Previously: `zoom_x = 620/764 = 0`, causing incorrect minimum zoom selection
  - Now: `zoom_x = max(1, view_width / obj_width)`
- **Fixed history dependency**: Successive F key presses no longer zoom out progressively
  - Excluded labels from bounding box calculation in `fitzoom()` to prevent scale-dependent width changes
  - Labels caused history dependency because `width = m_strwidth(...) / gg.scale`
- **Fixed coordinate calculation**: Corrected zoom calculation by removing incorrect `log_scale0` division
  - Drawing code uses: `screen = circuit * scale - xoff`
  - Object screen width = `obj_width * scale` (not divided by `log_scale0`)
- **Added comprehensive debugging**: New `CHIPMUNK_DEBUG_FITZOOM` environment variable for detailed zoom diagnostics
  - Shows window size, margins, bounding boxes, object dimensions, zoom calculations, and fit verification

### 📦 What's Included

This release includes:
- **Pre-built binaries** (via GitHub Actions CI):
  - Linux x86_64
  - macOS Intel (x86_64)
  - macOS Apple Silicon (ARM64)
- All configuration files and gate libraries
- **New PCMP pads**: Added pad definitions from shih chip to `log/lib/pads.gate`
- Interactive tutorial circuits (in `lessons/` directory)
- Complete documentation (README, CHANGELOG, DEVELOPER.md, HELP.md)
- Source code with cross-platform build system

## 🐛 Bug Fixes

### Zoom and View
- **History dependency in fitzoom()**: Fixed successive F key presses zooming out progressively
  - Root cause: Labels included in bounding box have width dependent on current zoom level
  - Solution: Exclude labels from `fitzoom()` bounding box calculation
- **Incorrect coordinate calculation**: Fixed zoom calculation by removing incorrect `log_scale0` division
- **Missing F key binding**: Fixed missing 'F' key macro binding to FIT command

### Build System
- **Makefile dependency tracking**: Fixed `analog` rebuild when source files change
  - Removed `bin/diglog` from `.PHONY` to prevent unnecessary rebuilds
  - Added proper source file dependencies (`LOG_SRC_FILES`, `LOG_MAKEFILE`) to `log/src/log` target
  - Simplified build logic using Make's built-in dependency tracking
- **Release tarball structure**: Fixed tarballs to include proper root folder structure
  - Prevents extraction from polluting current directory
  - Added `workflow_dispatch` support with tag input for manual rebuilds
  - Fixes issue where `tar xf chipmunk*.tgz` would extract files directly to current folder

### Path and Configuration
- **Path clarification**: Improved path handling documentation and behavior
- **genlog fixes**: Fixed `genlog` configuration and functionality
- **PCMP pads**: Added pad definitions from shih chip to gate library

## 📚 Documentation Improvements

### Updated Documentation
- **HELP.md**: 
  - Added `CHIPMUNK_DEBUG_FITZOOM` environment variable documentation
  - Enhanced cursor scale documentation with X11 font cursor details
  - Clarified cursor size control on different platforms (X/WSL/Wayland)
- **Inline code comments**: Comprehensive documentation of cursor system architecture

## 🚀 Quick Start

### Option 1: Use Pre-built Binaries (Recommended)

**For macOS (including Apple Silicon):**
```bash
# Download from releases page
wget https://github.com/sensorsINI/chipmunk/releases/download/v6.3.1/chipmunk-macos-arm64.tar.gz
# OR for Intel Macs:
wget https://github.com/sensorsINI/chipmunk/releases/download/v6.3.1/chipmunk-macos-intel.tar.gz

# Extract and run
tar -xzf chipmunk-macos-*.tar.gz
cd chipmunk-macos-*/bin
./analog
```

**For Linux:**
```bash
wget https://github.com/sensorsINI/chipmunk/releases/download/v6.3.1/chipmunk-linux-x86_64.tar.gz
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
git checkout v6.3.1
make

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
git checkout v6.3.1
make

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

### New Environment Variables

- **`CHIPMUNK_DEBUG_FITZOOM`**: Enable detailed `fitzoom()` debug output showing zoom calculations, bounding boxes, and coordinate transformations.
  ```bash
  CHIPMUNK_DEBUG_FITZOOM=1 analog circuit.lgf
  ```

### Cursor System Architecture

The cursor system now uses X11 font cursors by default for better visibility and compatibility:
- **Hardware cursors**: Uses X server's native font cursors (better performance, no flickering)
- **Bitmap cursors**: Available via `CHIPMUNK_USE_BITMAP_CURSOR=1` for classic look (with known redraw bug)
- **Cursor scale**: Default 2x for bitmap cursors when enabled; X11 font cursor size controlled by desktop environment

## 📝 Full Changelog

See [CHANGELOG.md](CHANGELOG.md) for complete details of all changes since v6.3.0.

### Key Commits in This Release
- `4f37c49`: Add distinct X11 cursors for all editing modes
- `6347911`: Increase default cursor scale to 2x and fix scaling bit order
- `740d75e`: Fix fitzoom() history dependency by excluding labels from bounding box
- `c31bfc2`: Fix integer division bug and document history dependency in fitzoom()
- `e07b216`: Fix fitzoom() coordinate calculation and add comprehensive debugging
- `8d17589`: Fix Makefile dependency tracking for analog rebuild
- `e71f4d4`: Fix release tarballs to include proper root folder structure
- `349c541`: Clarified path, fixed genlog and added pcmp pads

## 🙏 Credits

**Original Authors:**
- Dave Gillespie
- John Lazzaro
- Rick Koshi
- Glenn Gribble
- Adam Greenblatt
- Maryann Maher

**Version 6.3.1 Contributors:**
- Tobi Delbruck (Sensors Group, UZH-ETH Zurich) - Maintainer, cursor system improvements, zoom fit fixes, release management

**Special Thanks:**
- Community testing and feedback on cursor visibility and zoom behavior

## 🔗 Links

- **GitHub Repository**: https://github.com/sensorsINI/chipmunk
- **Original Chipmunk**: https://john-lazzaro.github.io/chipmunk/
- **Issue Tracker**: https://github.com/sensorsINI/chipmunk/issues
- **Release Page**: https://github.com/sensorsINI/chipmunk/releases/tag/v6.3.1

---

**Note**: This is a community-maintained fork focused on modern platform support. For the original Chipmunk distribution, visit [https://john-lazzaro.github.io/chipmunk/](https://john-lazzaro.github.io/chipmunk/)
