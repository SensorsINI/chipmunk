# Chipmunk Tools v6.3.2 Release

**Release Date:** November 28, 2025

This release fixes critical window resize performance issues, improves command-line help, and updates default simulation parameters.

## 🎉 What's New

### ✨ Key Features

#### **Fixed Window Resize Performance Issues**
- **Eliminated CPU busy-wait loops**: Fixed severe performance degradation that caused 100% CPU usage during window resizing
  - Root cause: ConfigureNotify events were being recycled via `XPutBackEvent()`, creating infinite event loops
  - Solution: Changed event handlers to consume events instead of recycling them
- **Removed empty busy-wait loop**: Fixed empty `do { } while (!pollkbd2())` loop in `inkey2()` that caused tight polling
- **Added intelligent throttling**: Implemented 10ms throttling in `m_pollkbd()` when no events are available
- **Fixed layout updates**: Added `update_screen_layout()` to properly recalculate layout variables after resize
- **Improved screen refresh**: Window geometry and menu positioning now update correctly after resize
- **Results**: Smooth window resizing with near-zero CPU usage during idle, multiple resizes work correctly, simulation continues running during resize

#### **Enhanced Command-Line Help System**
- **Clarified help options**: `--help` now explicitly documented (not `-h`, which is reserved for home directory)
- **Improved `-h` option**: Home directory option now properly documented and functional
- **Updated LOGLIB documentation**: Enhanced documentation explaining `LOGLIB` environment variable purpose and search path order
  - Clarified difference between `-h <dir>` (home directory) and `LOGLIB` (library directory)
  - Documented search order: current directory → launch directory → home directory → `$LOGLIB`

#### **Updated Default Simulation Parameters**
- **Default Vdd changed to 3.3V**: Updated from 5V to 3.3V to match modern CMOS standards
  - Updated in `log/lib/models.cnf` and `lessons/nfet.lgf`
  - Reflects modern semiconductor industry practices

#### **Enhanced Simulation Gates**
- **Added more simulation gates for beginners**: Expanded gate library in `analog.cnf` for educational use

## 📦 What's Included

This release includes:
- **Pre-built binaries** (via GitHub Actions CI):
  - Linux x86_64
  - macOS Intel (x86_64)
  - macOS Apple Silicon (ARM64)
- All configuration files and gate libraries
- Interactive tutorial circuits (in `lessons/` directory)
- Complete documentation (README, CHANGELOG, DEVELOPER.md, HELP.md)
- Source code with cross-platform build system

## 🐛 Bug Fixes

### Window Resize and Performance
- **CPU busy-wait during resize**: Fixed infinite event recycling causing 100% CPU usage
  - Removed `XPutBackEvent()` for ConfigureNotify events in `psys/src/mylib.c`
  - Fixed empty busy-wait loop in `log/src/log.c` `inkey2()` function
  - Added 10ms throttling to `m_pollkbd()` when no events available
- **Layout not updating after resize**: Fixed missing layout variable updates after window resize
  - Added `update_screen_layout()` function to recalculate layout variables
  - Modified `pen()` to detect size changes and trigger layout updates
- **Screen refresh issues**: Fixed screen not refreshing properly after resize
  - Set `needrefr=true` after resize to trigger proper screen refresh
  - Ensures zoomfit/autoscale work correctly after resize

### Command-Line Interface
- **Help option confusion**: Clarified that `--help` shows help (not `-h`)
  - `-h` is now properly reserved for home directory specification
  - Updated `bin/analog` wrapper script help output
- **LOGLIB documentation**: Improved documentation of `LOGLIB` environment variable
  - Clarified search path order and relationship with `-h` option

## 📚 Documentation Improvements

### Updated Documentation
- **HELP.md**: 
  - Clarified `--help` vs `-h` usage
  - Enhanced `LOGLIB` environment variable documentation
  - Updated search path order explanation
- **RESIZE_FIX.md**: Comprehensive technical documentation of window resize fix
  - Detailed root cause analysis using `strace` diagnostics
  - Design decisions and implementation details
  - Performance impact measurements

## 🚀 Quick Start

### Option 1: Use Pre-built Binaries (Recommended)

