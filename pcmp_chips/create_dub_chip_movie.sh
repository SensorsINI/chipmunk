#!/bin/bash

# Script to create MP4 movie from chip layout thumbnails synchronized to dub music beats
# Frame rate varies dynamically with the beat, with pauses at certain points
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
VIDEO_DURATION="${3:-45}"  # Target video duration (seconds)
VIDEO_FPS="${VIDEO_FPS:-60}"  # Video frame rate (fps) - MP4 container frame rate
CRF="${CRF:-23}"
FILE_LIMIT="${FILE_LIMIT:-0}"  # 0 = no limit, otherwise stop after N files

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
ANNOTATED_DIR="$TEMP_DIR/annotated"
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
    rm -rf "$TEMP_DIR"
}

# Trap signals
trap 'INTERRUPTED=true; cleanup; exit 130' INT TERM
trap 'cleanup' EXIT

echo "Creating dub-synchronized chip layout movie"
echo "==========================================="
echo "CSV Database: $CSV_DATABASE"
echo "Image list: $IMAGE_LIST"
echo "Audio file: $AUDIO_FILE"
echo "Audio start: ${AUDIO_START}s"
echo "Video duration: ${VIDEO_DURATION}s"
echo "Video frame rate: ${VIDEO_FPS} fps (MP4 container rate)"
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

# Step 1: Extract audio segment and detect beats
echo "Step 1: Extracting audio segment and detecting beats..."
AUDIO_END=$((AUDIO_START + VIDEO_DURATION))
EXTRACTED_AUDIO="$TEMP_DIR/audio_segment.m4a"

# Extract audio segment
ffmpeg -y -i "$AUDIO_FILE" -ss "$AUDIO_START" -t "$VIDEO_DURATION" -c copy "$EXTRACTED_AUDIO" -loglevel error

if [ ! -f "$EXTRACTED_AUDIO" ] || [ ! -s "$EXTRACTED_AUDIO" ]; then
    echo "Error: Failed to extract audio segment"
    exit 1
fi

# Detect beats using aubio
BEATS_FILE="$TEMP_DIR/beats.txt"
echo "Detecting beats in audio..."
# Use default onset detection (hfc - High Frequency Content)
# -t 0.3 is threshold (lower = more detections, higher = fewer)
aubioonset -i "$EXTRACTED_AUDIO" -t 0.3 > "$BEATS_FILE" 2>/dev/null

if [ ! -s "$BEATS_FILE" ]; then
    echo "Warning: No beats detected, using uniform timing"
    # Create uniform beats every 0.5 seconds as fallback
    seq 0 0.5 "$VIDEO_DURATION" > "$BEATS_FILE"
fi

BEAT_COUNT=$(wc -l < "$BEATS_FILE")
echo "Detected $BEAT_COUNT beats"
echo ""

# Step 2: Build sorted image list (same as original script)
echo "Step 2: Building sorted image list..."
SORTED_LIST="$TEMP_DIR/sorted_images.txt"
> "$SORTED_LIST"

declare -A csv_normalized_date
declare -A csv_username
declare -A csv_pname

while IFS='|' read -r pname normalized_date username; do
    if [ -n "$pname" ]; then
        csv_normalized_date["$pname"]="$normalized_date"
        csv_username["$pname"]="$username"
        csv_pname["$pname"]="$pname"
    fi
