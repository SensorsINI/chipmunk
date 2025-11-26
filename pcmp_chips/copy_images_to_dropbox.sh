#!/bin/bash

SOURCE_DIR="$HOME/pcmp_chips"
DEST_DIR="$HOME/Dropbox/pcmp_chips_images"
IMAGE_LIST="$SOURCE_DIR/layout_images.txt"

# Create destination directory
mkdir -p "$DEST_DIR"

echo "Copying chip layout images to Dropbox..."
echo "Source: $SOURCE_DIR"
echo "Destination: $DEST_DIR"
echo ""

# Wait for image list to be populated if it's empty
while [ ! -s "$IMAGE_LIST" ]; do
    echo "Waiting for images to be generated..."
    sleep 10
done

# Copy each image from the list
COPIED=0
FAILED=0

while IFS= read -r image_file; do
    if [ -f "$image_file" ]; then
        # Get the basename for the destination
        basename_img=$(basename "$image_file")
        
        # Copy with progress
        cp "$image_file" "$DEST_DIR/$basename_img"
        
        if [ $? -eq 0 ]; then
            COPIED=$((COPIED + 1))
            echo "[$COPIED] Copied: $basename_img"
        else
            FAILED=$((FAILED + 1))
            echo "ERROR copying: $image_file"
        fi
    else
        FAILED=$((FAILED + 1))
        echo "ERROR: File not found: $image_file"
    fi
done < "$IMAGE_LIST"

echo ""
echo "Copy complete!"
echo "Successfully copied: $COPIED images"
echo "Failed: $FAILED images"
echo "Destination: $DEST_DIR"
