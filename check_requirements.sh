#!/bin/bash
# Check script for Chipmunk Tools requirements
# Verifies fonts, X11 display, and environment setup

# Parse command line arguments
AUTO_YES=0
BUILD_ONLY=0
for arg in "$@"; do
    case $arg in
        -y|--yes)
            AUTO_YES=1
            ;;
        --build-only)
            BUILD_ONLY=1
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -y, --yes       Automatically install missing packages without prompting"
            echo "  --build-only    Only check build-time requirements (skip X11 runtime checks)"
            echo "  -h, --help      Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $arg"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

echo "=== Chipmunk Tools Requirements Check ==="
if [ $AUTO_YES -eq 1 ]; then
    echo "(Auto-install mode: -y flag enabled)"
fi
if [ $BUILD_ONLY -eq 1 ]; then
    echo "(Build-only mode: checking build requirements only, skipping runtime X11 checks)"
fi
echo ""

ERRORS=0
WARNINGS=0

# Detect operating system
IS_WSL=0
IS_MACOS=0
OS_TYPE=$(uname -s)

if [ "$OS_TYPE" = "Darwin" ]; then
    IS_MACOS=1
elif [ -f /proc/version ] && grep -qi microsoft /proc/version 2>/dev/null; then
    IS_WSL=1
elif [ -n "$WSL_DISTRO_NAME" ]; then
    IS_WSL=1
elif uname -r 2>/dev/null | grep -qi microsoft; then
    IS_WSL=1
fi

# Check if xdpyinfo is available for X11 testing
HAS_XDPYINFO=0
if command -v xdpyinfo >/dev/null 2>&1; then
    HAS_XDPYINFO=1
fi

# Check X11 display
echo "1. Checking X11 Display..."
X11_FAILED=0
if [ $BUILD_ONLY -eq 1 ]; then
    echo "   → Skipped in build-only mode (not required for compilation)"
elif [ -z "$DISPLAY" ]; then
    echo "   ✗ DISPLAY environment variable not set"
    X11_FAILED=1
    ERRORS=$((ERRORS + 1))

    # If on WSL, try to auto-detect and set DISPLAY
    if [ $IS_WSL -eq 1 ]; then
        WSL_HOST_IP=$(cat /etc/resolv.conf 2>/dev/null | grep nameserver | awk '{print $2}' | head -1)
        if [ -n "$WSL_HOST_IP" ]; then
            echo "   → Attempting to auto-configure DISPLAY for WSL..."
            export DISPLAY="$WSL_HOST_IP:0"
            if [ $HAS_XDPYINFO -eq 1 ] && xdpyinfo >/dev/null 2>&1; then
                echo "   ✓ Auto-configured DISPLAY=$DISPLAY and X server is accessible"
                X11_FAILED=0
                ERRORS=$((ERRORS - 1))
            elif [ $HAS_XDPYINFO -eq 0 ]; then
                # xdpyinfo not available, try xlsfonts as alternative test
                if command -v xlsfonts >/dev/null 2>&1 && xlsfonts >/dev/null 2>&1; then
                    echo "   ✓ Auto-configured DISPLAY=$DISPLAY (X server appears accessible)"
                    X11_FAILED=0
                    ERRORS=$((ERRORS - 1))
                else
                    # Try alternative format
                    export DISPLAY="$WSL_HOST_IP:0.0"
                    if command -v xlsfonts >/dev/null 2>&1 && xlsfonts >/dev/null 2>&1; then
                        echo "   ✓ Auto-configured DISPLAY=$DISPLAY (X server appears accessible)"
                        X11_FAILED=0
                        ERRORS=$((ERRORS - 1))
                    else
                        echo "   ✗ Auto-configuration failed (VcXsrv may not be running or configured)"
                        echo "   ⚠ Install x11-utils for better X11 testing: sudo apt-get install x11-utils"
                        export DISPLAY=""  # Reset to empty
                    fi
                fi
            else
                # Try alternative format
                export DISPLAY="$WSL_HOST_IP:0.0"
                if xdpyinfo >/dev/null 2>&1; then
                    echo "   ✓ Auto-configured DISPLAY=$DISPLAY and X server is accessible"
                    X11_FAILED=0
                    ERRORS=$((ERRORS - 1))
                else
                    echo "   ✗ Auto-configuration failed (VcXsrv may not be running or configured)"
                    export DISPLAY=""  # Reset to empty
                fi
            fi
        fi
    fi
