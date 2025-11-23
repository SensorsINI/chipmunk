# Top-level Makefile for Chipmunk Tools
# This Makefile orchestrates the build of psys libraries and log tools
#
# Version: 6.1.0
# Base Chipmunk Version: 5.66

# Use bash for targets that need bash features
SHELL := /bin/bash

.PHONY: all build clean install default help check setup install-deps uninstall uninstall-deps uninstall-rc uninstall-bashrc

# Sentinel file to track successful requirements check
REQUIREMENTS_CHECKED := .requirements_checked

# Default target
default: all

# Source file dependencies for rebuild detection
PSYS_SRC_FILES := $(wildcard psys/src/*.c) $(wildcard psys/src/*.h)
PSYS_INCLUDE_FILES := $(wildcard psys/include/p2c/*.h)
PSYS_MAKEFILE := psys/src/Makefile
LOG_SRC_FILES := $(wildcard log/src/*.c) $(wildcard log/src/*.h) $(wildcard log/src/ana/*.c) $(wildcard log/src/ana/*.h)
LOG_MAKEFILE := log/src/Makefile

# Build everything (psys must be built before log)
all: build

build: $(REQUIREMENTS_CHECKED) psys/src/libp2c.a bin/diglog
	@echo ""
	@echo "Build complete! You can now run: ./bin/analog"
	@./scripts/path_setup.sh || true

# Build psys libraries (required before building log)
# Depend on all source files, include headers, and Makefile so rebuilds trigger on any change
psys/src/libp2c.a: $(PSYS_SRC_FILES) $(PSYS_INCLUDE_FILES) $(PSYS_MAKEFILE)
	@echo "Building psys libraries..."
	@echo "Note: Format-overflow warnings from legacy 1980s code are expected and suppressed."
	@set -o pipefail; $(MAKE) -C psys/src install 2>&1 | { grep -v "gets.*function is dangerous" || test $$? = 1; }

# Build log tools (depends on psys and all log source files)
# Depend on all source files so any change triggers rebuild
log/src/log: psys/src/libp2c.a $(LOG_SRC_FILES) $(LOG_MAKEFILE)
	@echo "Building log tools..."
	@echo "Note: Format-overflow warnings from legacy 1980s code are expected and suppressed."
	@set -o pipefail; $(MAKE) -C log/src all 2>&1 | { grep -v "gets.*function is dangerous" || test $$? = 1; }

# Install log binary to bin/diglog (copies from log/src/log)
# Make will only run this if log/src/log is newer than bin/diglog
bin/diglog: log/src/log
	@set -o pipefail; $(MAKE) -C log/src install 2>&1 | { grep -v "gets.*function is dangerous" || test $$? = 1; }

# Clean all build artifacts
clean:
	@echo "Cleaning psys..."
	$(MAKE) -C psys/src clean
	@echo "Cleaning log..."
	$(MAKE) -C log/src clean
	@echo "Removing requirements check sentinel..."
	@rm -f $(REQUIREMENTS_CHECKED)
	@echo "Clean complete!"

# Install everything (same as build)
install: build

# Setup: Automatically install missing dependencies and build
setup: install-deps
	@echo ""
	@echo "Setup complete! Building Chipmunk tools..."
	@$(MAKE) build

# Install dependencies automatically (non-interactive)
install-deps:
	@echo "Installing dependencies automatically..."
	@if ./check_requirements.sh -y; then \
		touch $(REQUIREMENTS_CHECKED); \
		echo ""; \
		echo "Dependencies installed successfully."; \
		echo ""; \
	else \
		echo ""; \
		echo "Dependency installation failed. Please check the errors above."; \
		exit 1; \
	fi

# Uninstall dependencies (remove packages installed by install-deps)
uninstall-deps:
	@echo "Uninstalling Chipmunk dependencies..."
	@echo ""
	@PACKAGES_TO_REMOVE=""; \
	if dpkg -l | grep -q "^ii.*x11-utils"; then \
		PACKAGES_TO_REMOVE="$$PACKAGES_TO_REMOVE x11-utils"; \
	fi; \
	if dpkg -l | grep -q "^ii.*xfonts-base"; then \
		PACKAGES_TO_REMOVE="$$PACKAGES_TO_REMOVE xfonts-base"; \
	fi; \
	if dpkg -l | grep -q "^ii.*xfonts-75dpi"; then \
		PACKAGES_TO_REMOVE="$$PACKAGES_TO_REMOVE xfonts-75dpi"; \
	fi; \
	if dpkg -l | grep -q "^ii.*xfonts-100dpi"; then \
		PACKAGES_TO_REMOVE="$$PACKAGES_TO_REMOVE xfonts-100dpi"; \
	fi; \
	if [ -n "$$PACKAGES_TO_REMOVE" ]; then \
		echo "Packages to remove:$$PACKAGES_TO_REMOVE"; \
		echo ""; \
		sudo apt-get remove -y $$PACKAGES_TO_REMOVE; \
		echo ""; \
		echo "✓ Dependencies uninstalled successfully."; \
	else \
		echo "No Chipmunk dependencies found to uninstall."; \
	fi; \
	echo ""; \
	if dpkg -l | grep -q "^ii.*analog[[:space:]]"; then \
		echo "⚠ NOTE: System 'analog' package detected (web server log analyzer)"; \
		echo "   This is NOT related to Chipmunk and will conflict if both are in PATH."; \
		echo "   If you want to remove it: sudo apt-get remove analog"; \
	fi

# Uninstall: Remove shell RC file changes and optionally dependencies
uninstall: uninstall-rc
	@echo ""
	@read -p "Also uninstall dependencies (x11-utils, xfonts packages)? [y/N] " -n 1 -r; \
	echo ""; \
	if [[ $$REPLY =~ ^[Yy]$$ ]]; then \
		$(MAKE) uninstall-deps; \
	fi

# Remove DISPLAY export and PATH from shell RC file (.bashrc or .zshrc)
uninstall-rc:
	@# Detect user's default shell and corresponding RC file \
	USER_SHELL="$${SHELL:-/bin/bash}"; \
	if [[ "$$USER_SHELL" == */zsh ]]; then \
		RC_FILE=~/.zshrc; \
		SHELL_NAME="zsh"; \
	elif [[ "$$USER_SHELL" == */bash ]] || [[ "$$USER_SHELL" == */sh ]]; then \
		RC_FILE=~/.bashrc; \
		SHELL_NAME="bash"; \
	else \
		RC_FILE=~/.bashrc; \
		SHELL_NAME="bash"; \
	fi; \
	echo "Removing Chipmunk changes from $$RC_FILE ($$SHELL_NAME)..."; \
	if [ ! -f $$RC_FILE ]; then \
		echo "$$RC_FILE not found. Nothing to remove."; \
		exit 0; \
	fi; \
	BACKUP_FILE=$$RC_FILE.chipmunk-backup-$$(date +%Y%m%d-%H%M%S); \
	cp $$RC_FILE $$BACKUP_FILE; \
	echo "Created backup: $$BACKUP_FILE"; \
	REMOVED=0; \
	\
	# Remove chipmunk PATH block (between markers) \
	MARKER_BEGIN="# >>> chipmunk PATH (added by build) >>>"; \
	MARKER_END="# <<< chipmunk PATH (added by build) <<<"; \
	if grep -qF "$$MARKER_BEGIN" $$RC_FILE 2>/dev/null; then \
		awk -v b="$$MARKER_BEGIN" -v e="$$MARKER_END" ' \
			$$0==b{skip=1; next} $$0==e{skip=0; next} !skip{print} \
		' $$RC_FILE > $$RC_FILE.chipmunk.tmp && mv $$RC_FILE.chipmunk.tmp $$RC_FILE; \
		REMOVED=1; \
		echo "✓ Removed Chipmunk PATH export from $$RC_FILE"; \
	fi; \
	\
	# Remove lines matching DISPLAY export patterns (Linux/WSL-specific, check both rc files) \
	for RC in ~/.bashrc ~/.zshrc; do \
		if [ -f $$RC ]; then \
			RC_REMOVED=0; \
			# Pattern 1: export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0 \
			if grep -q "export DISPLAY.*resolv.conf" $$RC 2>/dev/null; then \
				sed -i '/export DISPLAY.*resolv\.conf/d' $$RC; \
				RC_REMOVED=1; \
				echo "✓ Removed DISPLAY export (resolv.conf pattern) from $$RC"; \
			fi; \
			# Pattern 2: export DISPLAY=IP:0 or export DISPLAY=IP:0.0 \
			if grep -qE "^[[:space:]]*export DISPLAY=[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+:0" $$RC 2>/dev/null; then \
				if sed --version >/dev/null 2>&1; then \
					sed -i -E '/^[[:space:]]*export DISPLAY=[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+:0/d' $$RC; \
				else \
					sed -iE '/^[[:space:]]*export DISPLAY=[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+:0/d' $$RC; \
				fi; \
				RC_REMOVED=1; \
				echo "✓ Removed DISPLAY export (IP:0 pattern) from $$RC"; \
			fi; \
			# Pattern 3: export DISPLAY=IP:0.0 \
			if grep -qE "^[[:space:]]*export DISPLAY=[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+:0\.0" $$RC 2>/dev/null; then \
				if sed --version >/dev/null 2>&1; then \
					sed -i -E '/^[[:space:]]*export DISPLAY=[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+:0\.0/d' $$RC; \
				else \
					sed -iE '/^[[:space:]]*export DISPLAY=[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+:0\.0/d' $$RC; \
				fi; \
				RC_REMOVED=1; \
				echo "✓ Removed DISPLAY export (IP:0.0 pattern) from $$RC"; \
			fi; \
			if [ $$RC_REMOVED -eq 1 ]; then \
				REMOVED=1; \
			fi; \
		fi; \
	done; \
	\
	if [ $$REMOVED -eq 1 ]; then \
		echo ""; \
		echo "✓ Chipmunk changes removed from shell RC file(s)"; \
		echo "  Backup saved to: $$BACKUP_FILE"; \
		if [[ "$$SHELL_NAME" == "zsh" ]]; then \
			echo "  Note: Open a new terminal or run 'source $$RC_FILE' to apply changes."; \
		else \
			echo "  Note: Open a new terminal or run '. $$RC_FILE' to apply changes."; \
		fi; \
	else \
		echo "No Chipmunk changes found in shell RC file(s)"; \
		rm -f $$BACKUP_FILE; \
	fi

