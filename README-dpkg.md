# Debian Package Guide for Chipmunk Tools

This guide documents how to create Debian packages (`.deb` files) for chipmunk-tools, enabling installation via `sudo dpkg -i chipmunk-tools.deb` or potentially `sudo apt install chipmunk-tools` (if a repository is set up).

## Overview

Chipmunk Tools can be packaged as a Debian package that:
- Installs binaries to `/usr/bin/`
- Installs configuration files and libraries to `/usr/share/chipmunk-tools/`
- Declares dependencies (libx11, X11 fonts, etc.)
- Provides proper package metadata

## Prerequisites

To build Debian packages, you need:
```bash
sudo apt-get install devscripts build-essential debhelper
```

## Package Structure

The Debian package will contain:

### Binaries (→ `/usr/bin/`)
- `analog` - Wrapper script for analog simulator
- `diglog` - Digital/analog simulator binary
- `loged` - Gate editor
- `fixfet7` - FET7 model fixer utility

### Configuration Files (→ `/usr/share/chipmunk-tools/lib/`)
- All files from `log/lib/`:
  - Configuration files (`.cnf`)
  - Gate libraries (`.gate`)
  - Lesson files (`.lgf`)
  - Documentation (`.text`, `.ps`, `.pdf`)
  - Helper scripts and utilities

### Documentation (→ `/usr/share/doc/chipmunk-tools/`)
- `README.md`
- `CHANGELOG.md`
- License files

## Step-by-Step Guide

### 1. Create `debian/` Directory Structure

```bash
mkdir -p debian/source
```

### 2. Create `debian/control`

This file defines package metadata and dependencies:

```
Source: chipmunk-tools
Section: electronics
Priority: optional
Maintainer: Tobi Delbruck <your-email@example.com>
Build-Depends: debhelper-compat (= 13), gcc, make, libx11-dev
Standards-Version: 4.6.0
Homepage: https://github.com/sensorsINI/chipmunk

Package: chipmunk-tools
Architecture: any
Depends: ${shlibs:Depends}, ${misc:Depends}, libx11-6, xfonts-base, xfonts-75dpi, xfonts-100dpi
Description: Analog and digital circuit simulation tools
 Chipmunk is a collection of software tools for Unix systems, including:
 .
  - Log: A schematic editor, analog and digital simulator, and netlist generator
  - Analog: Analog circuit simulation tool
  - Diglog: Digital circuit simulation tool
  - Loged: Gate editor for creating custom gate icons
 .
 The Analog simulator is particularly useful for interactive circuit
 simulation, providing real-time feedback like a circuit on a bench.
 .
 This package includes all binaries, configuration files, and lesson
 circuits needed to use the Chipmunk tools.
```

### 3. Create `debian/rules`

This is the build script (makefile) that calls your existing Makefile:

```makefile
#!/usr/bin/make -f
# debian/rules for chipmunk-tools

export DH_VERBOSE = 1

%:
	dh $@

override_dh_auto_build:
	$(MAKE) -C $(CURDIR) build

override_dh_auto_install:
	$(MAKE) -C $(CURDIR) install DESTDIR=$(CURDIR)/debian/chipmunk-tools
	# Install binaries to /usr/bin
	mkdir -p $(CURDIR)/debian/chipmunk-tools/usr/bin
	cp $(CURDIR)/bin/diglog $(CURDIR)/bin/loged $(CURDIR)/bin/fixfet7 \
	   $(CURDIR)/debian/chipmunk-tools/usr/bin/
	# Install analog wrapper script (needs path update)
	cp $(CURDIR)/bin/analog $(CURDIR)/debian/chipmunk-tools/usr/bin/
	# Install configuration files
	mkdir -p $(CURDIR)/debian/chipmunk-tools/usr/share/chipmunk-tools/lib
	cp -r $(CURDIR)/log/lib/* $(CURDIR)/debian/chipmunk-tools/usr/share/chipmunk-tools/lib/
	# Install documentation
	mkdir -p $(CURDIR)/debian/chipmunk-tools/usr/share/doc/chipmunk-tools
	cp $(CURDIR)/README.md $(CURDIR)/CHANGELOG.md \
	   $(CURDIR)/debian/chipmunk-tools/usr/share/doc/chipmunk-tools/

override_dh_fixperms:
	# Ensure binaries are executable
	chmod +x $(CURDIR)/debian/chipmunk-tools/usr/bin/*
	dh_fixperms

override_dh_strip:
	dh_strip --exclude=analog
```