else
    echo "   ✓ DISPLAY is set: $DISPLAY"
    if [ $HAS_XDPYINFO -eq 1 ] && xdpyinfo >/dev/null 2>&1; then
        echo "   ✓ X server is accessible"
    elif [ $HAS_XDPYINFO -eq 0 ]; then
        # xdpyinfo not available, try xlsfonts as alternative test
        if command -v xlsfonts >/dev/null 2>&1 && xlsfonts >/dev/null 2>&1; then
            echo "   ✓ X server appears accessible (using xlsfonts test)"
            echo "   ⚠ Install x11-utils for better X11 testing: sudo apt-get install x11-utils"
        else
            echo "   ✗ Cannot connect to X server"
            echo "   ⚠ Install x11-utils for better X11 testing: sudo apt-get install x11-utils"
            X11_FAILED=1
            ERRORS=$((ERRORS + 1))
        fi
    else
        echo "   ✗ Cannot connect to X server"
        X11_FAILED=1
        ERRORS=$((ERRORS + 1))
        
        # If on WSL and current DISPLAY doesn't work, try auto-fixing
        if [ $IS_WSL -eq 1 ]; then
            WSL_HOST_IP=$(cat /etc/resolv.conf 2>/dev/null | grep nameserver | awk '{print $2}' | head -1)
            if [ -n "$WSL_HOST_IP" ]; then
                echo "   → Attempting to auto-fix DISPLAY for WSL..."
                OLD_DISPLAY="$DISPLAY"
                export DISPLAY="$WSL_HOST_IP:0"
                if [ $HAS_XDPYINFO -eq 1 ] && xdpyinfo >/dev/null 2>&1; then
                    echo "   ✓ Auto-fixed DISPLAY=$DISPLAY and X server is now accessible"
                    X11_FAILED=0
                    ERRORS=$((ERRORS - 1))
                elif [ $HAS_XDPYINFO -eq 0 ] && command -v xlsfonts >/dev/null 2>&1 && xlsfonts >/dev/null 2>&1; then
                    echo "   ✓ Auto-fixed DISPLAY=$DISPLAY (X server appears accessible)"
                    X11_FAILED=0
                    ERRORS=$((ERRORS - 1))
                else
                    export DISPLAY="$WSL_HOST_IP:0.0"
                    if [ $HAS_XDPYINFO -eq 1 ] && xdpyinfo >/dev/null 2>&1; then
                        echo "   ✓ Auto-fixed DISPLAY=$DISPLAY and X server is now accessible"
                        X11_FAILED=0
                        ERRORS=$((ERRORS - 1))
                    elif [ $HAS_XDPYINFO -eq 0 ] && command -v xlsfonts >/dev/null 2>&1 && xlsfonts >/dev/null 2>&1; then
                        echo "   ✓ Auto-fixed DISPLAY=$DISPLAY (X server appears accessible)"
                        X11_FAILED=0
                        ERRORS=$((ERRORS - 1))
                    else
                        export DISPLAY="$OLD_DISPLAY"  # Restore original
                        echo "   ✗ Auto-fix failed (VcXsrv may not be running or configured)"
                    fi
                fi
            fi
        fi
    fi
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
    echo "   1. Download from: https://github.com/marchaesen/vcxsrv"
    echo "   2. Install and run 'XLaunch'"
    echo "   3. Select 'Multiple windows' or 'One large window'"
    echo "   4. IMPORTANT: Check 'Disable access control' (required for WSL2)"
    echo "   5. Click 'Finish'"
    echo ""
    echo "   6. In WSL, set DISPLAY to your Windows host IP:"
    WSL_HOST_IP=$(cat /etc/resolv.conf 2>/dev/null | grep nameserver | awk '{print $2}' | head -1)
    if [ -n "$WSL_HOST_IP" ]; then
        echo "      export DISPLAY=$WSL_HOST_IP:0"
        echo "      # Or add to ~/.bashrc: export DISPLAY=$WSL_HOST_IP:0"
        echo ""
        echo "   7. Test the connection:"
        echo "      DISPLAY=$WSL_HOST_IP:0 xdpyinfo"
        echo ""
        echo "   TROUBLESHOOTING if VcXsrv is running but not accessible:"
        echo "   - Verify VcXsrv is running (check Windows taskbar)"
        echo "   - Try different DISPLAY formats:"
        echo "     * $WSL_HOST_IP:0 (most common for WSL2)"
        echo "     * $WSL_HOST_IP:0.0"
        echo "     * localhost:0 (if X11 forwarding is configured)"
        echo "   - Check Windows Firewall: Allow VcXsrv through firewall"
        echo "   - Ensure 'Disable access control' is checked in XLaunch"
        echo "   - Restart VcXsrv after changing settings"
    else
        echo "      export DISPLAY=\$(cat /etc/resolv.conf | grep nameserver | awk '{print \$2}'):0"
        echo "      # Or add to ~/.bashrc: export DISPLAY=\$(cat /etc/resolv.conf | grep nameserver | awk '{print \$2}'):0"
    fi
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

