#!/bin/bash

# Script to create MP4 movie from chip layout thumbnails
# Usage: ./create_layout_movie.sh [output_file] [frame_rate] [compression]
# Examples:
#   ./create_layout_movie.sh chip_layouts.mp4
#   ./create_layout_movie.sh chip_layouts.mp4 2 23
#   ./create_layout_movie.sh chip_layouts.mp4 1 18  # Slower, higher quality

IMAGE_LIST="${IMAGE_LIST:-$HOME/chip_collection/layout_images.txt}"
OUTPUT_FILE="${1:-${OUTPUT_FILE:-$HOME/chip_collection/chip_layouts.mp4}}"
FRAME_RATE="${2:-${FRAME_RATE:-10}}"  # Default 2 Hz (2 frames per second)
CRF="${3:-${CRF:-18}}"                # Default compression (18-28, lower = higher quality)

# Check if image list exists
if [ ! -f "$IMAGE_LIST" ]; then
    echo "Error: Image list file not found: $IMAGE_LIST"
    echo "Usage: $0 [output_file] [frame_rate] [compression_crf]"
    echo ""
    echo "Parameters:"
    echo "  output_file    - Output MP4 file (default: ~/chip_collection/chip_layouts.mp4)"
    echo "  frame_rate     - Frames per second (default: 2)"
    echo "  compression_crf- Quality: 18-28, lower=higher quality (default: 23)"
    echo ""
    echo "Environment variables:"
    echo "  IMAGE_LIST     - Path to layout_images.txt (default: ~/chip_collection/layout_images.txt)"
    exit 1
fi

# Check if ffmpeg is available
if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "Error: ffmpeg is not installed"
    echo "Install with: sudo apt install ffmpeg"
    exit 1
fi

# Create temporary directory for numbered images
TEMP_DIR=$(mktemp -d)
INTERRUPTED=false

# Cleanup function
cleanup() {
    if [ "$INTERRUPTED" = true ]; then
        echo ""
        echo "Interrupted! Waiting for ffmpeg to finish current frame..."
        # Wait a moment for ffmpeg to finish gracefully
        sleep 2
    fi
    rm -rf "$TEMP_DIR"
}

# Trap signals to allow graceful shutdown
trap 'INTERRUPTED=true; cleanup; exit 130' INT TERM
trap 'cleanup' EXIT

echo "Creating chip layout movie"
echo "=========================="
echo "Image list: $IMAGE_LIST"
echo "Output file: $OUTPUT_FILE"
echo "Frame rate: $FRAME_RATE fps"
echo "Compression (CRF): $CRF (lower = higher quality)"
echo ""

# Count total images
TOTAL=$(wc -l < "$IMAGE_LIST")
echo "Found $TOTAL layout images"
echo ""

# Process each image
COUNT=0
MISSING=0
while IFS= read -r full_image; do
    # Convert full image path to thumbnail path
    thumbnail="${full_image/_layout.png/_layout_thumbnail.png}"
    
    # Check if thumbnail exists
    if [ ! -f "$thumbnail" ]; then
        echo "Warning: Thumbnail not found: $thumbnail" >&2
        MISSING=$((MISSING + 1))
        continue
    fi
    
    # Create numbered symlink for ffmpeg (needs sequential numbering)
    COUNT=$((COUNT + 1))
    # Pad with zeros for proper sorting: 0001.png, 0002.png, etc.
    numbered_name=$(printf "%06d.png" $COUNT)
    ln -s "$(realpath "$thumbnail")" "$TEMP_DIR/$numbered_name"
    
    if [ $((COUNT % 50)) -eq 0 ]; then
        echo "Processed $COUNT/$TOTAL images..."
    fi
done < "$IMAGE_LIST"

if [ $COUNT -eq 0 ]; then
    echo "Error: No valid thumbnails found"
    exit 1
fi

if [ $MISSING -gt 0 ]; then
    echo "Warning: $MISSING thumbnails were missing" >&2
fi

echo "Processed $COUNT thumbnails"
echo ""

# Create MP4 using ffmpeg with quality preservation settings
echo "Encoding MP4 video..."
echo ""

# Run ffmpeg and capture output
# Note: -r sets the output frame rate (playback speed)
#       -framerate sets input reading rate (should match for proper timing)
FFMPEG_OUTPUT=$(ffmpeg -y \
    -framerate "$FRAME_RATE" \
    -pattern_type glob \
    -i "$TEMP_DIR/*.png" \
    -r "$FRAME_RATE" \
    -c:v libx264 \
    -crf "$CRF" \
    -preset slow \
    -pix_fmt yuv420p \
    -vf "scale=iw:ih:flags=lanczos+accurate_rnd+full_chroma_int" \
    -colorspace bt709 \
    -color_primaries bt709 \
    -color_trc bt709 \
    -color_range pc \
    -sws_flags lanczos+accurate_rnd+full_chroma_int \
    -movflags +faststart \
    "$OUTPUT_FILE" 2>&1)

FFMPEG_EXIT=$?

# Show progress
echo "$FFMPEG_OUTPUT" | grep -E "(frame=|Duration=|bitrate=)" || true

# Check if file was created and is valid
if [ -f "$OUTPUT_FILE" ] && [ -s "$OUTPUT_FILE" ]; then
    # Verify it's a valid MP4 by checking file type
    if file "$OUTPUT_FILE" | grep -q "MP4\|ISO Media"; then
        echo ""
        if [ "$INTERRUPTED" = true ]; then
            echo "Interrupted but valid MP4 created: $OUTPUT_FILE"
        else
            echo "Successfully created: $OUTPUT_FILE"
        fi
        echo "File size: $(du -h "$OUTPUT_FILE" | cut -f1)"
        
        # Try to get actual duration from the file (if ffprobe is available)
        if command -v ffprobe >/dev/null 2>&1; then
            DURATION=$(ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 "$OUTPUT_FILE" 2>/dev/null | cut -d. -f1)
            if [ -n "$DURATION" ] && [ "$DURATION" != "N/A" ]; then
                echo "Duration: ${DURATION} seconds"
            else
                echo "Duration: ~$((COUNT / FRAME_RATE)) seconds (estimated)"
            fi
        else
            echo "Duration: ~$((COUNT / FRAME_RATE)) seconds (estimated)"
        fi
    else
        echo ""
        echo "Error: Output file exists but is not a valid MP4"
        rm -f "$OUTPUT_FILE"
        exit 1
    fi
elif [ "$FFMPEG_EXIT" -ne 0 ] && [ "$INTERRUPTED" = false ]; then
    echo ""
    echo "Error: Failed to create video file"
    echo "ffmpeg exit code: $FFMPEG_EXIT"
    exit 1
elif [ "$INTERRUPTED" = true ]; then
    echo ""
    echo "Interrupted - no valid output file created"
    rm -f "$OUTPUT_FILE"
    exit 130
fi