**Important**: Make `debian/rules` executable:
```bash
chmod +x debian/rules
```

### 4. Create `debian/changelog`

Version history for Debian. Update this when creating new package versions:

```
chipmunk-tools (6.0.0-1) unstable; urgency=medium

  * Initial Debian package release
  * Based on chipmunk-tools 6.0.0
  * Includes analog, diglog, loged, and fixfet7 binaries
  * Includes all configuration files and lesson circuits

 -- Tobi Delbruck <your-email@example.com>  Mon, 01 Jan 2024 12:00:00 +0000
```

Generate changelog automatically:
```bash
dch --create --package chipmunk-tools --newversion 6.0.0-1
```

### 5. Create `debian/copyright`

License information:

```
Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: chipmunk-tools
Source: https://github.com/sensorsINI/chipmunk

Files: *
Copyright: 1985, 1990 David Gillespie, John Lazzaro, Rick Koshi,
 Glenn Gribble, Adam Greenblatt, Maryann Maher
License: GPL-1+

Files: debian/*
Copyright: 2024 Tobi Delbruck
License: GPL-1+

License: GPL-1+
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation (any version).
 .
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 .
 On Debian systems, the complete text of the GNU General Public
 License version 1 can be found in `/usr/share/common-licenses/GPL-1'.
```

### 6. Create `debian/compat`

Debian helper compatibility level (usually 13 for recent Debian/Ubuntu):

```
13
```

### 7. Create `debian/source/format`

Source format (native package):

```
3.0 (native)
```

### 8. Update Wrapper Script for System Paths

The `bin/analog` wrapper script needs to be updated to use system paths instead of relative paths. 

**Option A**: Modify `bin/analog` to detect installation location:
```bash
#!/bin/bash
# Wrapper script for analog - system-installed version

# Determine installation directory
if [ -f /usr/share/chipmunk-tools/lib/analog.cnf ]; then
    # System installation
    export LOGLIB="/usr/share/chipmunk-tools/lib"
    DIGLOG_BIN="/usr/bin/diglog"
elif [ -f "$(dirname "$0")/../log/lib/analog.cnf" ]; then
    # Local/git installation
    CHIPMUNK_DIR="$(cd "$(dirname "$0")/.." && pwd)"
    export LOGLIB="${CHIPMUNK_DIR}/log/lib"
    DIGLOG_BIN="${CHIPMUNK_DIR}/bin/diglog"
else
    echo "Error: Cannot find chipmunk-tools installation" >&2
    exit 1
fi

export CHIPMUNK_MODE=analog

# ... rest of script ...
```

**Option B**: Create a separate wrapper for the package:
- Keep original `bin/analog` for git/clone usage
- Create `debian/analog.system` for system installation
- Copy `analog.system` as `analog` in the package

### 9. Create `debian/postinst` (Optional)

Post-installation script to handle font rehashing:

```bash
#!/bin/bash
# debian/postinst - post-installation script

set -e

case "$1" in
    configure)
        # Refresh X11 font cache if X server is running
        if [ -n "$DISPLAY" ] && command -v xset >/dev/null 2>&1; then
            xset fp rehash 2>/dev/null || true
        fi
        echo ""
        echo "chipmunk-tools installed successfully!"
        echo "Run 'analog' to start the analog circuit simulator."
        echo ""
        echo "Note: If fonts are not working, run: xset fp rehash"
        ;;
esac

exit 0
```

Make executable:
```bash
chmod +x debian/postinst
```

## Building the Package

### 1. Clean Build Environment

```bash
make clean
```

### 2. Build the Package

```bash
dpkg-buildpackage -us -uc -b
```

This creates:
- `../chipmunk-tools_6.0.0-1_amd64.deb` - The installable package
- `../chipmunk-tools_6.0.0-1_amd64.changes` - Change log for repository
- `../chipmunk-tools_6.0.0-1_amd64.buildinfo` - Build information

### 3. Alternative: Use debuild (recommended)

```bash
debuild -us -uc -b
```

This runs lintian checks and provides better output.

## Installing and Testing

### Install the Package

```bash
sudo dpkg -i ../chipmunk-tools_6.0.0-1_amd64.deb
```

If dependencies are missing:
```bash
sudo apt-get install -f
```

### Test Installation

```bash
# Check binaries are in PATH
which analog diglog loged fixfet7

