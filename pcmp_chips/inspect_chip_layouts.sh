#!/bin/bash

# Chip Layout Inspector - open all layout images in viewer for arrow-key navigation
# Usage: ./inspect_chip_layouts.sh [image_list_file] [--thumbnails]

IMAGE_LIST="${1:-/home/tobi/pcmp_chips/layout_images.txt}"
USE_THUMBNAILS=false

# Check for --thumbnails flag
if [ "$2" = "--thumbnails" ] || [ "$1" = "--thumbnails" ]; then
    USE_THUMBNAILS=true
    if [ "$1" = "--thumbnails" ]; then
        IMAGE_LIST="/home/tobi/pcmp_chips/layout_images.txt"
    fi
fi

# Check if image list exists
if [ ! -f "$IMAGE_LIST" ]; then
    echo "Error: Image list file not found: $IMAGE_LIST"
    echo "Usage: $0 [image_list_file] [--thumbnails]"
    exit 1
fi

# Check if image viewer is available (prefer viewers with good multi-image support)
if command -v feh >/dev/null 2>&1; then
    VIEWER="feh"
    VIEWER_ARGS="-."  # Slideshow mode with borderless window
elif command -v eog >/dev/null 2>&1; then
    VIEWER="eog"
    VIEWER_ARGS=""
elif command -v gwenview >/dev/null 2>&1; then
    VIEWER="gwenview"
    VIEWER_ARGS=""
elif command -v sxiv >/dev/null 2>&1; then
    VIEWER="sxiv"
    VIEWER_ARGS=""
else
    echo "Error: No suitable image viewer found"
    echo "Recommended: feh (best for keyboard navigation)"
    echo "Alternatives: eog, gwenview, sxiv"
    echo ""
    echo "Install feh with: sudo apt install feh"
    exit 1
fi

echo "Chip Layout Inspector"
echo "====================="
echo "Using viewer: $VIEWER"
echo "Image list: $IMAGE_LIST"

# Build list of image files
IMAGE_FILES=()
while IFS= read -r image_file; do
    # If using thumbnails, replace _layout.png with _layout_thumbnail.png
    if [ "$USE_THUMBNAILS" = true ]; then
        thumbnail_file="${image_file/_layout.png/_layout_thumbnail.png}"
        if [ -f "$thumbnail_file" ]; then
            IMAGE_FILES+=("$thumbnail_file")
        elif [ -f "$image_file" ]; then
            IMAGE_FILES+=("$image_file")
        fi
    else
        if [ -f "$image_file" ]; then
            IMAGE_FILES+=("$image_file")
        fi
    fi
done < "$IMAGE_LIST"

TOTAL=${#IMAGE_FILES[@]}

if [ "$TOTAL" -eq 0 ]; then
    echo "Error: No valid image files found"
    exit 1
fi

echo "Found $TOTAL images"
if [ "$USE_THUMBNAILS" = true ]; then
    echo "Mode: Thumbnails (fast preview)"
else
    echo "Mode: Full-size images"
fi
echo ""
echo "Controls (in $VIEWER):"
case $VIEWER in
    feh)
        echo "  - Right Arrow / Space: Next image"
        echo "  - Left Arrow / Backspace: Previous image"
        echo "  - Q / Escape: Quit"
        echo "  - Z: Zoom to fit"
        echo "  - +/-: Zoom in/out"
        ;;
    eog)
        echo "  - Right Arrow / Space: Next image"
        echo "  - Left Arrow / Backspace: Previous image"
        echo "  - Q / Escape: Quit"
        ;;
    *)
        echo "  - Arrow keys: Navigate"
        echo "  - Q / Escape: Quit"
        ;;
esac
echo ""

# Open all images in the viewer
$VIEWER $VIEWER_ARGS "${IMAGE_FILES[@]}" 2>/dev/null

echo ""
echo "Finished reviewing chip layouts!"
