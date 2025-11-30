#!/bin/bash

# Script to create MP4 movie from chip layout thumbnails synchronized to dub music beats
# Chips change exactly at each beat, synchronized with audio
# Audio fades out over 1 second at the end. By default includes photos from photos folder,
# If no audio file is provided, the script will use the default audio file in the chip directory.
# --short option creates a movie with one chip image per frame, no audio, with 0.5s pause on first and last frame.

# Parse --short option
SHORT_MODE=false
ARGS=()
for arg in "$@"; do
    if [ "$arg" = "--short" ]; then
        SHORT_MODE=true
    else
        ARGS+=("$arg")
    fi
done

# Show help if requested
if [ "${ARGS[0]}" = "--help" ] || [ "${ARGS[0]}" = "-h" ]; then
    cat << 'EOF'
Usage: ./create_dub_chip_movie.sh [--short] [output_file] [audio_segment_start] [duration]
       ./create_dub_chip_movie.sh --help

Options:
  --short              Create short movie: one chip image per frame, no audio,
                       with 0.5s pause on first and last frame

Arguments:
  output_file          Output MP4 filename (default: $HOME/pcmp_home/pcmp_chips_dub.mp4)
                       If no extension is provided, .mp4 will be appended automatically
  audio_segment_start  Start time in audio file in seconds (default: 0, ignored in --short mode)
  duration            Video duration in seconds (default: 0 = use all available chips or full audio)

Examples:
  ./create_dub_chip_movie.sh dub_chips.mp4
  ./create_dub_chip_movie.sh --short short_chips.mp4
  ./create_dub_chip_movie.sh dub_chips 0 45
  ./create_dub_chip_movie.sh output 0 30

Environment Variables:
  CHIP_DIR            Directory containing chip data (default: $HOME/pcmp_home)
  CSV_DATABASE       Path to chip database CSV file (default: $CHIP_DIR/chip_database.csv)
  IMAGE_LIST          Path to layout images list file (default: $CHIP_DIR/layout_images.txt)
  AUDIO_FILE          Path to audio file for synchronization (default: $CHIP_DIR/04 Reaching Dub.m4a)
                       (ignored in --short mode)
  OUTPUT_FILE         Output MP4 filename (overrides first argument if set)
  VIDEO_FPS           Video frame rate in fps (default: 60)
  CRF                 Compression quality 0-51, lower is better quality (default: 23)
  FILE_LIMIT          Limit number of files to process, 0 = unlimited (default: 0)
  BEAT_THRESHOLD      Beat detection sensitivity 0.0-1.0, lower = more sensitive (default: 0.6)
                       (ignored in --short mode)

Examples with environment variables:
  FILE_LIMIT=100 ./create_dub_chip_movie.sh test.mp4  # Test with 100 images
  BEAT_THRESHOLD=0.4 ./create_dub_chip_movie.sh output.mp4 0 30  # More sensitive beat detection
  CRF=18 ./create_dub_chip_movie.sh high_quality.mp4  # Higher quality output
  ./create_dub_chip_movie.sh --short short.mp4  # Short mode: one chip per frame, no audio
EOF
    exit 0
fi

CHIP_DIR="${CHIP_DIR:-$HOME/pcmp_home}"
CSV_DATABASE="${CSV_DATABASE:-$CHIP_DIR/chip_database.csv}"
IMAGE_LIST="${IMAGE_LIST:-$CHIP_DIR/layout_images.txt}"
AUDIO_FILE="${AUDIO_FILE:-$CHIP_DIR/04 Reaching Dub.m4a}"
OUTPUT_FILE="${ARGS[0]:-${OUTPUT_FILE:-$HOME/pcmp_home/pcmp_chips_dub.mp4}}"
AUDIO_START="${ARGS[1]:-0}"  # Start time in audio (seconds)
VIDEO_DURATION="${ARGS[2]:-0}"  # Target video duration (seconds)

# Ensure OUTPUT_FILE has .mp4 extension if no extension is provided
if [[ ! "$OUTPUT_FILE" =~ \.[^/]+$ ]]; then
    OUTPUT_FILE="${OUTPUT_FILE}.mp4"
fi
VIDEO_FPS="${VIDEO_FPS:-60}"  # Video frame rate (fps) - MP4 container frame rate
CRF="${CRF:-23}" # Compression quality (0-51, 0 is best quality)
FILE_LIMIT="${FILE_LIMIT:-0}"  # 0 = no limit, otherwise stop after N files
BEAT_THRESHOLD="${BEAT_THRESHOLD:-0.8}"  # Beat detection threshold for aubioonset (0.0-1.0, lower = more sensitive)
BEAT_METHOD="${BEAT_METHOD:-complex}"  # Beat detection method for aubioonset (phase, energy, hfc, complex, specdiff, kl, mkl, specflux)
# Note: No minimum dwell time - chips are shown for the actual beat interval duration

# Check dependencies
if ! command -v convert >/dev/null 2>&1; then
    echo "Error: ImageMagick 'convert' is not installed"
    echo "Install with: sudo apt install imagemagick"
    exit 1
fi

if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "Error: ffmpeg is not installed"
    echo "Install with: sudo apt install ffmpeg"
    exit 1
fi

if ! command -v aubioonset >/dev/null 2>&1; then
    echo "Error: aubio-tools is not installed"
    echo "Install with: sudo apt install aubio-tools"
    exit 1
fi

# Check if required files exist
if [ ! -f "$CSV_DATABASE" ]; then
    echo "Error: CSV database not found: $CSV_DATABASE"
    echo "Run generate_chip_database.sh first to create the database"
    exit 1
fi

if [ ! -f "$IMAGE_LIST" ]; then
    echo "Error: Image list file not found: $IMAGE_LIST"
    echo "Run convert_layouts_from_csv.sh first to create the image list"
    exit 1
fi

if [ "$SHORT_MODE" = false ] && [ ! -f "$AUDIO_FILE" ]; then
    echo "Error: Audio file not found: $AUDIO_FILE"
    exit 1
fi

# Create temporary directories
TEMP_DIR=$(mktemp -d)
CACHE_DIR="/tmp/pcmp_chip_movie"
ANNOTATED_DIR="$CACHE_DIR/annotated"
SEGMENTS_DIR="$TEMP_DIR/segments"
mkdir -p "$ANNOTATED_DIR" "$SEGMENTS_DIR"

INTERRUPTED=false

# Cleanup function
cleanup() {
    if [ "$INTERRUPTED" = true ]; then
        echo ""
        echo "Interrupted! Cleaning up..."
        sleep 2
    fi
    # Only remove temp dir, keep cache
    rm -rf "$TEMP_DIR"
    # Note: Annotated images in $ANNOTATED_DIR are kept for caching
}

# Trap signals
trap 'INTERRUPTED=true; cleanup; exit 130' INT TERM
trap 'cleanup' EXIT

# No minimum dwell time - use actual beat interval durations

# Handle VIDEO_DURATION=0 as special case: use all frames or entire song, whichever is shorter
ORIGINAL_VIDEO_DURATION="$VIDEO_DURATION"
if [ "$VIDEO_DURATION" = "0" ]; then
    echo "VIDEO_DURATION=0: Will use all available chips or full audio, whichever is shorter"
    
    # Get audio file duration
    if command -v ffprobe >/dev/null 2>&1; then
        AUDIO_FULL_DURATION=$(ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 "$AUDIO_FILE" 2>/dev/null)
        if [ -n "$AUDIO_FULL_DURATION" ] && [ "$AUDIO_FULL_DURATION" != "N/A" ]; then
            # Calculate available duration from AUDIO_START to end
            AVAILABLE_AUDIO_DURATION=$(echo "$AUDIO_FULL_DURATION - $AUDIO_START" | bc -l 2>/dev/null)
            if [ "$(echo "$AVAILABLE_AUDIO_DURATION <= 0" | bc -l 2>/dev/null || echo "1")" = "1" ]; then
                echo "Error: AUDIO_START ($AUDIO_START) is beyond audio file duration ($AUDIO_FULL_DURATION)"
                exit 1
            fi
            echo "  Audio file duration: ${AUDIO_FULL_DURATION}s"
            echo "  Available from AUDIO_START: ${AVAILABLE_AUDIO_DURATION}s"
            # Use the full available audio duration
            VIDEO_DURATION="$AVAILABLE_AUDIO_DURATION"
            echo "  Using full available audio: ${VIDEO_DURATION}s"
        else
            echo "Warning: Could not determine audio duration, defaulting to 45s"
            VIDEO_DURATION=45
        fi
    else
        echo "Warning: ffprobe not available, cannot determine audio duration, defaulting to 45s"
        VIDEO_DURATION=45
    fi
elif [ -z "$VIDEO_DURATION" ] || [ "$(echo "$VIDEO_DURATION <= 0" | bc -l 2>/dev/null || echo "1")" = "1" ]; then
    echo "Error: VIDEO_DURATION must be a positive number or 0 (got: ${VIDEO_DURATION})"
    echo "Usage: $0 [output_file] [audio_start] [duration]"
    echo "  duration: positive number (seconds) or 0 (use all frames/entire song, whichever is shorter)"
    echo "Example: $0 output.mp4 0 45"
    echo "Example: $0 output.mp4 0 0  # Use all available chips or full audio"
    exit 1
fi

if [ "$SHORT_MODE" = true ]; then
    echo "Creating short chip layout movie (--short mode)"
    echo "==========================================="
    echo "CSV Database: $CSV_DATABASE"
    echo "Image list: $IMAGE_LIST"
    echo "Mode: One chip image per frame, no audio"
    echo "Pause: 0.5s on first and last frame"
    echo "Video frame rate: ${VIDEO_FPS} fps"
    echo "Output file: $OUTPUT_FILE"
    echo "Compression (CRF): $CRF"
    if [ "$FILE_LIMIT" -gt 0 ]; then
        echo "File limit: $FILE_LIMIT files (test mode)"
    else
        echo "File limit: unlimited"
    fi
    echo ""
else
    echo "Creating dub-synchronized chip layout movie"
    echo "==========================================="
    echo "CSV Database: $CSV_DATABASE"
    echo "Image list: $IMAGE_LIST"
    echo "Audio file: $AUDIO_FILE"
    echo "Audio start: ${AUDIO_START}s"
    echo "Video duration: ${VIDEO_DURATION}s"
    echo "Video frame rate: ${VIDEO_FPS} fps (MP4 container rate)"
    echo "Beat detection threshold: ${BEAT_THRESHOLD} (lower = more sensitive, more beats)"
    echo "Beat detection method: ${BEAT_METHOD}"
    echo "Timing: One chip per beat interval (no minimum dwell time)"
    echo "Output file: $OUTPUT_FILE"
    echo "Compression (CRF): $CRF"
    if [ "$FILE_LIMIT" -gt 0 ]; then
        echo "File limit: $FILE_LIMIT files (test mode)"
    else
        echo "File limit: unlimited"
    fi
    echo ""
fi

# Function to extract year from normalized_date (YYYY-MM-DD format)
extract_year() {
    local date_str="$1"
    echo "$date_str" | cut -d'-' -f1
}

# Function to extract chip name (part before first underscore) from p_name
extract_chip_name() {
    local pname="$1"
    echo "$pname" | cut -d'_' -f1
}