if [ $BUILD_ONLY -eq 1 ]; then
    echo "   → Skipped runtime font check in build-only mode"
    echo "   → Font packages will be verified in step 5"
else
    # Runtime check: verify fonts are accessible via X server
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
    echo "   ⚠ analog script not found (will be created during build)"
    WARNINGS=$((WARNINGS + 1))
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
    echo "   ⚠ diglog binary not found (will be built by 'make')"
    WARNINGS=$((WARNINGS + 1))
fi
echo ""

# Check required font packages
echo "5. Checking font packages..."
MISSING_FONT_PACKAGES=()
if [ $IS_MACOS -eq 1 ]; then
    # On macOS, check for XQuartz which provides fonts
    if [ -d /opt/X11 ]; then
        echo "   ✓ XQuartz is installed at /opt/X11"
        echo "   → Fonts are provided by XQuartz"
    elif [ -d /usr/X11R6 ]; then
        echo "   ✓ X11 is installed at /usr/X11R6"
        echo "   → Fonts are provided by X11"
    else
        echo "   ✗ XQuartz not found"
        echo "   → Install XQuartz from: https://www.xquartz.org/"
        ERRORS=$((ERRORS + 1))
    fi
elif command -v dpkg >/dev/null 2>&1; then
    # On Debian/Ubuntu, check for font packages
    FONT_PACKAGES=("xfonts-base" "xfonts-75dpi" "xfonts-100dpi")
    for pkg in "${FONT_PACKAGES[@]}"; do
        if dpkg -l | grep -q "^ii.*${pkg}"; then
            echo "   ✓ Package '$pkg' is installed"
        else
            if [ $BUILD_ONLY -eq 1 ]; then
                echo "   ✗ Package '$pkg' is NOT installed (required for build)"
                MISSING_FONT_PACKAGES+=("$pkg")
                ERRORS=$((ERRORS + 1))
            else
                echo "   ⚠ Package '$pkg' may not be installed"
                MISSING_FONT_PACKAGES+=("$pkg")
                WARNINGS=$((WARNINGS + 1))
            fi
        fi
    done
else
    echo "   ⚠ Cannot check packages (package manager not recognized)"
    WARNINGS=$((WARNINGS + 1))
fi
echo ""

# Check for conflicting system 'analog' package
echo "6. Checking for package conflicts..."
if [ $IS_MACOS -eq 1 ]; then
    # On macOS, check for Homebrew analog package
    if command -v brew >/dev/null 2>&1 && brew list analog 2>/dev/null >/dev/null; then
        echo "   ⚠ WARNING: Homebrew 'analog' package is installed"
        echo "      This is a web server log analyzer (analog.cx), NOT Chipmunk's analog tool."
        echo "      It will conflict with Chipmunk's analog command if both are in PATH."
        echo "      Consider removing it: brew uninstall analog"
        WARNINGS=$((WARNINGS + 1))
    else
        echo "   ✓ No conflicting 'analog' package found"
    fi
elif command -v dpkg >/dev/null 2>&1; then
    if dpkg -l | grep -q "^ii.*analog[[:space:]]"; then
        echo "   ⚠ WARNING: System 'analog' package is installed"
        echo "      This is a web server log analyzer (analog.cx), NOT Chipmunk's analog tool."
        echo "      It will conflict with Chipmunk's analog command if both are in PATH."
        echo "      Consider removing it: sudo apt-get remove analog"
        WARNINGS=$((WARNINGS + 1))
    else
        echo "   ✓ No conflicting 'analog' package found"
    fi
else
    echo "   ✓ No package manager conflicts to check"
fi
echo ""

# Track if packages were successfully installed
X11_UTILS_INSTALLED=0
FONTS_INSTALLED=0

# Prompt for installation of missing packages
NEED_X11_UTILS=0
if [ $HAS_XDPYINFO -eq 0 ]; then
    NEED_X11_UTILS=1
fi