**For macOS (including Apple Silicon):**
```bash
# Download from releases page
wget https://github.com/sensorsINI/chipmunk/releases/download/v6.3.2/chipmunk-macos-arm64.tar.gz
# OR for Intel Macs:
wget https://github.com/sensorsINI/chipmunk/releases/download/v6.3.2/chipmunk-macos-intel.tar.gz

# Extract and run
tar -xzf chipmunk-macos-*.tar.gz
cd chipmunk-macos-*/bin
./analog
```

**For Linux:**
```bash
wget https://github.com/sensorsINI/chipmunk/releases/download/v6.3.2/chipmunk-linux-x86_64.tar.gz
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
git checkout v6.3.2
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
git checkout v6.3.2
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

### Window Resize Fix Details

The window resize performance fix addresses three critical issues:

1. **Event Recycling Loop**: ConfigureNotify events were being put back on the X11 event queue with `XPutBackEvent()`, causing infinite loops. Events are now consumed immediately after handling.

2. **Empty Busy-Wait**: An empty `do { } while (!pollkbd2())` loop in `inkey2()` was continuously polling for input. This has been removed.

3. **No Throttling**: `m_pollkbd()` was called in tight loops without throttling. Added intelligent 10ms delays when no events are available.

**Performance Impact:**
- Before: 100% CPU usage during resize, frequent hangs requiring Ctrl-Z
- After: Near-zero CPU usage during idle, smooth resizing, no hangs

See `RESIZE_FIX.md` for detailed technical analysis.

### Command-Line Options

The `analog` command supports the following options:

- **`--help`**: Show help message and exit (use `--help`, not `-h`)
- **`-h <dir>`**: Specify home directory for searching gate files, config files, etc.
- **`-c <file>`**: Specify configuration file (default: `analog.cnf`)
- **`-v`**: Vanilla LOG mode (no CNF file loaded)
- **`-x <display>`**: Specify X display name (e.g., `:0.0` or `hostname:0`)
- **`-z <file>`**: Enable trace mode with output file
- **`-d <file>`**: Specify dump file for debug output
- **`-t <file>`**: Specify trace file for trace output (alternative to `-z <file>`)
- **`-r <tool>`**: Run a specific tool immediately on startup (non-interactive mode)

**Environment Variables:**
- **`LOGLIB`**: Automatically set to `${CHIPMUNK_DIR}/log/lib` by wrapper script. Specifies directory for configuration files and gate libraries.
- **`-h <dir>`**: Sets home directory (default: `~/log`) for user-specific files.

## 📝 Full Changelog

See [CHANGELOG.md](CHANGELOG.md) for complete details of all changes since v6.3.1.

### Key Commits in This Release
- `02a7503`: Fix window resize hangs and CPU busy-wait loops
- `cb1e3c2`: Updated --help command and LOGLIB env var purpose and effect
- `2a8e3ce`, `8c30c55`: Change default Vdd to 3.3V
- `a3eef02`: Add more simulation gates for newbies
- `21edac6`: Bump to 6.3.2 for window resizing

## 🙏 Credits

**Original Authors:**
- Dave Gillespie
- John Lazzaro
- Rick Koshi
- Glenn Gribble
- Adam Greenblatt
- Maryann Maher

**Version 6.3.2 Contributors:**
- Tobi Delbruck (Sensors Group, UZH-ETH Zurich) - Maintainer, window resize performance fixes, help system improvements, release management

**Special Thanks:**
- Community feedback on window resize performance issues
- Testers who reported CPU usage problems during window resizing

## 🔗 Links

- **GitHub Repository**: https://github.com/sensorsINI/chipmunk
- **Original Chipmunk**: https://john-lazzaro.github.io/chipmunk/
- **Issue Tracker**: https://github.com/sensorsINI/chipmunk/issues
- **Release Page**: https://github.com/sensorsINI/chipmunk/releases/tag/v6.3.2

---

**Note**: This is a community-maintained fork focused on modern platform support. For the original Chipmunk distribution, visit [https://john-lazzaro.github.io/chipmunk/](https://john-lazzaro.github.io/chipmunk/)