done < <(python3 -c "
import csv
import sys

with open('$CSV_DATABASE', 'r', encoding='utf-8') as f:
    reader = csv.reader(f)
    next(reader)
    for row in reader:
        if len(row) >= 16:
            pname = row[3].strip()
            username = row[2].strip()
            normalized_date = row[14].strip()
            if pname:
                print(f'{pname}|{normalized_date}|{username}')
")

TOTAL=0
PROCESSED=0
NO_DATE=0
MISSING=0

while IFS= read -r full_image; do
    ((TOTAL++))
    
    if [ "$FILE_LIMIT" -gt 0 ] && [ "$PROCESSED" -ge "$FILE_LIMIT" ]; then
        break
    fi
    
    thumbnail="${full_image/_layout.png/_layout_thumbnail.png}"
    
    if [ ! -f "$thumbnail" ]; then
        ((MISSING++))
        continue
    fi
    
    basename=$(basename "$full_image" "_layout.png")
    pname="$basename"
    
    normalized_date="${csv_normalized_date[$pname]:-}"
    username="${csv_username[$pname]:-}"
    
    if [ -z "$normalized_date" ]; then
        ((NO_DATE++))
        normalized_date="9999-99-99"
    fi
    
    year=$(extract_year "$normalized_date")
    if [ -z "$year" ] || [ "$year" = "9999" ]; then
        year="?"
    fi
    
    chip_name=$(extract_chip_name "$pname")
    sort_date="$normalized_date"
    
    echo "$sort_date|$year|$chip_name|$username|$pname|$thumbnail" >> "$SORTED_LIST"
    
    ((PROCESSED++))
done < "$IMAGE_LIST"

sort -t'|' -k1,1 "$SORTED_LIST" > "$SORTED_LIST.sorted"

TOTAL_IMAGES=$(wc -l < "$SORTED_LIST.sorted")
echo "Found $TOTAL images, processed $PROCESSED, $MISSING missing thumbnails, $NO_DATE with no date"
echo ""

# Step 3: Create annotated images (same as original)
echo "Step 3: Creating annotated images..."
rm -f "$ANNOTATED_DIR"/*.png
COUNT=0

while IFS='|' read -r sort_date year chip_name username pname thumbnail; do
    ((COUNT++))
    numbered_name=$(printf "%06d.png" $COUNT)
    annotated_image="$ANNOTATED_DIR/$numbered_name"
    overlay_title_block "$thumbnail" "$annotated_image" "$year" "$chip_name" "$username"
    
    if [ $((COUNT % 50)) -eq 0 ]; then
        echo "Annotated $COUNT/$TOTAL_IMAGES images..."
    fi
done < "$SORTED_LIST.sorted"

echo "Created $COUNT annotated images"
echo ""

# Step 4: Generate timing map from beats
echo "Step 4: Generating frame timing map from beats..."
TIMING_MAP="$TEMP_DIR/timing_map.txt"
> "$TIMING_MAP"

# Read beats and create timing map
# Strategy: Assign frames to beat intervals
# - Fast transition on beats (0.1-0.2s per frame)
# - Slower between beats (0.3-0.5s per frame)
# - Pause every 4th beat (0.8-1.2s)

PYTHON_SCRIPT="$TEMP_DIR/generate_timing.py"
cat > "$PYTHON_SCRIPT" << 'PYTHON_EOF'
import sys

if len(sys.argv) < 5:
    print("Error: Not enough arguments", file=sys.stderr)
    sys.exit(1)

beats_file = sys.argv[1]
timing_map_file = sys.argv[2]
total_images = int(sys.argv[3])
video_duration = float(sys.argv[4])
PYTHON_EOF

cat >> "$PYTHON_SCRIPT" << 'PYTHON_EOF'

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
    print(f"Warning: Could not read beats file: {e}", file=sys.stderr)
    beats = []

# If no beats, create uniform timing
if not beats:
    interval = video_duration / total_images
    with open(timing_map_file, 'w') as f:
        for i in range(total_images):
            frame_time = i * interval
            duration = interval
            f.write(f"{i}|{duration}|0\n")
    sys.exit(0)

# Add end time
beats.append(video_duration)

# Assign frames to beat intervals
# Strategy: Ensure all images are used by distributing them across intervals
# Faster transitions on beats, slower between, with pauses every 4th beat

frame_idx = 0
pause_counter = 0
total_intervals = len(beats) - 1

# Calculate target frames per interval to use all images
# Account for pauses (every 4th beat doesn't advance frame)
normal_intervals = total_intervals - (total_intervals // 4)  # Exclude pause intervals
if normal_intervals > 0:
    target_frames_per_interval = total_images / normal_intervals
else:
    target_frames_per_interval = 1

with open(timing_map_file, 'w') as f:
    for i in range(total_intervals):
        beat_start = beats[i]
        beat_end = beats[i + 1]
        interval_duration = beat_end - beat_start
        
        # Determine if this is a pause (every 4th beat)
        is_pause = (pause_counter % 4 == 3)
        pause_counter += 1
        
        if is_pause:
            # Pause: show same frame for longer (don't advance frame_idx)
            if frame_idx < total_images:
                pause_duration = min(1.2, interval_duration * 1.5)
                f.write(f"{frame_idx}|{pause_duration}|1\n")
        else:
            # Normal: transition frames, ensuring we use all images
            # Count how many normal intervals remain (excluding pauses)
            remaining_normal_intervals = 0
            for j in range(i + 1, total_intervals):
                if (j % 4) != 3:  # Not a pause
                    remaining_normal_intervals += 1
            
            remaining_frames = total_images - frame_idx
            
            if remaining_normal_intervals > 0 and remaining_frames > 0:
                # Calculate frames needed to use all remaining images
                frames_needed = max(1, int(remaining_frames / remaining_normal_intervals))
                # But also consider interval duration (don't go too fast - max ~6 fps)
                duration_based_max = max(1, int(interval_duration * 6))
                frames_in_interval = min(frames_needed, duration_based_max, remaining_frames)
            else:
                # Last interval or no remaining: use all remaining frames
                frames_in_interval = max(1, remaining_frames)
            
            frames_in_interval = min(frames_in_interval, total_images - frame_idx)
            
            if frames_in_interval > 0:
                frame_duration = interval_duration / frames_in_interval
                # Faster on beat (first frame), slower after
                for j in range(frames_in_interval):
                    if frame_idx >= total_images:
                        break
                    if j == 0:
                        # Beat frame: faster
                        duration = min(0.15, frame_duration * 0.6)
                    else:
                        # Between beats: slower
                        duration = max(0.3, frame_duration * 1.2)
                    f.write(f"{frame_idx}|{duration}|0\n")
                    frame_idx += 1

# Fill remaining frames if any
remaining = total_images - frame_idx
if remaining > 0:
    avg_duration = (video_duration - sum(float(l.split('|')[1]) for l in open(timing_map_file).readlines())) / remaining
    with open(timing_map_file, 'a') as f:
        for i in range(remaining):
            f.write(f"{frame_idx}|{avg_duration}|0\n")
            frame_idx += 1
PYTHON_EOF

python3 "$PYTHON_SCRIPT" "$BEATS_FILE" "$TIMING_MAP" "$COUNT" "$VIDEO_DURATION" 2>&1

echo "Generated timing map for $COUNT frames"
echo ""

# Step 5: Create video segments with variable timing
echo "Step 5: Creating video segments..."
SEGMENT_LIST="$TEMP_DIR/segments.txt"
> "$SEGMENT_LIST"

SEGMENT_COUNT=0
while IFS='|' read -r frame_idx duration is_pause; do
    # Convert frame_idx to integer (bash arithmetic doesn't handle floats)
    frame_idx_int=$(printf "%.0f" "$frame_idx" 2>/dev/null || echo "$frame_idx" | cut -d'.' -f1)
    frame_file="$ANNOTATED_DIR/$(printf "%06d.png" $((frame_idx_int + 1)))"
    
    if [ ! -f "$frame_file" ]; then
        continue
    fi
    
    segment_file="$SEGMENTS_DIR/segment_$(printf "%06d" $SEGMENT_COUNT).mp4"
    
    # Create segment: single frame displayed for 'duration' seconds
    # Note: MP4 has fixed frame rate, but we simulate variable rate by varying frame duration
    # Each segment shows one frame for 'duration' seconds at the specified frame rate
    ffmpeg -y -loop 1 -i "$frame_file" \
        -t "$duration" \
        -vf "scale=iw:ih:flags=lanczos" \
        -r "$VIDEO_FPS" \
        -c:v libx264 \
        -crf "$CRF" \
        -pix_fmt yuv420p \
        -an \
        "$segment_file" \
        -loglevel error
    
    if [ -f "$segment_file" ] && [ -s "$segment_file" ]; then
        echo "file '$segment_file'" >> "$SEGMENT_LIST"
        ((SEGMENT_COUNT++))
    fi
done < "$TIMING_MAP"

echo "Created $SEGMENT_COUNT video segments"
echo ""

# Step 6: Concatenate segments
echo "Step 6: Concatenating video segments..."
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

# Step 7: Add audio with fade out
echo "Step 7: Adding audio with fade out..."
FADE_START=$(echo "$VIDEO_DURATION - 1.0" | bc -l)

ffmpeg -y -i "$VIDEO_NO_AUDIO" -i "$EXTRACTED_AUDIO" \
    -af "afade=t=out:st=${FADE_START}:d=1.0" \
    -c:v copy \
    -c:a aac \
    -b:a 192k \
    -shortest \
    -movflags +faststart \
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