# Backward compatibility alias for uninstall-bashrc
uninstall-bashrc: uninstall-rc

# Run check_requirements.sh and create sentinel file on success
$(REQUIREMENTS_CHECKED):
	@echo "Checking requirements..."
	@CHECK_FLAGS=""; \
	if [ -n "$$CI" ] || [ -n "$$GITHUB_ACTIONS" ]; then \
		CHECK_FLAGS="--build-only"; \
	fi; \
	if ./check_requirements.sh $$CHECK_FLAGS; then \
		touch $(REQUIREMENTS_CHECKED); \
		echo ""; \
		echo "Requirements check passed. Proceeding with build..."; \
		echo ""; \
	else \
		echo ""; \
		echo "Requirements check failed. Please fix the issues above and try again."; \
		echo "  Or run 'make setup' to automatically install missing dependencies."; \
		exit 1; \
	fi

# Check system requirements (compiler, X11, fonts, etc.)
check:
	@echo "Checking Chipmunk build dependencies..."
	@echo ""
	@echo "=== C Compiler ==="
	@if command -v gcc >/dev/null 2>&1; then \
		echo "✓ GCC found: $$(gcc --version | head -1)"; \
	else \
		echo "✗ GCC not found - install with: sudo apt-get install gcc"; \
		EXIT_CODE=1; \
	fi
	@echo ""
	@echo "=== X11 Development Libraries ==="
	@if pkg-config --exists x11 2>/dev/null; then \
		echo "✓ X11 library found (version $$(pkg-config --modversion x11))"; \
	elif [ -f /usr/include/X11/X.h ] || [ -f /usr/include/X11/Xlib.h ]; then \
		echo "✓ X11 headers found"; \
		if ldconfig -p 2>/dev/null | grep -q libX11.so || [ -f /usr/lib/x86_64-linux-gnu/libX11.so ] || [ -f /usr/lib/libX11.so ]; then \
			echo "✓ X11 library found"; \
		else \
			echo "✗ X11 library not found - install with: sudo apt-get install libx11-dev"; \
			EXIT_CODE=1; \
		fi; \
	else \
		echo "✗ X11 development package not found"; \
		echo "  Install with: sudo apt-get install libx11-dev"; \
		EXIT_CODE=1; \
	fi
	@echo ""
	@echo "=== Math Library ==="
	@if [ -f /usr/lib/x86_64-linux-gnu/libm.so ] || [ -f /usr/lib/libm.so ] || ldconfig -p 2>/dev/null | grep -q libm.so; then \
		echo "✓ Math library (libm) found"; \
	else \
		echo "✗ Math library not found (usually part of glibc)"; \
		EXIT_CODE=1; \
	fi
	@echo ""
	@echo "=== X11 Fonts ==="
	@if command -v xlsfonts >/dev/null 2>&1; then \
		if xlsfonts 2>/dev/null | grep -q "^6x10$$"; then \
			echo "✓ Font '6x10' found"; \
		else \
			echo "✗ Font '6x10' not found"; \
			echo "  Install with: sudo apt-get install xfonts-base xfonts-75dpi xfonts-100dpi"; \
			echo "  Then run: xset fp rehash"; \
			EXIT_CODE=1; \
		fi; \
		if xlsfonts 2>/dev/null | grep -q "^8x13$$"; then \
			echo "✓ Font '8x13' found"; \
		else \
			echo "✗ Font '8x13' not found"; \
			echo "  Install with: sudo apt-get install xfonts-base xfonts-75dpi xfonts-100dpi"; \
			echo "  Then run: xset fp rehash"; \
			EXIT_CODE=1; \
		fi; \
	else \
		echo "⚠ Cannot check fonts (xlsfonts not available - may need X server running)"; \
		echo "  Required fonts: 6x10, 8x13"; \
		echo "  Install with: sudo apt-get install xfonts-base xfonts-75dpi xfonts-100dpi"; \
	fi
	@echo ""
	@echo "=== Build System ==="
	@if command -v make >/dev/null 2>&1; then \
		echo "✓ Make found: $$(make --version | head -1)"; \
	else \
		echo "✗ Make not found - install with: sudo apt-get install make"; \
		EXIT_CODE=1; \
	fi
	@if command -v ar >/dev/null 2>&1; then \
		echo "✓ ar (archive tool) found"; \
	else \
		echo "✗ ar not found - install with: sudo apt-get install binutils"; \
		EXIT_CODE=1; \
	fi
	@echo ""
	@echo "=== Summary ==="
	@echo "Chipmunk requires:"
	@echo "  - GCC C compiler (ANSI C compatible)"
	@echo "  - X11/Xlib development libraries (libX11)"
	@echo "  - Math library (libm, usually part of glibc)"
	@echo "  - X11 fonts: 6x10, 8x13 (xfonts-base, xfonts-75dpi, xfonts-100dpi)"
	@echo "  - Build tools: make, ar, ranlib"
	@echo ""
	@if [ -z "$$EXIT_CODE" ]; then \
		echo "✓ All dependencies satisfied!"; \
		echo ""; \
		echo "You can now run: make build"; \
	else \
		echo "✗ Some dependencies are missing. Please install them before building."; \
		exit 1; \
	fi

# Help target
help:
	@echo "Chipmunk Tools Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build everything (default)"
	@echo "  make build        - Build everything"
	@echo "  make setup        - Automatically install dependencies and build"
	@echo "  make install-deps - Automatically install missing dependencies (non-interactive)"
	@echo "  make uninstall    - Remove shell RC file (.bashrc/.zshrc) changes and optionally uninstall dependencies"
	@echo "  make uninstall-deps - Uninstall dependencies (x11-utils, xfonts packages)"
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make install      - Build and install everything"
	@echo "  make check        - Check system requirements (compiler, X11, fonts, etc.)"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "After building, run: ./bin/analog"