# Test analog
analog --help

# Verify configuration files
ls /usr/share/chipmunk-tools/lib/

# Test running analog
analog
```

### Uninstall

```bash
sudo apt-get remove chipmunk-tools
```

## Distribution Options

### Option 1: Standalone `.deb` File (Simple)

- Build `.deb` file
- Host on GitHub Releases
- Users download and install: `sudo dpkg -i chipmunk-tools.deb`

### Option 2: APT Repository (Advanced)

To enable `sudo apt install chipmunk-tools`:

1. **Host Repository**: Set up web server or use GitHub Releases with proper structure

2. **Generate GPG Key** (if signing):
```bash
gpg --gen-key
gpg --export --armor YOUR_EMAIL > public.key
```

3. **Create Repository Structure**:
```
dists/
  stable/
    main/
      binary-amd64/
        Packages
        Packages.gz
      source/
        Sources
        Sources.gz
  stable/Release
  stable/Release.gpg
pool/
  main/
    c/
      chipmunk-tools/
        chipmunk-tools_6.0.0-1_amd64.deb
```

4. **Generate Repository Metadata**:
```bash
dpkg-scanpackages pool/ > dists/stable/main/binary-amd64/Packages
gzip -k dists/stable/main/binary-amd64/Packages
```

5. **Create Release File and Sign**:
```bash
apt-ftparchive release dists/stable/ > dists/stable/Release
gpg --armor --sign --detach-sig dists/stable/Release
```

6. **Users Add Repository**:
```bash
echo "deb https://github.com/sensorsINI/chipmunk/releases/download/deb/ stable main" | sudo tee /etc/apt/sources.list.d/chipmunk.list
wget -qO - https://github.com/sensorsINI/chipmunk/releases/download/deb/public.key | sudo apt-key add -
sudo apt-get update
sudo apt-get install chipmunk-tools
```

**Note**: GitHub Releases doesn't support proper APT repository structure natively. Consider:
- Using a separate repository branch for packages
- Using a dedicated package hosting service
- Self-hosting a simple web server

## Version Management

- Update `VERSION` file in repo root
- Update `debian/changelog` with new version
- Rebuild package with new version number

To increment version:
```bash
dch -i  # Edit changelog interactively
```

## Troubleshooting

### Common Issues

1. **Missing dependencies**: Add to `Depends:` in `debian/control`

2. **Binary not found after install**: Check paths in wrapper script

3. **Font errors**: Ensure xfonts packages are installed and run `xset fp rehash`

4. **Path issues**: Verify wrapper scripts use absolute paths or proper detection

5. **Build errors**: Check `debian/rules` calls correct Makefile targets

### Debugging Package Contents

Inspect package before installing:
```bash
dpkg -c chipmunk-tools_6.0.0-1_amd64.deb
dpkg-deb -I chipmunk-tools_6.0.0-1_amd64.deb
```

Extract without installing:
```bash
dpkg-deb -x chipmunk-tools_6.0.0-1_amd64.deb /tmp/extract
ls -la /tmp/extract
```

## Maintenance

### Updating the Package

1. Update source code
2. Update `VERSION` file
3. Update `debian/changelog`: `dch -i`
4. Rebuild: `dpkg-buildpackage -us -uc -b`
5. Test new package
6. Release

### Integration with Release Script

Consider updating `scripts/package-release.sh` to also build `.deb`:

```bash
# Add to package-release.sh after tar.gz creation:
echo "Building Debian package..."
dpkg-buildpackage -us -uc -b
mv ../chipmunk-tools_*.deb "${REPO_ROOT}/"
```

## References

- [Debian Packaging Guide](https://www.debian.org/doc/manuals/packaging-tutorial/)
- [Debian Policy Manual](https://www.debian.org/doc/debian-policy/)
- [dh_debhelper manpage](https://manpages.debian.org/testing/debhelper/dh.1.en.html)
- [Creating a Debian Repository](https://wiki.debian.org/HowToSetupADebianRepository)

## Summary

Creating a Debian package requires:
- **Initial Setup**: 2-4 hours to create `debian/` directory structure
- **Updates**: 30 minutes per release to update changelog and rebuild
- **Complexity**: Medium (standard C project with Makefiles)

The main work is:
1. Creating `debian/` directory with standard files
2. Updating wrapper scripts for system paths
3. Testing installation and functionality

Once set up, packaging new versions is straightforward.