# Function to overlay text on image (same as original script)
overlay_title_block() {
    local input_image="$1"
    local output_image="$2"
    local year="$3"
    local chip_name="$4"
    local username="$5"
    
    local width=$(identify -format "%w" "$input_image")
    local height=$(identify -format "%h" "$input_image")
    
    local font_size_year=$((height / 10))
    local font_size_chip=$((height / 20))
    local font_size_user=$((height / 17))
    
    local margin=$((height / 30))
    local shadow_offset=$((height / 200))
    if [ "$shadow_offset" -lt 2 ]; then
        shadow_offset=2
    fi
    
    local temp_dir=$(dirname "$output_image")
    local year_img="$temp_dir/.year_$$.png"
    local year_shadow="$temp_dir/.year_shadow_$$.png"
    local chip_img="$temp_dir/.chip_$$.png"
    local chip_shadow="$temp_dir/.chip_shadow_$$.png"
    local user_img="$temp_dir/.user_$$.png"
    local user_shadow="$temp_dir/.user_shadow_$$.png"
    
    if [ -z "$year" ] || [ "$year" = "Unknown" ] || [ "$year" = "?" ]; then
        year="?"
    fi
    
    convert -background transparent \
        -fill "rgba(0,0,0,0.7)" \
        -font "DejaVu-Sans-Bold" \
        -pointsize "$font_size_year" \
        label:"$year" \
        "$year_shadow"
    
    convert -background transparent \
        -fill "rgba(255,255,255,1.0)" \
        -font "DejaVu-Sans-Bold" \
        -pointsize "$font_size_year" \
        label:"$year" \
        "$year_img"
    
    if [ -z "$chip_name" ]; then
        chip_name="Unknown"
    fi
    
    convert -background transparent \
        -fill "rgba(0,0,0,0.7)" \
        -font "DejaVu-Sans-Bold" \
        -pointsize "$font_size_chip" \
        label:"$chip_name" \
        "$chip_shadow"
    
    convert -background transparent \
        -fill "rgba(255,255,255,1.0)" \
        -font "DejaVu-Sans-Bold" \
        -pointsize "$font_size_chip" \
        label:"$chip_name" \
        "$chip_img"
    
    if [ -z "$username" ]; then
        username="Unknown"
    fi
    
    convert -background transparent \
        -fill "rgba(0,0,0,0.7)" \
        -font "DejaVu-Sans" \
        -pointsize "$font_size_user" \
        label:"$username" \
        "$user_shadow"
    
    convert -background transparent \
        -fill "rgba(255,255,255,1.0)" \
        -font "DejaVu-Sans" \
        -pointsize "$font_size_user" \
        label:"$username" \
        "$user_img"
    
    local year_w=$(identify -format "%w" "$year_img")
    local year_h=$(identify -format "%h" "$year_img")
    local chip_w=$(identify -format "%w" "$chip_img")
    local chip_h=$(identify -format "%h" "$chip_img")
    local user_w=$(identify -format "%w" "$user_img")
    local user_h=$(identify -format "%h" "$user_img")
    
    local year_x=$margin
    local year_y=$margin
    local user_x=$margin
    local user_y=$((height - user_h - margin))
    local chip_x=$((width - chip_w - margin))
    local chip_y=$((height - chip_h - margin))
    
    rm -f "$output_image"
    
    convert "$input_image" \
        \( "$year_shadow" -geometry +$((year_x + shadow_offset))+$((year_y + shadow_offset)) \) -composite \
        \( "$year_img" -geometry +${year_x}+${year_y} \) -composite \
        \( "$user_shadow" -geometry +$((user_x + shadow_offset))+$((user_y + shadow_offset)) \) -composite \
        \( "$user_img" -geometry +${user_x}+${user_y} \) -composite \
        \( "$chip_shadow" -geometry +$((chip_x + shadow_offset))+$((chip_y + shadow_offset)) \) -composite \
        \( "$chip_img" -geometry +${chip_x}+${chip_y} \) -composite \
        "$output_image"
    
    rm -f "$year_img" "$year_shadow" "$chip_img" "$chip_shadow" "$user_img" "$user_shadow"
}

# Calculate timing statistics for user confirmation
echo "Calculating timing statistics..."
ESTIMATED_IMAGES=$(wc -l < "$IMAGE_LIST" 2>/dev/null || echo "0")

if [ "$SHORT_MODE" = true ]; then
    echo ""
    echo "==========================================="
    echo "SHORT MODE: ONE CHIP PER FRAME"
    echo "==========================================="
    echo "Available images: $ESTIMATED_IMAGES"
    echo "Timing: One chip image per frame"
    echo "Pause: 0.5s on first and last frame"
    echo "Audio: None"
    echo ""
    echo "==========================================="
else
    echo ""
    echo "==========================================="
    echo "MOVIE TIMING SUMMARY"
    echo "==========================================="
    echo "Video duration: ${VIDEO_DURATION}s"
    echo "Audio start: ${AUDIO_START}s"
    echo "Available images: $ESTIMATED_IMAGES"
    echo "Timing: Chips change exactly at each beat"
    echo ""
    echo "Strategy:"
    echo "  - Extract audio segment first"
    echo "  - Detect beats directly from extracted audio segment"
    echo "  - Create segments starting at each beat (aligned with audio)"
    echo "  - Chips will be randomly subsampled to match number of beats"
    echo ""
    echo "==========================================="
fi
if false; then
    # Check if USE_ALL_FRAMES environment variable is set
    if [ "${USE_ALL_FRAMES:-false}" = "true" ]; then
        # Non-interactive mode: automatically use all frames
        SUBSAMPLE_IMAGES=false
        # Update VIDEO_DURATION to estimated duration (add 10% buffer for safety)
        NEW_DURATION=$(echo "scale=0; ($ESTIMATED_VIDEO_DURATION * 1.1) / 1" | bc -l 2>/dev/null)
        if [ -n "$NEW_DURATION" ] && [ "$NEW_DURATION" != "0" ] && [ "$(echo "$NEW_DURATION > 0" | bc -l 2>/dev/null || echo "0")" = "1" ]; then
            VIDEO_DURATION="$NEW_DURATION"
        else
            # Fallback: use minimum duration + 20% buffer
            VIDEO_DURATION=$(echo "scale=0; ($MIN_VIDEO_DURATION * 1.2) / 1" | bc -l 2>/dev/null || echo "30")
        fi
        # Ensure VIDEO_DURATION is a valid positive number
        if [ -z "$VIDEO_DURATION" ] || [ "$VIDEO_DURATION" = "0" ] || [ "$(echo "$VIDEO_DURATION <= 0" | bc -l 2>/dev/null || echo "1")" = "1" ]; then
            VIDEO_DURATION="30"
        fi
        echo "USE_ALL_FRAMES=true: Automatically using all ${ESTIMATED_IMAGES} images"
        echo "Video duration adjusted to ~${VIDEO_DURATION}s (estimated ${ESTIMATED_VIDEO_DURATION}s + 10% buffer)"
    else
        # Interactive mode: prompt user
        echo "Choose an option:"
        echo "  1) Randomly subsample images to fit ${VIDEO_DURATION}s duration"
        echo "  2) Use all images (estimated ~${ESTIMATED_VIDEO_DURATION}s duration, minimum ${MIN_VIDEO_DURATION}s)"
        read -p "Enter choice (1 or 2): " -n 1 -r
        echo ""
        if [[ $REPLY == "1" ]]; then
            SUBSAMPLE_IMAGES=true
            TARGET_IMAGE_COUNT=$MAX_IMAGES_IN_DURATION
            echo "Will randomly subsample to ~${TARGET_IMAGE_COUNT} images"
        elif [[ $REPLY == "2" ]]; then
            SUBSAMPLE_IMAGES=false
            # Update VIDEO_DURATION to estimated duration (add 10% buffer for safety)
            NEW_DURATION=$(echo "scale=0; ($ESTIMATED_VIDEO_DURATION * 1.1) / 1" | bc -l 2>/dev/null)
            if [ -n "$NEW_DURATION" ] && [ "$NEW_DURATION" != "0" ] && [ "$(echo "$NEW_DURATION > 0" | bc -l 2>/dev/null || echo "0")" = "1" ]; then
                VIDEO_DURATION="$NEW_DURATION"
            else
                # Fallback: use minimum duration + 20% buffer
                VIDEO_DURATION=$(echo "scale=0; ($MIN_VIDEO_DURATION * 1.2) / 1" | bc -l 2>/dev/null || echo "30")
            fi
            # Ensure VIDEO_DURATION is a valid positive number
            if [ -z "$VIDEO_DURATION" ] || [ "$VIDEO_DURATION" = "0" ] || [ "$(echo "$VIDEO_DURATION <= 0" | bc -l 2>/dev/null || echo "1")" = "1" ]; then
                VIDEO_DURATION="30"
            fi
            echo "Will use all ${ESTIMATED_IMAGES} images"
            echo "Video duration adjusted to ~${VIDEO_DURATION}s (estimated ${ESTIMATED_VIDEO_DURATION}s + 10% buffer)"
        else
            echo "Invalid choice, aborting"
            exit 0
        fi
    fi
fi
# No prompt needed - automatically proceed with beat-synchronized timing
echo ""

# Handle short mode separately
if [ "$SHORT_MODE" = true ]; then
    # SHORT MODE: One chip per frame, no audio, 0.5s pause on first and last frame
    
    # Step 1: Load all chips
    echo "Step 1: Loading all chips..."
    CSV_SORTED="${CSV_SORTED:-$CHIP_DIR/chip_database_sorted.csv}"
    
    if [ ! -f "$CSV_SORTED" ]; then
        echo "Warning: Sorted CSV not found at $CSV_SORTED"
        echo "Creating sorted CSV from $CSV_DATABASE..."
        "$(dirname "$0")/sort_chip_database_by_date.sh"
        CSV_SORTED="$CHIP_DIR/chip_database_sorted.csv"
    fi
    
    if [ ! -f "$CSV_SORTED" ]; then
        echo "Error: Could not create or find sorted CSV file"
        exit 1
    fi
    
    # Extract chips from sorted CSV and match with image list
    SORTED_LIST="$TEMP_DIR/selected_chips.txt"
    > "$SORTED_LIST"
    
    PYTHON_CHIP_SCRIPT="$TEMP_DIR/extract_chips.py"
    cat > "$PYTHON_CHIP_SCRIPT" <<PYTHON_EOF
import csv
import sys
import os

csv_file = "$CSV_SORTED"
image_list_file = "$IMAGE_LIST"

# Load image list to check which chips have images
chips_with_images = set()
if os.path.exists(image_list_file):
    with open(image_list_file, 'r') as f:
        for line in f:
            line = line.strip()
            if line.endswith('_layout.png'):
                basename = os.path.basename(line)
                pname = basename.replace('_layout.png', '')
                chips_with_images.add(pname)

# Read sorted CSV and output chips that have images
with open(csv_file, 'r', encoding='utf-8') as f:
    reader = csv.reader(f)
    next(reader)  # Skip header
    for row in reader:
        if len(row) >= 16:
            pname = row[3].strip()
            username = row[2].strip()
            normalized_date = row[14].strip()
            
            if pname and pname in chips_with_images:
                # Find thumbnail path
                thumbnail = None
                with open(image_list_file, 'r') as imgf:
                    for img_line in imgf:
                        img_line = img_line.strip()
                        if img_line.endswith(f"{pname}_layout.png"):
                            thumbnail = img_line.replace('_layout.png', '_layout_thumbnail.png')
                            if os.path.exists(thumbnail):
                                print(f"{normalized_date}|{username}|{pname}|{thumbnail}")
                                break
