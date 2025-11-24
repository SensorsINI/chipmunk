# Chipmunk Tools v6.0.0 Release

**Release Date:** November 15, 2024

This is the first release of the Chipmunk Tools fork, based on the original Chipmunk 5.66 from [John Lazzaro's GitHub Pages site](https://john-lazzaro.github.io/chipmunk/).

## 🎉 What's New

This release brings significant improvements to make Chipmunk easier to build, use, and understand on modern Linux systems (Ubuntu/WSL2).

### ✨ Key Features

- **Quick Start Guide**: New users can get up and running in minutes with clear installation instructions
- **Improved User Experience**: 
  - Automatic tutorial loading for first-time users (`lesson1.lgf`)
  - Help system with `--help` option
  - Browser-based help (replaces xterm) with WSL2 support
- **Better Documentation**: 
  - Comprehensive keyboard shortcuts reference
  - Configuration files guide
  - Sample circuits documentation
- **Enhanced Build System**: 
  - Top-level Makefile with `build`, `clean`, and `check` targets
  - Dependency checking before build
  - Clear error messages

### 🐛 Bug Fixes

- **Fixed segfault**: Resolved crash in `XSetCommand()` that prevented program startup
- **Fixed window naming**: Windows now correctly display as "analog" and "analog-console"
- **Fixed configuration discovery**: Automatic `LOGLIB` environment variable setup
- **Fixed X11 font errors**: Documented required font packages

### 📦 What's Included

This release includes:
- Pre-built binaries for Linux x86_64 (9.6M)
- All configuration files and gate libraries
- Interactive tutorial circuits (`lesson1.lgf` through `lesson5.lgf`)
- Complete documentation (README, CHANGELOG, screenshots)
- Source code (for building from scratch)

## 🚀 Quick Start

### Option 1: Use Pre-built Binaries (Recommended)

1. Download `chipmunk-6.0.0.tar.gz` from the [Releases page](https://github.com/sensorsINI/chipmunk/releases)
2. Extract: `tar -xzf chipmunk-6.0.0.tar.gz`
3. Run: `cd chipmunk-6.0.0 && ./bin/analog`

### Option 2: Build from Source

```bash
# Install dependencies
sudo apt-get install gcc make libx11-dev xfonts-base xfonts-75dpi xfonts-100dpi
xset fp rehash

# Clone and build
git clone https://github.com/sensorsINI/chipmunk.git
cd chipmunk
make build

# Run
./bin/analog
```

## 📋 System Requirements

- **OS**: Linux (tested on Ubuntu/WSL2)
- **Compiler**: GCC (ANSI C compatible)
- **Libraries**: X11 development libraries (libX11)
- **Fonts**: X11 fonts `6x10` and `8x13` (provided by `xfonts-base`, `xfonts-75dpi`, `xfonts-100dpi`)

## 📚 Documentation

- **README.md**: Complete user guide with installation, usage, and keyboard shortcuts
- **CHANGELOG.md**: Detailed version history
- **Official Docs**: [https://john-lazzaro.github.io/chipmunk/document/log/index.html](https://john-lazzaro.github.io/chipmunk/document/log/index.html)

## 🎓 Learning Resources

- **Interactive Tutorials**: Start with `lesson1.lgf` (opens automatically for new users)
- **Cheat Sheet**: See `log/lib/cheat.text` for quick reference
- **Sample Circuits**: Explore `log/lib/` for example circuits

## 🔧 Technical Details

- **Base Version**: Chipmunk 5.66
- **This Release**: 6.0.0
- **License**: GNU GPL v1 or later
- **Platform**: Linux x86_64

## 🙏 Credits

**Original Authors:**
- Dave Gillespie
- John Lazzaro
- Rick Koshi
- Glenn Gribble
- Adam Greenblatt
- Maryann Maher

**Version 6.0.0 Maintainer:**
- Tobi Delbruck (Sensors Group, UZH-ETH Zurich)

## 📝 Full Changelog

See [CHANGELOG.md](CHANGELOG.md) for complete details of all changes.

---

**Note**: This is a community-maintained fork. For the original Chipmunk distribution, visit [https://john-lazzaro.github.io/chipmunk/](https://john-lazzaro.github.io/chipmunk/)

