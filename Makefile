# Top-level Makefile for Chipmunk Tools
# This Makefile orchestrates the build of psys libraries and log tools
#
# Version: 6.0.0
# Base Chipmunk Version: 5.66

.PHONY: all build clean install default help check

# Sentinel file to track successful requirements check
REQUIREMENTS_CHECKED := .requirements_checked

# Default target
default: all

# Build everything (psys must be built before log)
all: build

build: $(REQUIREMENTS_CHECKED) psys/src/libp2c.a bin/diglog
	@echo ""
	@echo "Build complete! You can now run: ./bin/analog"
	@./scripts/path_setup.sh || true

# Build psys libraries (required before building log)
psys/src/libp2c.a:
	@echo "Building psys libraries..."
	@echo "Note: Format-overflow warnings from legacy 1980s code are expected and suppressed."
	@$(MAKE) -C psys/src install 2>&1 | grep -v "gets.*function is dangerous" || true

# Build log tools (depends on psys)
bin/diglog: psys/src/libp2c.a
	@echo "Building log tools..."
	@echo "Note: Format-overflow warnings from legacy 1980s code are expected and suppressed."
	@$(MAKE) -C log/src install 2>&1 | grep -v "gets.*function is dangerous" || true

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

# Run check_requirements.sh and create sentinel file on success
$(REQUIREMENTS_CHECKED):
	@echo "Checking requirements..."
	@if ./check_requirements.sh; then \
		touch $(REQUIREMENTS_CHECKED); \
		echo ""; \
		echo "Requirements check passed. Proceeding with build..."; \
		echo ""; \
	else \
		echo ""; \
		echo "Requirements check failed. Please fix the issues above and try again."; \
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
	@echo "  make          - Build everything (default)"
	@echo "  make build    - Build everything"
	@echo "  make clean    - Remove all build artifacts"
	@echo "  make install  - Build and install everything"
	@echo "  make check    - Check system requirements (compiler, X11, fonts, etc.)"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "After building, run: ./bin/analog"