PYTHON_EOF
    
    python3 "$PYTHON_CHIP_SCRIPT" > "$TEMP_DIR/all_available_chips.txt" 2>&1
    
    TOTAL_AVAILABLE=$(wc -l < "$TEMP_DIR/all_available_chips.txt" 2>/dev/null || echo "0")
    if [ "$TOTAL_AVAILABLE" -eq 0 ]; then
        echo "ERROR: No chips found!" >&2
        echo "Python script output:" >&2
        head -20 "$TEMP_DIR/all_available_chips.txt" >&2
        exit 1
    fi
    echo "Found $TOTAL_AVAILABLE chips with images"
    
    # Apply FILE_LIMIT if set
    if [ "$FILE_LIMIT" -gt 0 ] && [ "$FILE_LIMIT" -lt "$TOTAL_AVAILABLE" ]; then
        head -n "$FILE_LIMIT" "$TEMP_DIR/all_available_chips.txt" > "$TEMP_DIR/all_available_chips_limited.txt"
        mv "$TEMP_DIR/all_available_chips_limited.txt" "$TEMP_DIR/all_available_chips.txt"
        TOTAL_AVAILABLE="$FILE_LIMIT"
        echo "Limited to $FILE_LIMIT chips (FILE_LIMIT set)"
    fi
    
    # Add index to all chips: index|normalized_date|username|pname|thumbnail
    awk '{print NR "|" $0}' "$TEMP_DIR/all_available_chips.txt" > "$SORTED_LIST"
    TOTAL_SELECTED="$TOTAL_AVAILABLE"
    echo "Using all $TOTAL_SELECTED chips"
    echo ""
    
    # Step 2: Create annotated images cache
    echo "Step 2: Creating annotated images cache..."
    mkdir -p "$ANNOTATED_DIR"
    
    CACHE_MAP="$CACHE_DIR/chip_cache_map.txt"
    > "$CACHE_MAP"
    
    TOTAL_TO_CACHE="$TOTAL_AVAILABLE"
    CACHED=0
    CREATED=0
    COUNT=0
    
    echo "Caching annotated images for all $TOTAL_TO_CACHE chips..."
    
    while IFS='|' read -r normalized_date username pname thumbnail; do
        ((COUNT++))
        
        cached_image="$ANNOTATED_DIR/$(printf "%06d.png" $COUNT)"
        echo "$COUNT|$pname|$cached_image" >> "$CACHE_MAP"
        
        if [ -f "$cached_image" ]; then
            if identify "$cached_image" >/dev/null 2>&1; then
                ((CACHED++))
                if [ $((COUNT % 100)) -eq 0 ]; then
                    echo "  Cached $COUNT/$TOTAL_TO_CACHE chips... (cached: $CACHED, created: $CREATED)"
                fi
                continue
            else
                rm -f "$cached_image"
            fi
        fi
        
        year=$(extract_year "$normalized_date")
        if [ -z "$year" ] || [ "$year" = "9999" ]; then
            year="?"
        fi
        chip_name=$(extract_chip_name "$pname")
        
        overlay_title_block "$thumbnail" "$cached_image" "$year" "$chip_name" "$username"
        ((CREATED++))
        
        if [ $((COUNT % 100)) -eq 0 ]; then
            echo "  Cached $COUNT/$TOTAL_TO_CACHE chips... (cached: $CACHED, created: $CREATED)"
        fi
    done < "$TEMP_DIR/all_available_chips.txt"
    
    echo "Cache complete: $CACHED cached, $CREATED newly created (total: $COUNT chips)"
    echo ""
    
    # Step 3: Create mapping for selected chips
    echo "Step 3: Creating mapping for selected chips..."
    CHIP_YEAR_MAP="$TEMP_DIR/chip_year_map.txt"
    > "$CHIP_YEAR_MAP"
    
    chip_idx=0
    while IFS='|' read -r chip_index normalized_date username pname thumbnail; do
        ((chip_idx++))
        
        cached_image="$ANNOTATED_DIR/$(printf "%06d.png" $chip_index)"
        
        year=$(extract_year "$normalized_date")
        if [ -z "$year" ] || [ "$year" = "9999" ]; then
            year="?"
        fi
        chip_name=$(extract_chip_name "$pname")
        
        echo "$chip_idx|$year|$normalized_date|$chip_name|$username|$pname|$cached_image" >> "$CHIP_YEAR_MAP"
    done < "$SORTED_LIST"
    
    echo "Created mapping for $chip_idx selected chips"
    echo ""
    
    # Get video dimensions from first chip image
    VIDEO_WIDTH=""
    VIDEO_HEIGHT=""
    if [ "$TOTAL_SELECTED" -gt 0 ]; then
        first_chip_info=$(awk -F'|' -v idx="1" '$1 == idx {print $0; exit}' "$CHIP_YEAR_MAP")
        if [ -n "$first_chip_info" ]; then
            first_chip_image=$(echo "$first_chip_info" | cut -d'|' -f7)
            if [ -f "$first_chip_image" ]; then
                VIDEO_WIDTH=$(identify -format "%w" "$first_chip_image" 2>/dev/null)
                VIDEO_HEIGHT=$(identify -format "%h" "$first_chip_image" 2>/dev/null)
                if [ -n "$VIDEO_WIDTH" ] && [ -n "$VIDEO_HEIGHT" ]; then
                    echo "Video dimensions determined from chip images: ${VIDEO_WIDTH}x${VIDEO_HEIGHT}"
                fi
            fi
        fi
    fi
    if [ -z "$VIDEO_WIDTH" ] || [ -z "$VIDEO_HEIGHT" ]; then
        echo "Warning: Could not determine video dimensions, using default 1920x1080"
        VIDEO_WIDTH=1920
        VIDEO_HEIGHT=1080
    fi
    echo ""
    
    # Step 4: Create video directly from images (one frame per chip, with 0.5s pause on first and last)
    echo "Step 4: Creating video from chip images..."
    
    # Calculate durations
    PAUSE_DURATION="0.5"
    SINGLE_FRAME_DURATION=$(echo "scale=6; 1 / $VIDEO_FPS" | bc -l 2>/dev/null || echo "0.016667")
    
    # Build list of images with their durations for ffmpeg concat filter
    IMAGE_LIST_FILE="$TEMP_DIR/image_list.txt"
    > "$IMAGE_LIST_FILE"
    
    FRAME_COUNT=0
    
    # First frame: 0.5s pause (showing first chip)
    if [ "$TOTAL_SELECTED" -gt 0 ]; then
        first_chip_info=$(awk -F'|' -v idx="1" '$1 == idx {print $0; exit}' "$CHIP_YEAR_MAP")
        if [ -n "$first_chip_info" ]; then
            first_chip_image=$(echo "$first_chip_info" | cut -d'|' -f7)
            if [ -f "$first_chip_image" ]; then
                abs_path=$(realpath "$first_chip_image")
                echo "$abs_path|$PAUSE_DURATION" >> "$IMAGE_LIST_FILE"
                ((FRAME_COUNT++))
            fi
        fi
    fi
    
    # All chips: one frame per chip (including first and last)
    for chip_num in $(seq 1 $TOTAL_SELECTED); do
        chip_info=$(awk -F'|' -v idx="$chip_num" '$1 == idx {print $0; exit}' "$CHIP_YEAR_MAP")
        if [ -z "$chip_info" ]; then
            continue
        fi
        
        frame_file=$(echo "$chip_info" | cut -d'|' -f7)
        
        if [ ! -f "$frame_file" ]; then
            continue
        fi
        
        abs_path=$(realpath "$frame_file")
        echo "$abs_path|$SINGLE_FRAME_DURATION" >> "$IMAGE_LIST_FILE"
        ((FRAME_COUNT++))
    done
    
    # Last frame: 0.5s pause (use last chip)
    if [ "$TOTAL_SELECTED" -gt 0 ]; then
        last_chip_info=$(awk -F'|' -v idx="$TOTAL_SELECTED" '$1 == idx {print $0; exit}' "$CHIP_YEAR_MAP")
        if [ -n "$last_chip_info" ]; then
            last_chip_image=$(echo "$last_chip_info" | cut -d'|' -f7)
            if [ -f "$last_chip_image" ]; then
                abs_path=$(realpath "$last_chip_image")
                echo "$abs_path|$PAUSE_DURATION" >> "$IMAGE_LIST_FILE"
                ((FRAME_COUNT++))
            fi
        fi
    fi
    
    echo "Created image list with $FRAME_COUNT entries"
    echo ""
    
    # Step 5: Create uncompressed video segments (much faster, no encoding)
    # Then concatenate and transcode to MP4 in final step
    echo "Step 5: Creating uncompressed video segments from images..."
    
    SEGMENTS_DIR="$TEMP_DIR/segments"
    mkdir -p "$SEGMENTS_DIR"
    SEGMENT_LIST="$TEMP_DIR/segments.txt"
    > "$SEGMENT_LIST"
    
    # Calculate frame counts
    PAUSE_FRAMES=$(echo "scale=0; (0.5 * $VIDEO_FPS) / 1" | bc -l 2>/dev/null || echo "30")
    SINGLE_FRAME=1
    
    TOTAL_IMAGES=$(wc -l < "$IMAGE_LIST_FILE" 2>/dev/null || echo "0")
    echo "Processing $TOTAL_IMAGES images (creating uncompressed segments)..."
    
    SEGMENT_COUNT=0
    IMAGE_COUNT=0
    SKIPPED_COUNT=0
    
    # Process each image - create uncompressed rawvideo segments
    # Use file descriptor to avoid subshell issues
    exec 3< "$IMAGE_LIST_FILE"
    while IFS='|' read -r image_path duration <&3; do
        # Skip empty lines
        [ -z "$image_path" ] && [ -z "$duration" ] && continue
        
        if [ ! -f "$image_path" ]; then
            ((SKIPPED_COUNT++))
            continue
        fi
        
        ((IMAGE_COUNT++))
        
        # Calculate frame count from duration
        if [ "$(echo "$duration >= 0.5" | bc -l 2>/dev/null || echo "0")" = "1" ]; then
            # Pause segment (0.5s)
            frame_count="$PAUSE_FRAMES"
        else
            # Single frame
            frame_count="$SINGLE_FRAME"
        fi
        
        # Use rawvideo format (uncompressed) - much faster
        segment_file="$SEGMENTS_DIR/segment_$(printf "%06d" $SEGMENT_COUNT).yuv"
        
        # Create uncompressed segment - no encoding, just scaling
        ffmpeg -y -loop 1 -i "$image_path" \
            -frames:v "$frame_count" \
            -vf "scale=${VIDEO_WIDTH}:${VIDEO_HEIGHT}:flags=lanczos" \
            -r "$VIDEO_FPS" \
            -f rawvideo \
            -pix_fmt yuv420p \
            "$segment_file" \
            -loglevel error 2>"$TEMP_DIR/ffmpeg_seg_${SEGMENT_COUNT}.log"
        
        if [ -f "$segment_file" ] && [ -s "$segment_file" ]; then
            # Store segment info: file path, frame count, width, height
            echo "$segment_file|$frame_count|${VIDEO_WIDTH}|${VIDEO_HEIGHT}" >> "$SEGMENT_LIST"
            ((SEGMENT_COUNT++))
            
            # Progress update every 50 images
            if [ $((IMAGE_COUNT % 50)) -eq 0 ]; then
                echo "  Processed $IMAGE_COUNT/$TOTAL_IMAGES images..."
            fi
        else
            echo "Warning: Failed to create segment for $image_path" >&2
        fi
    done
    exec 3<&-
    
    echo "Created $SEGMENT_COUNT uncompressed segments"
    if [ "$SKIPPED_COUNT" -gt 0 ]; then
        echo "  Skipped (file not found): $SKIPPED_COUNT images"
    fi
    echo ""
    
    # Step 6: Concatenate uncompressed segments into single rawvideo file, then encode to MP4
    echo "Step 6: Concatenating rawvideo segments..."
    
    # Calculate frame size for yuv420p
    FRAME_SIZE=$((VIDEO_WIDTH * VIDEO_HEIGHT * 3 / 2))
    CONCAT_RAWVIDEO="$TEMP_DIR/concat_rawvideo.yuv"
    
    # Concatenate all rawvideo segments by simply appending bytes
    > "$CONCAT_RAWVIDEO"
    SEGMENT_COUNT=0
    
    while IFS='|' read -r segment_file frame_count width height; do
        if [ -f "$segment_file" ] && [ -s "$segment_file" ]; then
            cat "$segment_file" >> "$CONCAT_RAWVIDEO"
            ((SEGMENT_COUNT++))
        fi
    done < "$SEGMENT_LIST"
    
    echo "Concatenated $SEGMENT_COUNT segments into rawvideo file"
    echo ""
    
    # Step 7: Encode concatenated rawvideo to MP4
    echo "Step 7: Encoding to MP4..."
    
    # Calculate total frames from segment list
    TOTAL_FRAMES=0
    PAUSE_FRAME_COUNT=0
    CHIP_FRAME_COUNT=0
    
    while IFS='|' read -r segment_file frame_count width height; do
        TOTAL_FRAMES=$((TOTAL_FRAMES + frame_count))
        if [ "$frame_count" -eq "$PAUSE_FRAMES" ]; then
            PAUSE_FRAME_COUNT=$((PAUSE_FRAME_COUNT + frame_count))
        else
            CHIP_FRAME_COUNT=$((CHIP_FRAME_COUNT + frame_count))
        fi
    done < "$SEGMENT_LIST"
    
    # Calculate expected duration
    TOTAL_DURATION=$(echo "scale=2; $TOTAL_FRAMES / $VIDEO_FPS" | bc -l 2>/dev/null || echo "0")
    CHIP_DURATION=$(echo "scale=2; $CHIP_FRAME_COUNT / $VIDEO_FPS" | bc -l 2>/dev/null || echo "0")
    PAUSE_DURATION=$(echo "scale=2; $PAUSE_FRAME_COUNT / $VIDEO_FPS" | bc -l 2>/dev/null || echo "0")
    
    echo "Duration breakdown:"
    echo "  Total frames: $TOTAL_FRAMES"
    echo "  Chip frames: $CHIP_FRAME_COUNT ($CHIP_DURATION seconds)"
    echo "  Pause frames: $PAUSE_FRAME_COUNT ($PAUSE_DURATION seconds)"
    echo "  Total duration: $TOTAL_DURATION seconds"
    echo ""
    
    # Verify rawvideo file size matches expected frame count
    FRAME_SIZE=$((VIDEO_WIDTH * VIDEO_HEIGHT * 3 / 2))  # yuv420p: Y + U/2 + V/2
    EXPECTED_FILE_SIZE=$((FRAME_SIZE * TOTAL_FRAMES))
    ACTUAL_FILE_SIZE=$(stat -f%z "$CONCAT_RAWVIDEO" 2>/dev/null || stat -c%s "$CONCAT_RAWVIDEO" 2>/dev/null || echo "0")
    
    echo "Verifying rawvideo file:"
    echo "  Expected size: $EXPECTED_FILE_SIZE bytes ($TOTAL_FRAMES frames × $FRAME_SIZE bytes/frame)"
    echo "  Actual size: $ACTUAL_FILE_SIZE bytes"
    
    if [ "$ACTUAL_FILE_SIZE" -ne "$EXPECTED_FILE_SIZE" ]; then
        echo "WARNING: File size mismatch! Expected $EXPECTED_FILE_SIZE bytes, got $ACTUAL_FILE_SIZE bytes"
        echo "  This may cause incorrect duration. Recalculating frames from file size..."
        CALCULATED_FRAMES=$((ACTUAL_FILE_SIZE / FRAME_SIZE))
        echo "  Calculated frames from file size: $CALCULATED_FRAMES"
        if [ "$CALCULATED_FRAMES" -gt 0 ] && [ "$CALCULATED_FRAMES" -ne "$TOTAL_FRAMES" ]; then
            echo "  Using calculated frame count: $CALCULATED_FRAMES"
            TOTAL_FRAMES="$CALCULATED_FRAMES"
        fi
    fi
    echo ""
    
    # Encode rawvideo to MP4
    # Use -sseof to ensure we read exactly the right amount, or let ffmpeg calculate from file size
    ffmpeg -y -f rawvideo \
        -video_size "${VIDEO_WIDTH}x${VIDEO_HEIGHT}" \
        -pixel_format yuv420p \
        -framerate "$VIDEO_FPS" \
        -i "$CONCAT_RAWVIDEO" \
        -frames:v "$TOTAL_FRAMES" \
        -c:v libx264 \
        -preset medium \
        -crf "$CRF" \
        -pix_fmt yuv420p \
        -an \
        -movflags +faststart \
        "$OUTPUT_FILE" \
        -loglevel info
    
    if [ ! -f "$OUTPUT_FILE" ] || [ ! -s "$OUTPUT_FILE" ]; then
        echo "Error: Failed to encode final video"
        exit 1
    fi
    
    if [ ! -f "$OUTPUT_FILE" ] || [ ! -s "$OUTPUT_FILE" ]; then
        echo "Error: Failed to create final video"
        exit 1
    fi
    
    echo "Video creation complete!"
    
    if [ ! -f "$OUTPUT_FILE" ] || [ ! -s "$OUTPUT_FILE" ]; then
        echo "Error: Failed to create video file"
        exit 1
    fi
    
    if [ -f "$OUTPUT_FILE" ] && [ -s "$OUTPUT_FILE" ]; then
        if file "$OUTPUT_FILE" | grep -q "MP4\|ISO Media"; then
            echo ""
            echo "Successfully created: $OUTPUT_FILE"
            echo "File size: $(du -h "$OUTPUT_FILE" | cut -f1)"
            
            if command -v ffprobe >/dev/null 2>&1; then
                DURATION=$(ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 "$OUTPUT_FILE" 2>/dev/null | cut -d. -f1)
                if [ -n "$DURATION" ] && [ "$DURATION" != "N/A" ]; then
                    echo "Duration: ${DURATION} seconds"
                fi
            fi
            
            OUTPUT_ABS=$(realpath "$OUTPUT_FILE")
            echo ""
            echo "Output file (absolute path):"
            echo "$OUTPUT_ABS"
        else
            echo ""
            echo "Error: Output file exists but is not a valid MP4"
            rm -f "$OUTPUT_FILE"
            exit 1
        fi
    else
        echo ""
        echo "Error: Failed to create video file"
        exit 1
    fi
    
    # Short mode complete - exit early
    exit 0
