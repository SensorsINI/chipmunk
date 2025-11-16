#!/usr/bin/env bash
# Post-build PATH/alias helper for Chipmunk (bash only)
# - Prompts interactively to add chipmunk/bin to PATH if missing
# - No-op if already present or running non-interactively

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN_DIR="$PROJECT_ROOT/bin"
RC_FILE="${HOME}/.bashrc"
MARKER_BEGIN="# >>> chipmunk PATH (added by build) >>>"
MARKER_END="# <<< chipmunk PATH (added by build) <<<"

# Ensure bin exists
if [[ ! -d "$BIN_DIR" ]]; then
  exit 0
fi

# Already available?
case ":$PATH:" in
  *":$BIN_DIR:"*) HAS_PATH=1 ;;
  *) HAS_PATH=0 ;;
esac

# If non-interactive, just print guidance once
if [[ $HAS_PATH -eq 0 && ! -t 1 ]]; then
  echo ""
  echo "Tip: Add Chipmunk bin to your PATH to run 'analog' from any directory:"
  echo "  echo \"${MARKER_BEGIN}\" >> \"$RC_FILE\""
  echo "  echo \"export PATH=\\\"$BIN_DIR:\\\$PATH\\\"\" >> \"$RC_FILE\""
  echo "  echo \"${MARKER_END}\" >> \"$RC_FILE\""
  echo "  . \"$RC_FILE\""
  exit 0
fi

# If already present, nothing to do
if [[ $HAS_PATH -eq 1 ]]; then
  exit 0
fi

# Interactive prompt for bash users
echo ""
echo "Chipmunk build: add '$BIN_DIR' to your PATH in '$RC_FILE' so you can run 'analog' anywhere?"
read -r -p "Add to PATH now? [Y/n]: " reply
reply="${reply:-Y}"
if [[ "$reply" =~ ^[Yy]$ ]]; then
  # Remove previous block if present
  if grep -qF "$MARKER_BEGIN" "$RC_FILE" 2>/dev/null; then
    awk -v b="$MARKER_BEGIN" -v e="$MARKER_END" '
      $0==b{skip=1; next} $0==e{skip=0; next} !skip{print}
    ' "$RC_FILE" > "${RC_FILE}.chipmunk.tmp" && mv "${RC_FILE}.chipmunk.tmp" "$RC_FILE"
  fi
  {
    echo "$MARKER_BEGIN"
    echo "export PATH=\"$BIN_DIR:\$PATH\""
    echo "$MARKER_END"
  } >> "$RC_FILE"
  echo "Added. Open a NEW terminal to apply, or run:"
  echo "  . \"$RC_FILE\""
else
  echo "Skipped. You can add it later with:"
  echo "  echo \"${MARKER_BEGIN}\" >> \"$RC_FILE\""
  echo "  echo \"export PATH=\\\"$BIN_DIR:\\\$PATH\\\"\" >> \"$RC_FILE\""
  echo "  echo \"${MARKER_END}\" >> \"$RC_FILE\""
  echo "  . \"$RC_FILE\""
fi

exit 0


