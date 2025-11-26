#!/bin/bash

# Extract pcmp-home-4TB.tgz with progress indicator
# Usage: ./extract_archive.sh

ARCHIVE="$HOME/pcmp-home-4TB.tgz"
EXTRACT_DIR="$HOME"

if [ ! -f "$ARCHIVE" ]; then
    echo "Error: Archive not found at $ARCHIVE"
    exit 1
fi

echo "Extracting $ARCHIVE to $EXTRACT_DIR"
echo "This may take a while..."
echo ""

# Method 1: Using pv (pipe viewer) - best option if available
if command -v pv >/dev/null 2>&1; then
    # Get archive size for pv
    ARCHIVE_SIZE=$(stat -f%z "$ARCHIVE" 2>/dev/null || stat -c%s "$ARCHIVE" 2>/dev/null)
    if [ -n "$ARCHIVE_SIZE" ]; then
        echo "Using pv for progress display..."
        pv -p -t -e -r -b -s "$ARCHIVE_SIZE" "$ARCHIVE" | tar -xzmf - -C "$EXTRACT_DIR"
    else
        echo "Using pv for progress display (size unknown)..."
        pv "$ARCHIVE" | tar -xzmf - -C "$EXTRACT_DIR"
    fi
else
    # Method 2: Using tar checkpoint (fallback)
    # -m preserves modification times (default, but explicit)
    echo "Using tar checkpoint for progress (every 1000 files)..."
    tar --checkpoint=1000 --checkpoint-action=echo="Extracted %T files" -xzmf "$ARCHIVE" -C "$EXTRACT_DIR"
fi

echo ""
echo "Extraction complete!"
