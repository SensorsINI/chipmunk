#!/bin/bash

# Script to delete layout images by name pattern
# Usage: ./delete_layout_images.sh [pattern]
# Examples:
#   ./delete_layout_images.sh              # Delete all *_layout.png files
#   ./delete_layout_images.sh "chip008*"   # Delete chip008*_layout.png files
#   ./delete_layout_images.sh "amon*"      # Delete amon*_layout.png files

CHIP_DIR="$HOME/chip_collection"
PATTERN="${1:-*}"  # Default to all files if no pattern provided

# Build the full pattern
FULL_PATTERN="${PATTERN}_layout.png"

echo "Deleting layout images matching: $FULL_PATTERN"
echo "Directory: $CHIP_DIR"
echo ""

# Find matching files
MATCHING_FILES=$(find "$CHIP_DIR" -name "$FULL_PATTERN" -type f)
FILE_COUNT=$(echo "$MATCHING_FILES" | grep -c "." 2>/dev/null || echo "0")

if [ "$FILE_COUNT" -eq 0 ]; then
    echo "No files found matching pattern: $FULL_PATTERN"
    exit 0
fi

echo "Found $FILE_COUNT files to delete:"
echo "$MATCHING_FILES" | head -10
if [ "$FILE_COUNT" -gt 10 ]; then
    echo "... and $((FILE_COUNT - 10)) more files"
fi
echo ""

# Confirm deletion
read -p "Delete these $FILE_COUNT files? (y/N): " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    find "$CHIP_DIR" -name "$FULL_PATTERN" -type f -delete
    echo "Deleted $FILE_COUNT layout image(s)"
    
    # Also clean up any related log files if deleting all
    if [ "$PATTERN" = "*" ]; then
        echo ""
        read -p "Also delete conversion log files? (y/N): " -n 1 -r
        echo ""
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            rm -f "$CHIP_DIR/layout_images.txt"
            rm -f "$CHIP_DIR/converted_mosis_files.txt"
            rm -f "$CHIP_DIR/no_pname_mosis_files.txt"
            rm -f "$CHIP_DIR/layout_conversion_errors.log"
            echo "Deleted log files"
        fi
    fi
else
    echo "Deletion cancelled"
    exit 1
fi