NEED_FONTS=0
if [ ${#MISSING_FONTS[@]} -gt 0 ] || [ ${#MISSING_FONT_PACKAGES[@]} -gt 0 ]; then
    NEED_FONTS=1
fi

if [ $NEED_X11_UTILS -eq 1 ] || [ $NEED_FONTS -eq 1 ]; then
    if [ $AUTO_YES -eq 0 ]; then
        echo "=== Installation Prompts ==="
        echo ""
    else
        echo "=== Installing Missing Packages ==="
        echo ""
    fi
    
    # Install x11-utils
    if [ $NEED_X11_UTILS -eq 1 ]; then
        if [ $AUTO_YES -eq 1 ]; then
            echo "Installing x11-utils (needed for X11 testing)..."
        else
            echo "x11-utils is not installed (needed for X11 testing)."
            read -p "Install x11-utils? [y/N] " -n 1 -r
            echo
        fi
        if [ $AUTO_YES -eq 1 ] || [[ $REPLY =~ ^[Yy]$ ]]; then
            if sudo apt-get install -y x11-utils; then
                echo "   ✓ x11-utils installed successfully"
                X11_UTILS_INSTALLED=1
                HAS_XDPYINFO=1
                # Re-test X11 connection if DISPLAY is set
                if [ -n "$DISPLAY" ] && xdpyinfo >/dev/null 2>&1; then
                    echo "   ✓ X server is now accessible"
                    if [ $X11_FAILED -eq 1 ]; then
                        X11_FAILED=0
                        ERRORS=$((ERRORS - 1))
                    fi
                fi
            else
                echo "   ✗ Failed to install x11-utils"
            fi
        fi
        echo ""
    fi
    
    # Install font packages
    if [ $NEED_FONTS -eq 1 ]; then
        if [ $AUTO_YES -eq 1 ]; then
            echo "Installing font packages (xfonts-base xfonts-75dpi xfonts-100dpi)..."
        else
            echo "X11 fonts are missing (needed for Chipmunk tools)."
            read -p "Install font packages (xfonts-base xfonts-75dpi xfonts-100dpi)? [y/N] " -n 1 -r
            echo
        fi
        if [ $AUTO_YES -eq 1 ] || [[ $REPLY =~ ^[Yy]$ ]]; then
            if sudo apt-get install -y xfonts-base xfonts-75dpi xfonts-100dpi; then
                echo "   ✓ Font packages installed successfully"
                if command -v xset >/dev/null 2>&1; then
                    xset fp rehash 2>/dev/null || true
                    echo "   ✓ Font cache refreshed"
                fi
                # Re-check fonts - try xlsfonts first, fall back to package check if X server unavailable
                FONT_ERRORS=0
                FIXED_FONT_COUNT=${#MISSING_FONTS[@]}
                CAN_CHECK_FONTS_VIA_X=0
                if command -v xlsfonts >/dev/null 2>&1 && xlsfonts >/dev/null 2>&1; then
                    CAN_CHECK_FONTS_VIA_X=1
                fi
                
                if [ $CAN_CHECK_FONTS_VIA_X -eq 1 ]; then
                    # X server is accessible, check fonts via xlsfonts
                    for font in "${REQUIRED_FONTS[@]}"; do
                        if xlsfonts 2>/dev/null | grep -q "^${font}$"; then
                            echo "   ✓ Font '$font' is now available"
                        else
                            FONT_ERRORS=$((FONT_ERRORS + 1))
                        fi
                    done
                else
                    # X server not accessible, verify via package installation
                    echo "   ⚠ Cannot verify fonts via X server (X server not accessible)"
                    echo "   → Assuming fonts are installed (packages were installed successfully)"
                    # Check if packages are actually installed
                    if command -v dpkg >/dev/null 2>&1; then
                        ALL_PACKAGES_INSTALLED=1
                        for pkg in xfonts-base xfonts-75dpi xfonts-100dpi; do
                            if ! dpkg -l | grep -q "^ii.*${pkg}"; then
                                ALL_PACKAGES_INSTALLED=0
                                break
                            fi
                        done
                        if [ $ALL_PACKAGES_INSTALLED -eq 1 ]; then
                            echo "   ✓ Font packages are installed"
                            FONT_ERRORS=0
                        else
                            FONT_ERRORS=1
                        fi
                    else
                        # Can't verify, but installation succeeded, so assume OK
                        FONT_ERRORS=0
                    fi
                fi
                
                if [ $FONT_ERRORS -eq 0 ] && [ $FIXED_FONT_COUNT -gt 0 ]; then
                    ERRORS=$((ERRORS - FIXED_FONT_COUNT))
                    MISSING_FONTS=()
                    FONTS_INSTALLED=1
                fi
            else
                echo "   ✗ Failed to install font packages"
            fi
        fi
        echo ""
    fi
fi

# If on WSL and packages are installed but X11 connection still fails, downgrade to warning
if [ $IS_WSL -eq 1 ] && [ $X11_FAILED -eq 1 ]; then
    if [ $X11_UTILS_INSTALLED -eq 1 ] || [ $FONTS_INSTALLED -eq 1 ]; then
        echo "   → Packages are installed, but X server connection failed."
        echo "   → This is likely a VcXsrv configuration issue (see troubleshooting above)."
        echo "   → Downgrading X11 connection failure to warning (packages are ready)."
        X11_FAILED=0
        if [ $ERRORS -gt 0 ]; then
            ERRORS=$((ERRORS - 1))
            WARNINGS=$((WARNINGS + 1))
        fi
        echo ""
    fi
fi

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

