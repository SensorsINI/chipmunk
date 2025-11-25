#!/bin/bash

# Script to delete layout images by name pattern
# Usage: ./delete_layout_images.sh [pattern]
# Examples:
#   ./delete_layout_images.sh              # Delete all *_layout.png files
#   ./delete_layout_images.sh "chip008*"   # Delete chip008*_layout.png files
#   ./delete_layout_images.sh "amon*"      # Delete amon*_layout.png files

CHIP_DIR="$HOME/chip_collection"
PATTERN="${1:-*}"  # Default to all files if no pattern provided

# Build the full patterns for both main images and thumbnails
FULL_PATTERN="${PATTERN}_layout.png"
THUMBNAIL_PATTERN="${PATTERN}_layout_thumbnail.png"

echo "Deleting layout images matching: $FULL_PATTERN"
echo "Directory: $CHIP_DIR"
echo ""

# Find matching files (both main and thumbnails)
MATCHING_FILES=$(find "$CHIP_DIR" -name "$FULL_PATTERN" -type f)
MATCHING_THUMBNAILS=$(find "$CHIP_DIR" -name "$THUMBNAIL_PATTERN" -type f)

if [ -z "$MATCHING_FILES" ]; then
    FILE_COUNT=0
else
    FILE_COUNT=$(echo "$MATCHING_FILES" | wc -l)
fi

if [ -z "$MATCHING_THUMBNAILS" ]; then
    THUMBNAIL_COUNT=0
else
    THUMBNAIL_COUNT=$(echo "$MATCHING_THUMBNAILS" | wc -l)
fi

TOTAL_COUNT=$((FILE_COUNT + THUMBNAIL_COUNT))

if [ "$TOTAL_COUNT" -eq 0 ]; then
    echo "No files found matching pattern: $FULL_PATTERN or $THUMBNAIL_PATTERN"
    exit 0
fi

echo "Found $FILE_COUNT layout image(s) and $THUMBNAIL_COUNT thumbnail(s) to delete:"
echo "$MATCHING_FILES" | head -5
if [ "$FILE_COUNT" -gt 5 ]; then
    echo "... and $((FILE_COUNT - 5)) more layout images"
fi
if [ "$THUMBNAIL_COUNT" -gt 0 ]; then
    echo "$MATCHING_THUMBNAILS" | head -3
    if [ "$THUMBNAIL_COUNT" -gt 3 ]; then
        echo "... and $((THUMBNAIL_COUNT - 3)) more thumbnails"
    fi
fi
echo ""

# Confirm deletion
read -p "Delete these $TOTAL_COUNT files ($FILE_COUNT images + $THUMBNAIL_COUNT thumbnails)? (y/N): " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    find "$CHIP_DIR" -name "$FULL_PATTERN" -type f -delete
    find "$CHIP_DIR" -name "$THUMBNAIL_PATTERN" -type f -delete
    echo "Deleted $FILE_COUNT layout image(s) and $THUMBNAIL_COUNT thumbnail(s)"
    
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