fi

# Normal mode continues below with audio processing
# Step 1: Extract audio segment first
echo "Step 1: Extracting audio segment (${VIDEO_DURATION}s from ${AUDIO_START}s)..."
EXTRACTED_AUDIO="$TEMP_DIR/audio_segment.m4a"

# Extract audio segment
ffmpeg -y -i "$AUDIO_FILE" -ss "$AUDIO_START" -t "$VIDEO_DURATION" -c copy "$EXTRACTED_AUDIO" -loglevel error

if [ ! -f "$EXTRACTED_AUDIO" ] || [ ! -s "$EXTRACTED_AUDIO" ]; then
    echo "Error: Failed to extract audio segment"
    exit 1
fi

# Step 2: Detect beats directly from extracted audio segment
# Beats will be relative to 0 (start of the extracted segment)
echo "Step 2: Detecting beats in audio segment..."
echo "  Using threshold: ${BEAT_THRESHOLD} (lower = more sensitive, detects more beats)"
BEATS_FILE="$TEMP_DIR/beats.txt"

# Beat detection: Run aubioonset and capture both output and errors for debugging
aubioonset -i "$EXTRACTED_AUDIO" -t "$BEAT_THRESHOLD" -O "$BEAT_METHOD" > "$BEATS_FILE" 2>"$TEMP_DIR/aubioonset_errors.txt"

if [ ! -s "$BEATS_FILE" ]; then
    echo "Warning: No beats detected with threshold ${BEAT_THRESHOLD}, using uniform timing"
    if [ -s "$TEMP_DIR/aubioonset_errors.txt" ]; then
        echo "  aubioonset errors:"
        cat "$TEMP_DIR/aubioonset_errors.txt" | sed 's/^/    /'
    fi
    seq 0 0.5 "$VIDEO_DURATION" > "$BEATS_FILE"
fi

BEAT_COUNT=$(wc -l < "$BEATS_FILE")
echo "  Detected $BEAT_COUNT beats using threshold ${BEAT_THRESHOLD}"
if [ "$BEAT_COUNT" -gt 0 ] && [ "$BEAT_COUNT" -lt 10 ]; then
    echo "  First few beat times:"
    head -n 5 "$BEATS_FILE" | awk '{printf "    %.3fs\n", $1}'
fi
echo ""

# Step 3: Calculate segments from beats (one segment starts at each beat)
echo "Step 3: Creating segments from beats..."
BEAT_INTERVALS="$TEMP_DIR/beat_intervals.txt"
> "$BEAT_INTERVALS"

# Use Python to calculate beat intervals and determine how many chips we need
# This script creates intervals from beat times, ensuring no quantization errors accumulate.
# FRAME QUANTIZATION: To prevent accumulated quantization errors, we quantize absolute
# beat times to frame boundaries, then calculate frame counts. This ensures continuity
# and prevents errors from accumulating across segments.
PYTHON_BEAT_SCRIPT="$TEMP_DIR/calculate_beat_intervals.py"
cat > "$PYTHON_BEAT_SCRIPT" <<PYTHON_EOF
import sys
import math

beats_file = "$BEATS_FILE"
intervals_file = "$BEAT_INTERVALS"
video_duration = float("$VIDEO_DURATION")
video_fps = float("$VIDEO_FPS")

frame_interval = 1.0 / video_fps

