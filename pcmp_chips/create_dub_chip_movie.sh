#!/bin/bash

# Script to create MP4 movie from chip layout thumbnails synchronized to dub music beats
# Chips change exactly at each beat, synchronized with audio
# Audio fades out over 1 second at the end
#
# Usage: ./create_dub_chip_movie.sh [output_file] [audio_segment_start] [duration]
# Examples:
#   ./create_dub_chip_movie.sh dub_chips.mp4
#   ./create_dub_chip_movie.sh dub_chips.mp4 0 45  # Start at 0s, 45s duration
#   FILE_LIMIT=100 ./create_dub_chip_movie.sh test.mp4  # Test with 100 images

CHIP_DIR="${CHIP_DIR:-$HOME/pcmp_home}"
CSV_DATABASE="${CSV_DATABASE:-$CHIP_DIR/chip_database.csv}"
IMAGE_LIST="${IMAGE_LIST:-$CHIP_DIR/layout_images.txt}"
AUDIO_FILE="${AUDIO_FILE:-$CHIP_DIR/04 Reaching Dub.m4a}"
OUTPUT_FILE="${1:-${OUTPUT_FILE:-$HOME/pcmp_home/pcmp_chips_dub.mp4}}"
AUDIO_START="${2:-0}"  # Start time in audio (seconds)
VIDEO_DURATION="${3:-0}"  # Target video duration (seconds)
VIDEO_FPS="${VIDEO_FPS:-60}"  # Video frame rate (fps) - MP4 container frame rate
CRF="${CRF:-23}" # Compression quality (0-51, 0 is best quality)
FILE_LIMIT="${FILE_LIMIT:-0}"  # 0 = no limit, otherwise stop after N files
BEAT_THRESHOLD="${BEAT_THRESHOLD:-0.6}"  # Beat detection threshold for aubioonset (0.0-1.0, lower = more sensitive)
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

if [ ! -f "$AUDIO_FILE" ]; then
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

echo "Creating dub-synchronized chip layout movie"
echo "==========================================="
echo "CSV Database: $CSV_DATABASE"
echo "Image list: $IMAGE_LIST"
echo "Audio file: $AUDIO_FILE"
echo "Audio start: ${AUDIO_START}s"
echo "Video duration: ${VIDEO_DURATION}s"
echo "Video frame rate: ${VIDEO_FPS} fps (MP4 container rate)"
echo "Beat detection threshold: ${BEAT_THRESHOLD} (lower = more sensitive, more beats)"
echo "Timing: One chip per beat interval (no minimum dwell time)"
echo "Output file: $OUTPUT_FILE"
echo "Compression (CRF): $CRF"
if [ "$FILE_LIMIT" -gt 0 ]; then
    echo "File limit: $FILE_LIMIT files (test mode)"
else
    echo "File limit: unlimited"
fi
echo ""

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

# Run aubioonset and capture both output and errors for debugging
aubioonset -i "$EXTRACTED_AUDIO" -t "$BEAT_THRESHOLD" > "$BEATS_FILE" 2>"$TEMP_DIR/aubioonset_errors.txt"

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
    # Update REQUIRED_CHIPS to match available chips (will reuse if needed)
    if [ "$TOTAL_AVAILABLE" -lt "$REQUIRED_CHIPS" ]; then
        echo "  Note: Have $TOTAL_AVAILABLE chips but $REQUIRED_CHIPS beats - will reuse chips"
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
    echo "Will reuse chips if necessary"
    REQUIRED_CHIPS="$TOTAL_SELECTED"
fi

# Ensure SEGMENTS_DIR exists
mkdir -p "$SEGMENTS_DIR"

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
    # Calculate which chip to use (chip index starts at 1)
    chip_num=$((SEGMENT_COUNT + 1))
    if [ "$TOTAL_SELECTED" -gt 0 ] && [ "$chip_num" -gt "$TOTAL_SELECTED" ]; then
        # Reuse chips if we run out
        chip_num=$((((chip_num - 1) % TOTAL_SELECTED) + 1))
    elif [ "$TOTAL_SELECTED" -eq 0 ]; then
        echo "ERROR: No chips available for segment $SEGMENT_COUNT" >&2
        continue
    fi
    
    # Get chip info including cached image path
    chip_info=$(awk -F'|' -v idx="$chip_num" '$1 == idx {print $0; exit}' "$CHIP_YEAR_MAP")
    if [ -z "$chip_info" ]; then
        echo "WARNING: No chip info found for chip_num=$chip_num" >> "$SEGMENT_DEBUG_LOG"
        continue
    fi
    
    frame_file=$(echo "$chip_info" | cut -d'|' -f7)
    chip_year=$(echo "$chip_info" | cut -d'|' -f2)
    chip_name=$(echo "$chip_info" | cut -d'|' -f4)
    
    if [ ! -f "$frame_file" ]; then
        echo "WARNING: Chip $chip_num image not found: $frame_file" >> "$SEGMENT_DEBUG_LOG"
        continue
    fi
    
    segment_file="$SEGMENTS_DIR/segment_$(printf "%06d" $SEGMENT_COUNT).mp4"
    
    # Use frame_count for exact frame quantization (prevents accumulated errors)
    # Calculate duration from frame_count to ensure exact frame alignment
    actual_duration=$(echo "scale=6; $frame_count / $VIDEO_FPS" | bc -l 2>/dev/null)
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
            echo "WARNING: Failed to create segment $SEGMENT_COUNT (chip $chip_num, duration=$actual_duration)" >&2
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
    echo "$SEGMENT_COUNT|$chip_num|$chip_year|$start_time|$end_time|$actual_duration|$frame_count" >> "$SEGMENT_DEBUG_LOG"
    
    if [ -z "$FIRST_SEGMENT_YEAR" ]; then
        FIRST_SEGMENT_YEAR="$chip_year"
    fi
    LAST_SEGMENT_YEAR="$chip_year"
    
    # Log first 10 and every 50th segments
    if [ "$SEGMENT_COUNT" -lt 10 ] || [ $((SEGMENT_COUNT % 50)) -eq 0 ]; then
        start_formatted=$(printf "%.3f" "$start_time" 2>/dev/null || echo "$start_time")
        end_formatted=$(printf "%.3f" "$end_time" 2>/dev/null || echo "$end_time")
        duration_formatted=$(printf "%.3f" "$actual_duration" 2>/dev/null || echo "$actual_duration")
        echo "  Segment $SEGMENT_COUNT: chip $chip_num (${chip_name}, year: $chip_year), ${start_formatted}s-${end_formatted}s (${frame_count} frames, ${duration_formatted}s)"
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

# Step 8: Add audio with fade out
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
