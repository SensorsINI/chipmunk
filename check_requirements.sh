#!/bin/bash
# Check script for Chipmunk Tools requirements
# Verifies fonts, X11 display, and environment setup

echo "=== Chipmunk Tools Requirements Check ==="
echo ""

ERRORS=0
WARNINGS=0

# Check X11 display
echo "1. Checking X11 Display..."
X11_FAILED=0
if [ -z "$DISPLAY" ]; then
    echo "   ✗ DISPLAY environment variable not set"
    X11_FAILED=1
    ERRORS=$((ERRORS + 1))
else
    echo "   ✓ DISPLAY is set: $DISPLAY"
    if xdpyinfo >/dev/null 2>&1; then
        echo "   ✓ X server is accessible"
    else
        echo "   ✗ Cannot connect to X server"
        X11_FAILED=1
        ERRORS=$((ERRORS + 1))
    fi
fi

# Check if running on WSL (Windows Subsystem for Linux)
IS_WSL=0
if [ -f /proc/version ] && grep -qi microsoft /proc/version 2>/dev/null; then
    IS_WSL=1
elif [ -n "$WSL_DISTRO_NAME" ]; then
    IS_WSL=1
elif uname -r 2>/dev/null | grep -qi microsoft; then
    IS_WSL=1
fi

# If X11 failed and we're on WSL, provide specific advice
if [ $X11_FAILED -eq 1 ] && [ $IS_WSL -eq 1 ]; then
    echo ""
    echo "   ⚠ Detected WSL (Windows Subsystem for Linux)"
    echo "   WSL does not include an X11 server by default."
    echo ""
    echo "   To use Chipmunk tools on WSL, you need to install an X11 server on Windows:"
    echo ""
    echo "   RECOMMENDED: VcXsrv (free, open-source, easy to use)"
    echo "   1. Download from: https://sourceforge.net/projects/vcxsrv/"
    echo "   2. Install and run 'XLaunch'"
    echo "   3. Select 'Multiple windows' or 'One large window'"
    echo "   4. Check 'Disable access control' (for simplicity)"
    echo "   5. Click 'Finish'"
    echo "   6. In WSL, set: export DISPLAY=\$(cat /etc/resolv.conf | grep nameserver | awk '{print \$2}'):0"
    echo "   7. Or add to ~/.bashrc: export DISPLAY=\$(cat /etc/resolv.conf | grep nameserver | awk '{print \$2}'):0"
    echo ""
    echo "   Alternative: X410 (paid, $9.99, available on Microsoft Store)"
    echo "   - More polished but requires purchase"
    echo "   - Available at: https://www.microsoft.com/store/productId/9NL6M1ZSP5WG"
    echo ""
fi
echo ""

# Check required fonts
echo "2. Checking X11 Fonts..."
REQUIRED_FONTS=("6x10" "8x13")
MISSING_FONTS=()

for font in "${REQUIRED_FONTS[@]}"; do
    if xlsfonts 2>/dev/null | grep -q "^${font}$"; then
        echo "   ✓ Font '$font' is available"
    else
        echo "   ✗ Font '$font' is NOT available"
        MISSING_FONTS+=("$font")
        ERRORS=$((ERRORS + 1))
    fi
done

if [ ${#MISSING_FONTS[@]} -gt 0 ]; then
    echo ""
    echo "   To install missing fonts, run:"
    echo "   sudo apt-get install xfonts-base xfonts-75dpi xfonts-100dpi"
    echo "   xset fp rehash"
fi
echo ""

# Check LOGLIB environment variable
echo "3. Checking LOGLIB environment variable..."
if [ -z "$LOGLIB" ]; then
    echo "   ⚠ LOGLIB not set (will be set automatically by wrapper script)"
    WARNINGS=$((WARNINGS + 1))
else
    echo "   ✓ LOGLIB is set: $LOGLIB"
    if [ -d "$LOGLIB" ]; then
        echo "   ✓ LOGLIB directory exists"
        if [ -f "$LOGLIB/analog.cnf" ]; then
            echo "   ✓ analog.cnf found"
        else
            echo "   ⚠ analog.cnf not found in LOGLIB"
            WARNINGS=$((WARNINGS + 1))
        fi
    else
        echo "   ✗ LOGLIB directory does not exist"
        ERRORS=$((ERRORS + 1))
    fi
fi
echo ""

# Check if binaries exist
echo "4. Checking Chipmunk binaries..."
CHIPMUNK_DIR="$(cd "$(dirname "$0")" && pwd)"
if [ -f "$CHIPMUNK_DIR/bin/analog" ]; then
    echo "   ✓ analog script exists"
    if [ -x "$CHIPMUNK_DIR/bin/analog" ]; then
        echo "   ✓ analog script is executable"
    else
        echo "   ✗ analog script is not executable"
        ERRORS=$((ERRORS + 1))
    fi
else
    echo "   ✗ analog script not found"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "$CHIPMUNK_DIR/bin/diglog" ]; then
    echo "   ✓ diglog binary exists"
    if [ -x "$CHIPMUNK_DIR/bin/diglog" ]; then
        echo "   ✓ diglog binary is executable"
    else
        echo "   ✗ diglog binary is not executable"
        ERRORS=$((ERRORS + 1))
    fi
else
    echo "   ✗ diglog binary not found (run 'make' to build)"
    ERRORS=$((ERRORS + 1))
fi
echo ""

# Check required font packages (Ubuntu/Debian)
echo "5. Checking font packages..."
if command -v dpkg >/dev/null 2>&1; then
    FONT_PACKAGES=("xfonts-base" "xfonts-75dpi" "xfonts-100dpi")
    for pkg in "${FONT_PACKAGES[@]}"; do
        if dpkg -l | grep -q "^ii.*${pkg}"; then
            echo "   ✓ Package '$pkg' is installed"
        else
            echo "   ⚠ Package '$pkg' may not be installed"
            WARNINGS=$((WARNINGS + 1))
        fi
    done
else
    echo "   ⚠ Cannot check packages (dpkg not available)"
    WARNINGS=$((WARNINGS + 1))
fi
echo ""

# Summary
echo "=== Summary ==="
if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo "✓ All checks passed! Chipmunk tools should work correctly."
    exit 0
elif [ $ERRORS -eq 0 ]; then
    echo "⚠ Checks passed with $WARNINGS warning(s). Chipmunk tools should work."
    exit 0
else
    echo "✗ Found $ERRORS error(s) and $WARNINGS warning(s)."
    echo "  Please fix the errors before running analog."
    exit 1
fi

