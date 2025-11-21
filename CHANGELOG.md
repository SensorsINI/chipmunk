# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [6.1.0] - 2025-11-17

### Added
- **Startup Welcome Message**: Terminal displays essential keyboard shortcuts at launch for first-time users
- **Fit-and-Zoom Feature**: Press 'F' key to automatically center and zoom all circuit objects to fit in window
- **Extended Zoom Levels**: Expanded from 5 to 7 zoom levels (scales: 1, 2, 3, 5, 8, 12, 20)
- **Auto-Fit on Startup**: Circuits automatically fit to window size when loaded
- **ESC Key Support**: ESC key now exits modes (wire-drawing, delete, etc.) in addition to Ctrl-C
- **Window Geometry Persistence**: Window size is saved to `~/.chipmunk_geometry` and restored on restart
- **Focus-Aware Click Handling**: Prevents accidental wire-drawing when clicking to focus window
- **Improved Crash Reporting**: Shows source file and line numbers in crash dumps for PIE binaries
- **Comprehensive Code Documentation**: Added detailed headers to `log.c` and `mylib.c` explaining coordinate system, zoom logic, and X11 event handling
- **HELP.md**: Complete user guide with keyboard shortcuts, navigation, and workflow tips
- **Test Infrastructure**: Crash handler test utility with documentation

### Fixed
- **Build System**: Added `bin/diglog` to `.PHONY` targets so top-level `make` reliably detects source changes
- **String Buffer Warnings**: Increased buffer sizes and added compiler pragmas to suppress false positives
- **Gets Warning**: Filtered deprecated `gets()` warnings from build output
- **PATH Setup**: Fixed WSL compatibility issues in PATH setup script
- **Coordinate System**: Corrected fit-and-zoom centering formula for accurate view positioning
- **Zoom Safety**: Added 80% safety factor to ensure objects fit with margin after discrete zoom level selection
- **Division by Zero**: Added defensive checks in `fitzoom()` to prevent crashes on empty circuits
- **Global Variable Corruption**: Fixed zoom calculation to use local variables before updating globals

### Changed
- **Default Window Size**: Increased from 512x390 to 1280x960 for modern displays
- **Wire-Drawing Cancel**: Clarified that right-click is the intended way to cancel wire-drawing (ESC/Ctrl-C exit the mode after starting)
- **Debug Support**: Added conditional debug logging via `CHIPMUNK_DEBUG_ESC` environment variable
- **Keyboard Mapping**: Implemented ESC-to-Ctrl-C mapping at low level in `mylib.c` input functions

### Developer Experience
- **Improved Build Reliability**: `.PHONY` target ensures source changes are always detected
- **Better Documentation**: Code headers explain complex coordinate transformations and event handling
- **Debug Utilities**: Environment-variable-controlled debug logging for troubleshooting

## [6.0.0] - 2024-11-15

### Added
- Wrapper script (`bin/analog`) that automatically sets `LOGLIB` environment variable
- Automatic opening of `lesson1.lgf` tutorial for first-time users
- Help option (`--help` or `-h`) with usage information
- Dynamic window naming based on program name (analog/analog-console vs diglog/diglog-console)
- Browser-based help system (replaces xterm-based help)
- WSL2 detection and Windows browser integration for help system
- Comprehensive keyboard shortcuts documentation in README
- Screenshots of analog and analog-console windows
- Version information section in README
- CHANGELOG.md for tracking changes
- Top-level Makefile with `build`, `clean`, and `check` targets
- Improved dependency checking in Makefile
- `.gitignore` to exclude build artifacts and binaries

### Fixed
- Fixed X11 font errors by documenting required font packages (xfonts-base, xfonts-75dpi, xfonts-100dpi)
- Fixed segfault in `XSetCommand()` by using proper `char **argv` array instead of `char (*)[256]`
- Fixed window naming to correctly display "analog" and "analog-console" when run via wrapper script
- Fixed configuration file discovery by setting `LOGLIB` environment variable automatically
- Updated version number from 5.61 to 5.66 in source code for consistency

### Changed
- Updated version to 6.0.0 for this fork/modification
- Removed compiled binaries from git tracking (now in `.gitignore`)
- Improved build system with better error messages and dependency checks
- Enhanced README with comprehensive documentation including:
  - Keyboard shortcuts reference
  - Configuration files documentation
  - Sample circuits information
  - Installation instructions for modern Linux systems

### Security
- No security-related changes in this release

## [5.66] - Original Chipmunk Release

This version represents the original Chipmunk tools (version 5.66) from the official source at https://john-lazzaro.github.io/chipmunk/

### Original Components
- LOG: Circuit editing and simulation system
- AnaLOG: Analog circuit simulator
- DigLOG: Digital circuit simulator
- Loged: Gate editor for creating custom gate icons
- SPICE converter: Version 1.0 Beta

### Original Authors
- Dave Gillespie
- John Lazzaro
- Rick Koshi
- Glenn Gribble
- Adam Greenblatt
- Maryann Maher