# Read beats
beats = []
try:
    with open(beats_file, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                beat_time = float(line)
                if 0 <= beat_time <= video_duration:
                    beats.append(beat_time)
            except (ValueError, TypeError):
                continue
except (IOError, OSError) as e:
    print(f"Error reading beats file: {e}", file=sys.stderr)
    sys.exit(1)

# Sort beats (should already be sorted, but ensure)
beats = sorted(set(beats))

# FRAME QUANTIZATION STRATEGY:
# To minimize accumulated quantization errors, we quantize absolute times to frame
# boundaries, then calculate frame counts. This ensures:
# 1. Continuity: Each segment starts exactly where previous ended (at frame boundary)
# 2. No accumulation: Errors don't compound because we use absolute quantized times
# 3. Perfect total: Total frames = quantized(video_duration) * fps (exact match)

# Quantize video duration to frame boundary
video_duration_frames = round(video_duration * video_fps)
quantized_video_duration = video_duration_frames / video_fps

# Quantize all beats to frame boundaries
quantized_beats = [round(beat * video_fps) / video_fps for beat in beats]
quantized_beats = sorted(set(quantized_beats))

# Create intervals using quantized times
intervals = []

# If first beat is not at 0, add initial segment from 0 to first beat
if quantized_beats and quantized_beats[0] > 0:
    start_frame = 0
    end_frame = round(quantized_beats[0] * video_fps)
    frame_count = end_frame - start_frame
    quantized_start = start_frame / video_fps
    quantized_end = end_frame / video_fps
    quantized_duration = frame_count / video_fps
    intervals.append((quantized_start, quantized_end, quantized_duration, frame_count))

# Create segments starting at each quantized beat
for i in range(len(quantized_beats)):
    start_time = quantized_beats[i]
    start_frame = round(start_time * video_fps)
    
    # End is next beat, or quantized video duration if last beat
    if i + 1 < len(quantized_beats):
        end_time = quantized_beats[i + 1]
    else:
        end_time = quantized_video_duration
    
    end_frame = round(end_time * video_fps)
    frame_count = end_frame - start_frame
    
    if frame_count > 0:
        quantized_duration = frame_count / video_fps
        intervals.append((start_time, end_time, quantized_duration, frame_count))

# Validate intervals to ensure no gaps, overlaps, or quantization errors
if intervals:
    # Check continuity: each interval should start where previous ended
    prev_end = None
    prev_end_frame = None
    for start, end, duration, frame_count in intervals:
        start_frame = round(start * video_fps)
        end_frame = round(end * video_fps)
        
        if prev_end_frame is not None:
            gap_frames = start_frame - prev_end_frame
            if gap_frames != 0:
                gap_time = gap_frames / video_fps
                print(f"WARNING: Gap/overlap detected: {gap_frames} frames ({gap_time:.9f}s)", file=sys.stderr)
        prev_end = end
        prev_end_frame = end_frame
    
    # Check total duration matches quantized video duration
    total_frames = sum(iv[3] for iv in intervals)
    total_duration = total_frames / video_fps
    duration_error = abs(total_duration - quantized_video_duration)
    if duration_error > frame_interval / 2:  # Allow half frame tolerance
        print(f"WARNING: Total frames ({total_frames}) doesn't match expected ({video_duration_frames}), error: {duration_error:.9f}s", file=sys.stderr)
    
    # Check first interval starts at 0
    if intervals[0][0] != 0.0:
        print(f"WARNING: First interval starts at {intervals[0][0]}, not 0.0", file=sys.stderr)
    
    # Check last interval ends at quantized video duration
    if abs(intervals[-1][1] - quantized_video_duration) > frame_interval / 2:
        print(f"WARNING: Last interval ends at {intervals[-1][1]:.9f}, not {quantized_video_duration:.9f}", file=sys.stderr)

# Write intervals: start_time|end_time|duration|frame_count
# Format: quantized start, quantized end, quantized duration, frame count
# Using 6 decimal places for times, integer frame counts
with open(intervals_file, 'w') as f:
    for start, end, duration, frame_count in intervals:
        f.write(f"{start:.6f}|{end:.6f}|{duration:.6f}|{frame_count}\n")

# Calculate required chip count
required_chips = len(intervals)
total_frames = sum(iv[3] for iv in intervals)
print(f"Required chips: {required_chips}")
print(f"  - Chips change exactly at each beat (no pauses)")
print(f"  - Number of intervals: {len(intervals)}")
print(f"  - Total frames: {total_frames} (quantized duration: {total_frames / video_fps:.6f}s)")
print(f"  - Frame quantization: beats quantized to frame boundaries ({frame_interval:.9f}s per frame)")
if intervals:
    if quantized_beats and quantized_beats[0] > 0:
        first_frames = intervals[0][3]
        print(f"  - First interval: 0.0s to {quantized_beats[0]:.3f}s ({first_frames} frames)")
    min_frames = min(iv[3] for iv in intervals)
    max_frames = max(iv[3] for iv in intervals)
    min_duration = min_frames / video_fps
    max_duration = max_frames / video_fps
    print(f"  - Frame count range: {min_frames}-{max_frames} frames ({min_duration:.3f}s to {max_duration:.3f}s)")

# No minimum dwell time check - use all intervals as-is

sys.exit(0)
PYTHON_EOF

python3 "$PYTHON_BEAT_SCRIPT" 2>&1
REQUIRED_CHIPS=$(python3 -c "
beats_file = '$BEATS_FILE'
video_duration = float('$VIDEO_DURATION')

beats = []
with open(beats_file, 'r') as f:
    for line in f:
        line = line.strip()
        if line:
            try:
                beat = float(line)
                if 0 <= beat <= video_duration:
                    beats.append(beat)
            except:
                pass

beats = sorted(set(beats))
# Count intervals: one per interval between consecutive beats
# Plus one initial interval if first beat is not at 0
if beats:
    required = len(beats)  # One interval per beat (each beat to next)
    if beats[0] > 0:
        required += 1  # Add initial interval from 0 to first beat
else:
    required = 1  # At least one chip if no beats
print(required)
")
echo "Chips required for beat-synchronized video: $REQUIRED_CHIPS"
echo ""

# Step 4: Load sorted CSV and randomly sample chips
echo "Step 4: Loading sorted chip database and selecting chips..."
CSV_SORTED="${CSV_SORTED:-$CHIP_DIR/chip_database_sorted.csv}"

if [ ! -f "$CSV_SORTED" ]; then
    echo "Warning: Sorted CSV not found at $CSV_SORTED"
    echo "Creating sorted CSV from $CSV_DATABASE..."
    "$(dirname "$0")/sort_chip_database_by_date.sh"
    CSV_SORTED="$CHIP_DIR/chip_database_sorted.csv"
fi

if [ ! -f "$CSV_SORTED" ]; then
    echo "Error: Could not create or find sorted CSV file"
    exit 1
fi

# Extract chips from sorted CSV and match with image list
SORTED_LIST="$TEMP_DIR/selected_chips.txt"
> "$SORTED_LIST"

# Load CSV data into associative arrays
declare -A csv_normalized_date
declare -A csv_username
declare -A csv_pname
declare -A csv_path

# Read sorted CSV and extract chip info
# Write Python script to file to ensure proper output capture
PYTHON_CHIP_SCRIPT="$TEMP_DIR/extract_chips.py"
cat > "$PYTHON_CHIP_SCRIPT" <<PYTHON_EOF
import csv
import sys
import os

csv_file = "$CSV_SORTED"
image_list_file = "$IMAGE_LIST"

# Load image list to check which chips have images
chips_with_images = set()
if os.path.exists(image_list_file):
    with open(image_list_file, 'r') as f:
        for line in f:
            line = line.strip()
            if line.endswith('_layout.png'):
                basename = os.path.basename(line)
                pname = basename.replace('_layout.png', '')
                chips_with_images.add(pname)

# Read sorted CSV and output chips that have images
with open(csv_file, 'r', encoding='utf-8') as f:
    reader = csv.reader(f)
    next(reader)  # Skip header
    for row in reader:
        if len(row) >= 16:
            pname = row[3].strip()
            username = row[2].strip()
            normalized_date = row[14].strip()
            
            if pname and pname in chips_with_images:
                # Find thumbnail path
                thumbnail = None
                with open(image_list_file, 'r') as imgf:
                    for img_line in imgf:
                        img_line = img_line.strip()
                        if img_line.endswith(f"{pname}_layout.png"):
                            thumbnail = img_line.replace('_layout.png', '_layout_thumbnail.png')
                            if os.path.exists(thumbnail):
                                print(f"{normalized_date}|{username}|{pname}|{thumbnail}")
                                break
PYTHON_EOF

# Execute Python script and capture output
python3 "$PYTHON_CHIP_SCRIPT" > "$TEMP_DIR/all_available_chips.txt" 2>&1

TOTAL_AVAILABLE=$(wc -l < "$TEMP_DIR/all_available_chips.txt" 2>/dev/null || echo "0")
if [ "$TOTAL_AVAILABLE" -eq 0 ]; then
    echo "ERROR: No chips found!" >&2
    echo "Python script output:" >&2
    head -20 "$TEMP_DIR/all_available_chips.txt" >&2
    exit 1
fi
echo "Found $TOTAL_AVAILABLE chips with images in sorted database"

# Randomly subsample to required number of chips, maintaining chronological order
# Preserve index (line number) in SORTED_LIST for easy cache lookup
# If VIDEO_DURATION was originally 0, use all available chips (don't subsample)
USE_ALL_CHIPS=false
if [ "${ORIGINAL_VIDEO_DURATION:-}" = "0" ]; then
    USE_ALL_CHIPS=true
fi

if [ "$USE_ALL_CHIPS" = true ]; then
    echo "Using all $TOTAL_AVAILABLE available chips (VIDEO_DURATION=0 mode)"
    # Add index to all chips: index|normalized_date|username|pname|thumbnail
    awk '{print NR "|" $0}' "$TEMP_DIR/all_available_chips.txt" > "$SORTED_LIST"
    # Update REQUIRED_CHIPS to match available chips (will freeze last chip if needed)
    if [ "$TOTAL_AVAILABLE" -lt "$REQUIRED_CHIPS" ]; then
        echo "  Note: Have $TOTAL_AVAILABLE chips but $REQUIRED_CHIPS beats - will freeze last chip image"
    else
        REQUIRED_CHIPS="$TOTAL_AVAILABLE"
    fi
elif [ "$TOTAL_AVAILABLE" -gt "$REQUIRED_CHIPS" ]; then
    echo "Randomly subsampling from $TOTAL_AVAILABLE to $REQUIRED_CHIPS chips (maintaining date order)..."
    # Add line numbers, shuffle, take first N, then sort by line number to maintain order
    # Keep index in output: index|normalized_date|username|pname|thumbnail
    awk '{print NR "|" $0}' "$TEMP_DIR/all_available_chips.txt" | shuf | head -n "$REQUIRED_CHIPS" | sort -t'|' -k1,1n > "$SORTED_LIST"
else
    echo "Using all $TOTAL_AVAILABLE available chips (need $REQUIRED_CHIPS)"
    # Add index to all chips: index|normalized_date|username|pname|thumbnail
    awk '{print NR "|" $0}' "$TEMP_DIR/all_available_chips.txt" > "$SORTED_LIST"
    REQUIRED_CHIPS="$TOTAL_AVAILABLE"
fi

TOTAL_SELECTED=$(wc -l < "$SORTED_LIST")
echo "Selected $TOTAL_SELECTED chips for video"
echo ""

# Extract year and chip name for each selected chip
# SORTED_LIST format: index|normalized_date|username|pname|thumbnail
SELECTED_LIST="$TEMP_DIR/selected_chips_annotated.txt"
> "$SELECTED_LIST"
chip_idx=0
while IFS='|' read -r chip_index normalized_date username pname thumbnail; do
    ((chip_idx++))
    year=$(extract_year "$normalized_date")
    if [ -z "$year" ] || [ "$year" = "9999" ]; then
        year="?"
    fi
    chip_name=$(extract_chip_name "$pname")
    echo "$chip_idx|$normalized_date|$year|$chip_name|$username|$pname|$thumbnail" >> "$SELECTED_LIST"
done < "$SORTED_LIST"

# Debug: Show selected chips
echo "DEBUG: Selected chips (first 5 and last 5):"
head -n 5 "$SELECTED_LIST" | awk -F'|' '{printf "  Chip %d: %s (%s) - %s\n", $1, $4, $3, $2}'
if [ "$TOTAL_SELECTED" -gt 10 ]; then
    echo "  ..."
    tail -n 5 "$SELECTED_LIST" | awk -F'|' '{printf "  Chip %d: %s (%s) - %s\n", $1, $4, $3, $2}'
fi
echo ""

# Step 5: Create annotated images cache for ALL chips (one-time, reusable)
echo "Step 5: Creating annotated images cache for all chips..."
mkdir -p "$ANNOTATED_DIR"

# Cache all available chips using numerical index based on sorted CSV order
# Cache files: 000001.png, 000002.png, etc. (index corresponds to line number in all_available_chips.txt)
CACHE_MAP="$CACHE_DIR/chip_cache_map.txt"
> "$CACHE_MAP"

TOTAL_TO_CACHE=$(wc -l < "$TEMP_DIR/all_available_chips.txt")
CACHED=0
CREATED=0
COUNT=0

echo "Caching annotated images for all $TOTAL_TO_CACHE chips (using numerical index as cache key)..."

while IFS='|' read -r normalized_date username pname thumbnail; do
    ((COUNT++))
    
    # Use numerical index as cache filename (000001.png, 000002.png, etc.)
    cached_image="$ANNOTATED_DIR/$(printf "%06d.png" $COUNT)"
    
    # Store mapping: index|pname -> cached image path (for reference/debugging)
    echo "$COUNT|$pname|$cached_image" >> "$CACHE_MAP"
    
    # Check if cached image exists and is valid
    if [ -f "$cached_image" ]; then
        if identify "$cached_image" >/dev/null 2>&1; then
            ((CACHED++))
            if [ $((COUNT % 100)) -eq 0 ]; then
                echo "  Cached $COUNT/$TOTAL_TO_CACHE chips... (cached: $CACHED, created: $CREATED)"
            fi
            continue
        else
            # Invalid cached file, remove it
            rm -f "$cached_image"
        fi
    fi
    
    # Create new annotated image
    year=$(extract_year "$normalized_date")
    if [ -z "$year" ] || [ "$year" = "9999" ]; then
        year="?"
    fi
    chip_name=$(extract_chip_name "$pname")
    
    overlay_title_block "$thumbnail" "$cached_image" "$year" "$chip_name" "$username"
    ((CREATED++))
    
    if [ $((COUNT % 100)) -eq 0 ]; then
        echo "  Cached $COUNT/$TOTAL_TO_CACHE chips... (cached: $CACHED, created: $CREATED)"
    fi
done < "$TEMP_DIR/all_available_chips.txt"

echo "Cache complete: $CACHED cached, $CREATED newly created (total: $COUNT chips)"
echo ""

# Step 5.5: Create mapping for selected chips (using numerical index from all_available_chips.txt)
echo "Creating mapping for selected chips..."
CHIP_YEAR_MAP="$TEMP_DIR/chip_year_map.txt"
> "$CHIP_YEAR_MAP"

chip_idx=0
# SORTED_LIST format: index|normalized_date|username|pname|thumbnail
while IFS='|' read -r chip_index normalized_date username pname thumbnail; do
    ((chip_idx++))
    
    # Use numerical index directly from SORTED_LIST to get cached image path
    cached_image="$ANNOTATED_DIR/$(printf "%06d.png" $chip_index)"
    
    # Get chip info
    year=$(extract_year "$normalized_date")
    if [ -z "$year" ] || [ "$year" = "9999" ]; then
        year="?"
    fi
    chip_name=$(extract_chip_name "$pname")
    
    # Store mapping: chip_idx|year|normalized_date|chip_name|username|pname|cached_image_path
    echo "$chip_idx|$year|$normalized_date|$chip_name|$username|$pname|$cached_image" >> "$CHIP_YEAR_MAP"
done < "$SORTED_LIST"

echo "Created mapping for $chip_idx selected chips"
echo ""

# Get video dimensions from first chip image (for photo scaling)
VIDEO_WIDTH=""
VIDEO_HEIGHT=""
if [ "$TOTAL_SELECTED" -gt 0 ]; then
    first_chip_info=$(awk -F'|' -v idx="1" '$1 == idx {print $0; exit}' "$CHIP_YEAR_MAP")
    if [ -n "$first_chip_info" ]; then
        first_chip_image=$(echo "$first_chip_info" | cut -d'|' -f7)
        if [ -f "$first_chip_image" ]; then
            VIDEO_WIDTH=$(identify -format "%w" "$first_chip_image" 2>/dev/null)
            VIDEO_HEIGHT=$(identify -format "%h" "$first_chip_image" 2>/dev/null)
            if [ -n "$VIDEO_WIDTH" ] && [ -n "$VIDEO_HEIGHT" ]; then
                echo "Video dimensions determined from chip images: ${VIDEO_WIDTH}x${VIDEO_HEIGHT}"
            fi
        fi
    fi
fi
if [ -z "$VIDEO_WIDTH" ] || [ -z "$VIDEO_HEIGHT" ]; then
    echo "Warning: Could not determine video dimensions, using default 1920x1080"
    VIDEO_WIDTH=1920
    VIDEO_HEIGHT=1080
fi
echo ""

# Function to process photo: check aspect ratio, split if tall, scale to video size
# Returns: number of segments created, and sets global PHOTO_SEGMENTS array
process_photo() {
    local photo_path="$1"
    local photo_index="$2"
    local output_dir="$3"
    
    if [ ! -f "$photo_path" ]; then
        echo "0"
        return
    fi
    
    # Get photo dimensions
    local photo_width=$(identify -format "%w" "$photo_path" 2>/dev/null)
    local photo_height=$(identify -format "%h" "$photo_path" 2>/dev/null)
    
    if [ -z "$photo_width" ] || [ -z "$photo_height" ] || [ "$photo_width" -eq 0 ] || [ "$photo_height" -eq 0 ]; then
        echo "0"
        return
    fi
    
    # Calculate aspect ratio (height/width)
    local aspect_ratio=$(echo "scale=2; $photo_height / $photo_width" | bc -l 2>/dev/null || echo "1.0")
    
    # Determine number of segments based on aspect ratio
    # Check larger ratios first
    local num_segments=1
    if [ "$(echo "$aspect_ratio >= 3.5" | bc -l 2>/dev/null || echo "0")" = "1" ]; then
        # ~4:1 or taller - split into 4 segments
        num_segments=4
    elif [ "$(echo "$aspect_ratio >= 2.5" | bc -l 2>/dev/null || echo "0")" = "1" ]; then
        # ~3:1 or taller - split into 3 segments
        num_segments=3
    elif [ "$(echo "$aspect_ratio >= 1.8" | bc -l 2>/dev/null || echo "0")" = "1" ]; then
        # ~2:1 or taller - split into 2 segments
        num_segments=2
    fi
    
    # Create segments
    local segment_height=$((photo_height / num_segments))
    local processed_segments=0
    
    for ((seg=0; seg<num_segments; seg++)); do
        local seg_y=$((seg * segment_height))
        local seg_output="$output_dir/photo_${photo_index}_seg_${seg}.png"
        
        if [ $num_segments -eq 1 ]; then
            # Single segment - just scale entire photo to video dimensions
            convert "$photo_path" \
                -resize "${VIDEO_WIDTH}x${VIDEO_HEIGHT}^" \
                -gravity center \
                -extent "${VIDEO_WIDTH}x${VIDEO_HEIGHT}" \
                "$seg_output" 2>/dev/null
        else
            # Multiple segments - crop and scale each segment
            convert "$photo_path" \
                -crop "${photo_width}x${segment_height}+0+${seg_y}" \
                -resize "${VIDEO_WIDTH}x${VIDEO_HEIGHT}^" \
                -gravity center \
                -extent "${VIDEO_WIDTH}x${VIDEO_HEIGHT}" \
                "$seg_output" 2>/dev/null
        fi
        
        if [ -f "$seg_output" ] && [ -s "$seg_output" ]; then
            PHOTO_SEGMENTS[$processed_segments]="$seg_output"
            ((processed_segments++))
        fi
    done
    
    # If no segments were created, create at least one (fallback)
    if [ $processed_segments -eq 0 ]; then
        local seg_output="$output_dir/photo_${photo_index}_seg_0.png"
        convert "$photo_path" \
            -resize "${VIDEO_WIDTH}x${VIDEO_HEIGHT}^" \
            -gravity center \
            -extent "${VIDEO_WIDTH}x${VIDEO_HEIGHT}" \
            "$seg_output" 2>/dev/null
        if [ -f "$seg_output" ] && [ -s "$seg_output" ]; then
            processed_segments=1
        fi
    fi
    
    echo "$processed_segments"
}

# Step 5.6: Load photos from photos directory as fallback
echo "Loading photos from photos directory (fallback when chips run out)..."
PHOTOS_DIR="$CHIP_DIR/photos"
PHOTOS_LIST="$TEMP_DIR/photos_list.txt"
PHOTOS_PROCESSED_DIR="$TEMP_DIR/photos_processed"
mkdir -p "$PHOTOS_PROCESSED_DIR"
> "$PHOTOS_LIST"

# Track photo segments: photo_index -> number of segments
declare -A PHOTO_SEGMENT_COUNT

if [ -d "$PHOTOS_DIR" ]; then
    # Find all image files, sort by name
    find "$PHOTOS_DIR" -type f \( -iname "*.jpg" -o -iname "*.jpeg" -o -iname "*.png" -o -iname "*.gif" -o -iname "*.bmp" \) | sort > "$PHOTOS_LIST"
    TOTAL_PHOTOS=$(wc -l < "$PHOTOS_LIST" 2>/dev/null || echo "0")
    if [ "$TOTAL_PHOTOS" -gt 0 ]; then
        echo "Found $TOTAL_PHOTOS photos in $PHOTOS_DIR"
        echo "Processing photos (checking aspect ratios, splitting tall photos, scaling to ${VIDEO_WIDTH}x${VIDEO_HEIGHT})..."
        
        # Process each photo to determine segment count
        photo_idx=0
        while IFS= read -r photo_path; do
            ((photo_idx++))
            declare -a PHOTO_SEGMENTS
            seg_count=$(process_photo "$photo_path" "$photo_idx" "$PHOTOS_PROCESSED_DIR")
            PHOTO_SEGMENT_COUNT[$photo_idx]=$seg_count
            PHOTO_SEGMENT_CURRENT[$photo_idx]=0
            
            if [ "$seg_count" -gt 1 ]; then
                echo "  Photo $photo_idx: split into $seg_count segments (tall aspect ratio)"
            fi
            
            if [ $((photo_idx % 10)) -eq 0 ]; then
                echo "  Processed $photo_idx/$TOTAL_PHOTOS photos..."
            fi
        done < "$PHOTOS_LIST"
        
        echo "Photo processing complete"
    else
        echo "No photos found in $PHOTOS_DIR"
    fi
else
    echo "Photos directory not found: $PHOTOS_DIR"
    TOTAL_PHOTOS=0
fi
echo ""

# Step 6: Create video segments from beats (one chip per beat)
echo "Step 6: Creating video segments from beats..."
SEGMENT_LIST="$TEMP_DIR/segments.txt"
> "$SEGMENT_LIST"

# Create segments directly from beat intervals
# Strategy: One chip per beat interval
# - First chip shown from 0 to first beat
# - Subsequent chips shown from one beat to the next

SEGMENT_COUNT=0
SEGMENT_DEBUG_LOG="$TEMP_DIR/segment_debug.log"
> "$SEGMENT_DEBUG_LOG"
FIRST_SEGMENT_YEAR=""
LAST_SEGMENT_YEAR=""

# Ensure we have enough chips
if [ "$TOTAL_SELECTED" -lt 1 ]; then
    echo "ERROR: No chips were selected! Cannot create video."
    echo "Check that chips have matching thumbnails in $IMAGE_LIST"
    exit 1
fi

if [ "$TOTAL_SELECTED" -lt "$REQUIRED_CHIPS" ]; then
    echo "WARNING: Only $TOTAL_SELECTED chips available but $REQUIRED_CHIPS needed"
    if [ "$TOTAL_PHOTOS" -gt 0 ]; then
        echo "Will use photos from $PHOTOS_DIR when chips run out, then freeze on last photo"
    else
        echo "Will freeze last chip image for remaining audio duration"
    fi
    REQUIRED_CHIPS="$TOTAL_SELECTED"
fi

# Ensure SEGMENTS_DIR exists
mkdir -p "$SEGMENTS_DIR"

# Track transition from chips to photos
TRANSITION_CREATED=false
LAST_CHIP_IMAGE=""
LAST_CHIP_YEAR=""
LAST_CHIP_NAME=""

# Track photo segment accumulation (for minimum 2 beats per photo)
PHOTO_ACCUMULATING=false
PHOTO_ACCUM_DURATION=0
PHOTO_ACCUM_FRAMES=0
PHOTO_ACCUM_START_TIME=""
PHOTO_CURRENT_IMAGE=""
PHOTO_CURRENT_YEAR=""
PHOTO_CURRENT_NAME=""
PHOTO_SEGMENT_READY=false

# Read beat intervals and create segments
# Intervals now include frame_count: start_time|end_time|duration|frame_count
# We use frame_count to create segments with exact frame quantization, preventing error accumulation
# Use file descriptor to avoid subshell issues
exec 3< "$BEAT_INTERVALS"
while IFS='|' read -r start_time end_time duration frame_count <&3; do
    # Skip empty lines
    [ -z "$start_time" ] && [ -z "$end_time" ] && continue
    # If frame_count is missing (old format), calculate it
    if [ -z "$frame_count" ]; then
        frame_count=$(echo "scale=0; ($duration * $VIDEO_FPS) / 1" | bc -l 2>/dev/null || echo "1")
    fi
    # Calculate which image to use
    # First use chips, then photos, then freeze on last photo
    # Note: SEGMENT_COUNT includes transition segments, so we need to track original count
    original_segment_count=$SEGMENT_COUNT
    if [ "$TRANSITION_CREATED" = true ]; then
        # Subtract the 2 transition segments (hold + dissolve) from count for chip/photo calculation
        original_segment_count=$((SEGMENT_COUNT - 2))
    fi
    chip_num=$((original_segment_count + 1))
    using_photo=false
    frame_file=""
    chip_year=""
    chip_name=""
    
    if [ "$TOTAL_SELECTED" -gt 0 ] && [ "$chip_num" -le "$TOTAL_SELECTED" ]; then
        # Use chip image
        chip_info=$(awk -F'|' -v idx="$chip_num" '$1 == idx {print $0; exit}' "$CHIP_YEAR_MAP")
        if [ -z "$chip_info" ]; then
            echo "WARNING: No chip info found for chip_num=$chip_num" >> "$SEGMENT_DEBUG_LOG"
            continue
        fi
        
        frame_file=$(echo "$chip_info" | cut -d'|' -f7)
        chip_year=$(echo "$chip_info" | cut -d'|' -f2)
        chip_name=$(echo "$chip_info" | cut -d'|' -f4)
        
        # Save last chip info for transition
        if [ "$chip_num" -eq "$TOTAL_SELECTED" ]; then
            LAST_CHIP_IMAGE="$frame_file"
            LAST_CHIP_YEAR="$chip_year"
            LAST_CHIP_NAME="$chip_name"
        fi
    elif [ "$TOTAL_SELECTED" -gt 0 ] && [ "$TOTAL_PHOTOS" -gt 0 ]; then
        # Use photo segments
        using_photo=true
        
        # Create transition segments if this is the first photo segment
        if [ "$TRANSITION_CREATED" = false ] && [ -n "$LAST_CHIP_IMAGE" ] && [ -f "$LAST_CHIP_IMAGE" ]; then
            echo "Creating transition from last chip to first photo..."
            
            # Calculate frames for hold (0.75s) and dissolve (0.5s)
            hold_frames=$(echo "scale=0; (0.75 * $VIDEO_FPS) / 1" | bc -l 2>/dev/null || echo "45")
            dissolve_frames=$(echo "scale=0; (0.5 * $VIDEO_FPS) / 1" | bc -l 2>/dev/null || echo "30")
            
            # Get first photo segment
            first_photo_idx=1
            first_photo_seg=0
            first_photo_file="$PHOTOS_PROCESSED_DIR/photo_${first_photo_idx}_seg_${first_photo_seg}.png"
            
            if [ ! -f "$first_photo_file" ]; then
                echo "WARNING: First photo segment not found: $first_photo_file" >> "$SEGMENT_DEBUG_LOG"
            else
                # Create 0.75s hold segment of last chip
                hold_segment="$SEGMENTS_DIR/transition_hold_$(printf "%06d" $SEGMENT_COUNT).mp4"
                hold_duration=$(echo "scale=6; $hold_frames / $VIDEO_FPS" | bc -l 2>/dev/null)
                if [[ "$hold_duration" =~ ^\..* ]]; then
                    hold_duration="0$hold_duration"
                fi
                
                ffmpeg -y -loop 1 -i "$LAST_CHIP_IMAGE" \
                    -frames:v "$hold_frames" \
                    -vf "scale=iw:ih:flags=lanczos" \
                    -r "$VIDEO_FPS" \
                    -c:v libx264 \
                    -threads 0 \
                    -crf "$CRF" \
                    -pix_fmt yuv420p \
                    -an \
                    "$hold_segment" \
                    -loglevel error 2>"$TEMP_DIR/ffmpeg_error_hold.txt"
                
                if [ -f "$hold_segment" ] && [ -s "$hold_segment" ]; then
                    echo "file '$hold_segment'" >> "$SEGMENT_LIST"
                    echo "$SEGMENT_COUNT|hold|$LAST_CHIP_YEAR|hold|hold|${hold_duration}|$hold_frames" >> "$SEGMENT_DEBUG_LOG"
                    ((SEGMENT_COUNT++))
                    echo "  Created hold segment: ${hold_duration}s"
                fi
                
                # Create 0.5s dissolve segment from last chip to first photo
                dissolve_segment="$SEGMENTS_DIR/transition_dissolve_$(printf "%06d" $SEGMENT_COUNT).mp4"
                dissolve_duration=$(echo "scale=6; $dissolve_frames / $VIDEO_FPS" | bc -l 2>/dev/null)
                if [[ "$dissolve_duration" =~ ^\..* ]]; then
                    dissolve_duration="0$dissolve_duration"
                fi
                
                # Create dissolve using xfade filter
                # Both inputs need same duration, fade starts at offset
                ffmpeg -y \
                    -loop 1 -t "$dissolve_duration" -i "$LAST_CHIP_IMAGE" \
                    -loop 1 -t "$dissolve_duration" -i "$first_photo_file" \
                    -filter_complex "[0:v]scale=${VIDEO_WIDTH}:${VIDEO_HEIGHT}:flags=lanczos,setpts=PTS-STARTPTS[v0];[1:v]scale=${VIDEO_WIDTH}:${VIDEO_HEIGHT}:flags=lanczos,setpts=PTS-STARTPTS[v1];[v0][v1]xfade=transition=fade:duration=${dissolve_duration}:offset=0[v]" \
                    -map "[v]" \
                    -frames:v "$dissolve_frames" \
                    -r "$VIDEO_FPS" \
                    -c:v libx264 \
                    -threads 0 \
                    -crf "$CRF" \
                    -pix_fmt yuv420p \
                    -an \
                    "$dissolve_segment" \
                    -loglevel error 2>"$TEMP_DIR/ffmpeg_error_dissolve.txt"
                
                if [ -f "$dissolve_segment" ] && [ -s "$dissolve_segment" ]; then
                    echo "file '$dissolve_segment'" >> "$SEGMENT_LIST"
                    echo "$SEGMENT_COUNT|dissolve|$LAST_CHIP_YEAR|dissolve|dissolve|${dissolve_duration}|$dissolve_frames" >> "$SEGMENT_DEBUG_LOG"
                    ((SEGMENT_COUNT++))
                    echo "  Created dissolve segment: ${dissolve_duration}s"
                fi
            fi
            
            TRANSITION_CREATED=true
        fi
        
        # For photos, accumulate beat intervals until we have at least 4 beats
        # Calculate which photo/segment to use (based on accumulated intervals, not current one)
        segments_after_chips=$((chip_num - TOTAL_SELECTED))
        
        # Track which photo and segment we're on (based on accumulated photo segments created, not beat intervals)
        # We need a separate counter for photo segments (not beat intervals)
        if [ -z "${PHOTO_SEGMENT_NUM:-}" ]; then
            PHOTO_SEGMENT_NUM=0
            # If transition was created, dissolve already showed first photo seg_0, so start from seg_1
            if [ "$TRANSITION_CREATED" = true ]; then
                first_photo_seg_count=${PHOTO_SEGMENT_COUNT[1]:-1}
                if [ "$first_photo_seg_count" -gt 1 ]; then
                    # Start accumulating for seg_1
                    PHOTO_CURRENT_IDX=1
                    PHOTO_CURRENT_SEG=1
                else
                    # Move to next photo
                    PHOTO_CURRENT_IDX=2
                    PHOTO_CURRENT_SEG=0
                fi
            else
                PHOTO_CURRENT_IDX=1
                PHOTO_CURRENT_SEG=0
            fi
            PHOTO_CURRENT_IMAGE=""
            PHOTO_CURRENT_NAME=""
        fi
        
        # Initialize accumulation if starting new photo segment
        if [ "$PHOTO_ACCUMULATING" = false ]; then
            PHOTO_ACCUMULATING=true
            PHOTO_ACCUM_DURATION=0
            PHOTO_ACCUM_FRAMES=0
            PHOTO_ACCUM_START_TIME="$start_time"
            PHOTO_ACCUM_BEAT_COUNT=0
            
            # Get current photo/segment image
            if [ $PHOTO_CURRENT_IDX -le "$TOTAL_PHOTOS" ]; then
                seg_count=${PHOTO_SEGMENT_COUNT[$PHOTO_CURRENT_IDX]:-1}
                if [ "$seg_count" -gt 0 ] && [ $PHOTO_CURRENT_SEG -lt "$seg_count" ]; then
                    PHOTO_CURRENT_IMAGE="$PHOTOS_PROCESSED_DIR/photo_${PHOTO_CURRENT_IDX}_seg_${PHOTO_CURRENT_SEG}.png"
                    original_photo=$(sed -n "${PHOTO_CURRENT_IDX}p" "$PHOTOS_LIST")
                    PHOTO_CURRENT_NAME=$(basename "$original_photo")
                    if [ "$seg_count" -gt 1 ]; then
                        PHOTO_CURRENT_NAME="${PHOTO_CURRENT_NAME} (seg $((PHOTO_CURRENT_SEG + 1))/$seg_count)"
                    fi
                else
                    # Move to next photo
                    ((PHOTO_CURRENT_IDX++))
                    PHOTO_CURRENT_SEG=0
                    if [ $PHOTO_CURRENT_IDX -le "$TOTAL_PHOTOS" ]; then
                        seg_count=${PHOTO_SEGMENT_COUNT[$PHOTO_CURRENT_IDX]:-1}
                        if [ "$seg_count" -gt 0 ]; then
                            PHOTO_CURRENT_IMAGE="$PHOTOS_PROCESSED_DIR/photo_${PHOTO_CURRENT_IDX}_seg_${PHOTO_CURRENT_SEG}.png"
                            original_photo=$(sed -n "${PHOTO_CURRENT_IDX}p" "$PHOTOS_LIST")
                            PHOTO_CURRENT_NAME=$(basename "$original_photo")
                            if [ "$seg_count" -gt 1 ]; then
                                PHOTO_CURRENT_NAME="${PHOTO_CURRENT_NAME} (seg $((PHOTO_CURRENT_SEG + 1))/$seg_count)"
                            fi
                        fi
                    else
                        # Exhausted all photos, freeze on last segment of last photo
                        PHOTO_CURRENT_IDX=$TOTAL_PHOTOS
                        last_seg_count=${PHOTO_SEGMENT_COUNT[$PHOTO_CURRENT_IDX]:-1}
                        if [ "$last_seg_count" -gt 0 ]; then
                            PHOTO_CURRENT_SEG=$((last_seg_count - 1))
                            PHOTO_CURRENT_IMAGE="$PHOTOS_PROCESSED_DIR/photo_${PHOTO_CURRENT_IDX}_seg_${PHOTO_CURRENT_SEG}.png"
                            original_photo=$(sed -n "${PHOTO_CURRENT_IDX}p" "$PHOTOS_LIST")
                            PHOTO_CURRENT_NAME=$(basename "$original_photo")
                            if [ "$last_seg_count" -gt 1 ]; then
                                PHOTO_CURRENT_NAME="${PHOTO_CURRENT_NAME} (seg $((PHOTO_CURRENT_SEG + 1))/$last_seg_count)"
                            fi
                        fi
                    fi
                fi
            else
                # Already exhausted, use last photo
                PHOTO_CURRENT_IDX=$TOTAL_PHOTOS
                last_seg_count=${PHOTO_SEGMENT_COUNT[$PHOTO_CURRENT_IDX]:-1}
                if [ "$last_seg_count" -gt 0 ]; then
                    PHOTO_CURRENT_SEG=$((last_seg_count - 1))
                    PHOTO_CURRENT_IMAGE="$PHOTOS_PROCESSED_DIR/photo_${PHOTO_CURRENT_IDX}_seg_${PHOTO_CURRENT_SEG}.png"
                    original_photo=$(sed -n "${PHOTO_CURRENT_IDX}p" "$PHOTOS_LIST")
                    PHOTO_CURRENT_NAME=$(basename "$original_photo")
                    if [ "$last_seg_count" -gt 1 ]; then
                        PHOTO_CURRENT_NAME="${PHOTO_CURRENT_NAME} (seg $((PHOTO_CURRENT_SEG + 1))/$last_seg_count)"
                    fi
                fi
            fi
        fi
        
        # Accumulate this beat interval
        PHOTO_ACCUM_DURATION=$(echo "scale=6; $PHOTO_ACCUM_DURATION + $duration" | bc -l 2>/dev/null || echo "$PHOTO_ACCUM_DURATION")
        PHOTO_ACCUM_FRAMES=$((PHOTO_ACCUM_FRAMES + frame_count))
        PHOTO_ACCUM_BEAT_COUNT=$((PHOTO_ACCUM_BEAT_COUNT + 1))
        
        # Check if we have at least 4 beats
        if [ $PHOTO_ACCUM_BEAT_COUNT -ge 4 ] && [ -n "$PHOTO_CURRENT_IMAGE" ] && [ -f "$PHOTO_CURRENT_IMAGE" ]; then
            # Create photo segment with accumulated duration
            PHOTO_SEGMENT_READY=true
            frame_file="$PHOTO_CURRENT_IMAGE"
            chip_year="Photo"
            chip_name="$PHOTO_CURRENT_NAME"
            actual_duration="$PHOTO_ACCUM_DURATION"
            frame_count="$PHOTO_ACCUM_FRAMES"
            
            # Reset accumulation and move to next photo/segment
            PHOTO_ACCUMULATING=false
            PHOTO_SEGMENT_NUM=$((PHOTO_SEGMENT_NUM + 1))
            
            # Move to next segment/photo
            seg_count=${PHOTO_SEGMENT_COUNT[$PHOTO_CURRENT_IDX]:-1}
            if [ "$seg_count" -gt 0 ] && [ $((PHOTO_CURRENT_SEG + 1)) -lt "$seg_count" ]; then
                # Next segment of same photo
                ((PHOTO_CURRENT_SEG++))
            else
                # Move to next photo
                ((PHOTO_CURRENT_IDX++))
                PHOTO_CURRENT_SEG=0
                if [ $PHOTO_CURRENT_IDX -gt "$TOTAL_PHOTOS" ]; then
                    # Freeze on last segment of last photo
                    PHOTO_CURRENT_IDX=$TOTAL_PHOTOS
                    last_seg_count=${PHOTO_SEGMENT_COUNT[$PHOTO_CURRENT_IDX]:-1}
                    if [ "$last_seg_count" -gt 0 ]; then
                        PHOTO_CURRENT_SEG=$((last_seg_count - 1))
                    else
                        PHOTO_CURRENT_SEG=0
                    fi
                fi
            fi
        else
            # Not enough beats yet, skip creating segment this iteration
            continue
        fi
    elif [ "$TOTAL_SELECTED" -gt 0 ]; then
        # No photos available, freeze last chip
        chip_num="$TOTAL_SELECTED"
        chip_info=$(awk -F'|' -v idx="$chip_num" '$1 == idx {print $0; exit}' "$CHIP_YEAR_MAP")
        if [ -z "$chip_info" ]; then
            echo "WARNING: No chip info found for chip_num=$chip_num" >> "$SEGMENT_DEBUG_LOG"
            continue
        fi
        
        frame_file=$(echo "$chip_info" | cut -d'|' -f7)
        chip_year=$(echo "$chip_info" | cut -d'|' -f2)
        chip_name=$(echo "$chip_info" | cut -d'|' -f4)
    else
        echo "ERROR: No chips or photos available for segment $SEGMENT_COUNT" >&2
        continue
    fi
    
    if [ ! -f "$frame_file" ]; then
        if [ "$using_photo" = true ]; then
            echo "WARNING: Photo not found: $frame_file" >> "$SEGMENT_DEBUG_LOG"
        else
            echo "WARNING: Chip $chip_num image not found: $frame_file" >> "$SEGMENT_DEBUG_LOG"
        fi
        continue
    fi
    
    segment_file="$SEGMENTS_DIR/segment_$(printf "%06d" $SEGMENT_COUNT).mp4"
    
    # Use frame_count for exact frame quantization (prevents accumulated errors)
    # For photos with accumulated beats, use the accumulated duration; otherwise calculate from frame_count
    if [ "$using_photo" = true ] && [ "$PHOTO_SEGMENT_READY" = true ]; then
        # Use accumulated duration for photos (already set above)
        # Recalculate from frame_count to ensure frame alignment
        actual_duration=$(echo "scale=6; $frame_count / $VIDEO_FPS" | bc -l 2>/dev/null)
        PHOTO_SEGMENT_READY=false
    else
        # Calculate duration from frame_count to ensure exact frame alignment
        actual_duration=$(echo "scale=6; $frame_count / $VIDEO_FPS" | bc -l 2>/dev/null)
    fi
    # Ensure minimum is at least one frame
    if [ "$frame_count" -lt 1 ]; then
        frame_count=1
        actual_duration=$(echo "scale=6; 1 / $VIDEO_FPS" | bc -l 2>/dev/null || echo "0.016667")
    fi
    
    # Ensure duration has leading zero if needed (ffmpeg requires 0.123 not .123)
    if [[ "$actual_duration" =~ ^\..* ]]; then
        actual_duration="0$actual_duration"
    fi
    
    # Create segment with exact frame count using -frames:v instead of -t
    # This ensures perfect frame quantization without accumulation errors
    ffmpeg -y -loop 1 -i "$frame_file" \
        -frames:v "$frame_count" \
        -vf "scale=iw:ih:flags=lanczos" \
        -r "$VIDEO_FPS" \
        -c:v libx264 \
        -threads 0 \
        -crf "$CRF" \
        -pix_fmt yuv420p \
        -an \
        "$segment_file" \
        -loglevel error 2>"$TEMP_DIR/ffmpeg_error_${SEGMENT_COUNT}.txt"
    
    # Check if segment was created successfully
    if [ ! -f "$segment_file" ] || [ ! -s "$segment_file" ]; then
        if [ "$SEGMENT_COUNT" -lt 5 ]; then
            if [ "$using_photo" = true ]; then
                echo "WARNING: Failed to create segment $SEGMENT_COUNT (photo ${chip_name}, duration=$actual_duration)" >&2
            else
                echo "WARNING: Failed to create segment $SEGMENT_COUNT (chip $chip_num, duration=$actual_duration)" >&2
            fi
            echo "  Frame file: $frame_file" >&2
            echo "  Segment file: $segment_file" >&2
            if [ -f "$TEMP_DIR/ffmpeg_error_${SEGMENT_COUNT}.txt" ]; then
                echo "  FFmpeg error:" >&2
                cat "$TEMP_DIR/ffmpeg_error_${SEGMENT_COUNT}.txt" >&2
            fi
        fi
        continue
    fi
    
    # Segment created successfully - add to list
    echo "file '$segment_file'" >> "$SEGMENT_LIST"
    if [ "$using_photo" = true ]; then
        echo "$SEGMENT_COUNT|photo|$chip_year|$start_time|$end_time|$actual_duration|$frame_count|$chip_name" >> "$SEGMENT_DEBUG_LOG"
    else
        echo "$SEGMENT_COUNT|$chip_num|$chip_year|$start_time|$end_time|$actual_duration|$frame_count" >> "$SEGMENT_DEBUG_LOG"
    fi
    
    if [ -z "$FIRST_SEGMENT_YEAR" ]; then
        FIRST_SEGMENT_YEAR="$chip_year"
    fi
    LAST_SEGMENT_YEAR="$chip_year"
    
    # Log first 10 and every 50th segments
    if [ "$SEGMENT_COUNT" -lt 10 ] || [ $((SEGMENT_COUNT % 50)) -eq 0 ]; then
        start_formatted=$(printf "%.3f" "$start_time" 2>/dev/null || echo "$start_time")
        end_formatted=$(printf "%.3f" "$end_time" 2>/dev/null || echo "$end_time")
        duration_formatted=$(printf "%.3f" "$actual_duration" 2>/dev/null || echo "$actual_duration")
        if [ "$using_photo" = true ]; then
            echo "  Segment $SEGMENT_COUNT: photo ${chip_name}, ${start_formatted}s-${end_formatted}s (${frame_count} frames, ${duration_formatted}s)"
        else
            echo "  Segment $SEGMENT_COUNT: chip $chip_num (${chip_name}, year: $chip_year), ${start_formatted}s-${end_formatted}s (${frame_count} frames, ${duration_formatted}s)"
        fi
    fi
    
    ((SEGMENT_COUNT++))
done
exec 3<&-

echo ""
echo "Created $SEGMENT_COUNT video segments from beat intervals"
echo "DEBUG: Segment summary:"
echo "  First segment year: $FIRST_SEGMENT_YEAR"
echo "  Last segment year: $LAST_SEGMENT_YEAR"
echo "  Total segments: $SEGMENT_COUNT"
echo ""

# Step 7: Concatenate segments
echo "Step 7: Concatenating video segments..."
VIDEO_NO_AUDIO="$TEMP_DIR/video_no_audio.mp4"

ffmpeg -y -f concat -safe 0 -i "$SEGMENT_LIST" \
    -c copy \
    "$VIDEO_NO_AUDIO" \
    -loglevel error

if [ ! -f "$VIDEO_NO_AUDIO" ] || [ ! -s "$VIDEO_NO_AUDIO" ]; then
    echo "Error: Failed to concatenate video segments"
    exit 1
fi

echo "Video segments concatenated"
echo ""

# Step 8: Add audio with fade out (skip fade if using entire audio)
if [ "${ORIGINAL_VIDEO_DURATION:-}" = "0" ]; then
    echo "Step 8: Adding audio (no fade - using entire audio)..."
    ffmpeg -y -i "$VIDEO_NO_AUDIO" -i "$EXTRACTED_AUDIO" \
        -c:v copy \
        -c:a aac \
        -threads 0 \
        -b:a 192k \
        -movflags +faststart \
        -fflags +genpts \
        "$OUTPUT_FILE" \
        -loglevel warning
else
    echo "Step 8: Adding audio with fade out..."
    FADE_START=$(echo "$VIDEO_DURATION - 1.0" | bc -l)
    
    ffmpeg -y -i "$VIDEO_NO_AUDIO" -i "$EXTRACTED_AUDIO" \
        -af "afade=t=out:st=${FADE_START}:d=1.0" \
        -c:v copy \
        -c:a aac \
        -threads 0 \
        -b:a 192k \
        -movflags +faststart \
        -fflags +genpts \
        "$OUTPUT_FILE" \
        -loglevel warning
fi

if [ -f "$OUTPUT_FILE" ] && [ -s "$OUTPUT_FILE" ]; then
    if file "$OUTPUT_FILE" | grep -q "MP4\|ISO Media"; then
        echo ""
        if [ "$INTERRUPTED" = true ]; then
            echo "Interrupted but valid MP4 created: $OUTPUT_FILE"
        else
            echo "Successfully created: $OUTPUT_FILE"
        fi
        echo "File size: $(du -h "$OUTPUT_FILE" | cut -f1)"
        
        if command -v ffprobe >/dev/null 2>&1; then
            DURATION=$(ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 "$OUTPUT_FILE" 2>/dev/null | cut -d. -f1)
            if [ -n "$DURATION" ] && [ "$DURATION" != "N/A" ]; then
                echo "Duration: ${DURATION} seconds"
            fi
        fi
        
        OUTPUT_ABS=$(realpath "$OUTPUT_FILE")
        echo ""
        echo "Output file (absolute path):"
        echo "$OUTPUT_ABS"
        
        # Save timing data for visualization (before cleanup)
        TIMING_DATA="${OUTPUT_FILE%.mp4}_timing_data.txt"
        if [ -f "$SEGMENT_DEBUG_LOG" ] && [ -f "$BEATS_FILE" ]; then
            echo "# Timing data for visualization" > "$TIMING_DATA"
            echo "# Format: segment_index|chip_num|chip_year|start_time|end_time|duration|frame_count" >> "$TIMING_DATA"
            cat "$SEGMENT_DEBUG_LOG" >> "$TIMING_DATA"
            echo "" >> "$TIMING_DATA"
            echo "# Beat times" >> "$TIMING_DATA"
            cat "$BEATS_FILE" >> "$TIMING_DATA"
            echo "Timing data saved to: $TIMING_DATA"
        fi
    else
        echo ""
        echo "Error: Output file exists but is not a valid MP4"
        rm -f "$OUTPUT_FILE"
        exit 1
    fi
elif [ "$INTERRUPTED" = false ]; then
    echo ""
    echo "Error: Failed to create video file"
    exit 1
fi
