#!/bin/bash

# Script to create MP4 movie from chip layout thumbnails ordered by date
# Each thumbnail is overlaid with:
#   - Year in upper-left corner
#   - Chip name (part before underscore) over username in lower-left corner
#
# Usage: ./create_dated_layout_movie.sh [output_file] [frame_rate] [compression]
# Examples:
#   ./create_dated_layout_movie.sh chip_layouts_by_date.mp4
#   ./create_dated_layout_movie.sh chip_layouts_by_date.mp4 2 23
#   FILE_LIMIT=10 ./create_dated_layout_movie.sh test.mp4  # Test with 10 files

CHIP_DIR="${CHIP_DIR:-$HOME/pcmp_home}"
CSV_DATABASE="${CSV_DATABASE:-$CHIP_DIR/chip_database.csv}"
IMAGE_LIST="${IMAGE_LIST:-$CHIP_DIR/layout_images.txt}"
OUTPUT_FILE="${1:-${OUTPUT_FILE:-$HOME/pcmp_home/pcmp_chips.mp4}}"
FRAME_RATE="${2:-${FRAME_RATE:-4}}"
CRF="${3:-${CRF:-23}}"
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

# Create temporary directories
TEMP_DIR=$(mktemp -d)
ANNOTATED_DIR="$TEMP_DIR/annotated"
mkdir -p "$ANNOTATED_DIR"

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

echo "Creating dated chip layout movie"
echo "================================="
echo "CSV Database: $CSV_DATABASE"
echo "Image list: $IMAGE_LIST"
echo "Output file: $OUTPUT_FILE"
echo "Frame rate: $FRAME_RATE fps"
echo "Compression (CRF): $CRF (lower = higher quality)"
if [ "$FILE_LIMIT" -gt 0 ]; then
    echo "File limit: $FILE_LIMIT files (test mode)"
else
    echo "File limit: unlimited"
fi
echo ""

# Function to extract year from normalized_date (YYYY-MM-DD format)
extract_year() {
    local date_str="$1"
    # Extract first 4 digits (year) from YYYY-MM-DD format
    echo "$date_str" | cut -d'-' -f1
}

# Function to extract chip name (part before first underscore) from p_name
extract_chip_name() {
    local pname="$1"
    # Extract part before first underscore
    echo "$pname" | cut -d'_' -f1
}

# Function to overlay text on image
# Year in upper-left, username in lower-left, chip name in lower-right
overlay_title_block() {
    local input_image="$1"
    local output_image="$2"
    local year="$3"
    local chip_name="$4"
    local username="$5"
    
    # Get image dimensions
    local width=$(identify -format "%w" "$input_image")
    local height=$(identify -format "%h" "$input_image")
    
    # Calculate font sizes based on image height
    local font_size_year=$((height / 10))
    local font_size_chip=$((height / 20))
    local font_size_user=$((height / 17))
    
    # Calculate padding/margins
    local margin=$((height / 30))
    local shadow_offset=$((height / 200))
    if [ "$shadow_offset" -lt 2 ]; then
        shadow_offset=2
    fi
    
    # Create text images with shadows
    local temp_dir=$(dirname "$output_image")
    local year_img="$temp_dir/.year_$$.png"
    local year_shadow="$temp_dir/.year_shadow_$$.png"
    local chip_img="$temp_dir/.chip_$$.png"
    local chip_shadow="$temp_dir/.chip_shadow_$$.png"
    local user_img="$temp_dir/.user_$$.png"
    local user_shadow="$temp_dir/.user_shadow_$$.png"
    
    # Create year text (upper-left) - bright white with shadow
    if [ -z "$year" ] || [ "$year" = "Unknown" ] || [ "$year" = "?" ]; then
        year="?"
    fi
    # Create shadow (black, semi-transparent)
    convert -background transparent \
        -fill "rgba(0,0,0,0.7)" \
        -font "DejaVu-Sans-Bold" \
        -pointsize "$font_size_year" \
        label:"$year" \
        "$year_shadow"
    # Create text (bright white)
    convert -background transparent \
        -fill "rgba(255,255,255,1.0)" \
        -font "DejaVu-Sans-Bold" \
        -pointsize "$font_size_year" \
        label:"$year" \
        "$year_img"
    
    # Create chip name text (lower-right) - bright white with shadow, right-aligned
    if [ -z "$chip_name" ]; then
        chip_name="Unknown"
    fi
    # Create shadow (black, semi-transparent)
    convert -background transparent \
        -fill "rgba(0,0,0,0.7)" \
        -font "DejaVu-Sans-Bold" \
        -pointsize "$font_size_chip" \
        label:"$chip_name" \
        "$chip_shadow"
    # Create text (bright white)
    convert -background transparent \
        -fill "rgba(255,255,255,1.0)" \
        -font "DejaVu-Sans-Bold" \
        -pointsize "$font_size_chip" \
        label:"$chip_name" \
        "$chip_img"
    
    # Create username text (lower-left) - bright white with shadow
    if [ -z "$username" ]; then
        username="Unknown"
    fi
    # Create shadow (black, semi-transparent)
    convert -background transparent \
        -fill "rgba(0,0,0,0.7)" \
        -font "DejaVu-Sans" \
        -pointsize "$font_size_user" \
        label:"$username" \
        "$user_shadow"
    # Create text (bright white)
    convert -background transparent \
        -fill "rgba(255,255,255,1.0)" \
        -font "DejaVu-Sans" \
        -pointsize "$font_size_user" \
        label:"$username" \
        "$user_img"
    
    # Get dimensions for positioning
    local year_w=$(identify -format "%w" "$year_img")
    local year_h=$(identify -format "%h" "$year_img")
    local chip_w=$(identify -format "%w" "$chip_img")
    local chip_h=$(identify -format "%h" "$chip_img")
    local user_w=$(identify -format "%w" "$user_img")
    local user_h=$(identify -format "%h" "$user_img")
    
    # Calculate positions
    local year_x=$margin
    local year_y=$margin
    local user_x=$margin
    local user_y=$((height - user_h - margin))
    local chip_x=$((width - chip_w - margin))
    local chip_y=$((height - chip_h - margin))
    
    # Remove output file if it exists to ensure overwrite
    rm -f "$output_image"
    
    # Composite all text overlays with shadows onto original image
    # Shadows first (offset by shadow_offset), then text on top
    convert "$input_image" \
        \( "$year_shadow" -geometry +$((year_x + shadow_offset))+$((year_y + shadow_offset)) \) -composite \
        \( "$year_img" -geometry +${year_x}+${year_y} \) -composite \
        \( "$user_shadow" -geometry +$((user_x + shadow_offset))+$((user_y + shadow_offset)) \) -composite \
        \( "$user_img" -geometry +${user_x}+${user_y} \) -composite \
        \( "$chip_shadow" -geometry +$((chip_x + shadow_offset))+$((chip_y + shadow_offset)) \) -composite \
        \( "$chip_img" -geometry +${chip_x}+${chip_y} \) -composite \
        "$output_image"
    
    # Cleanup temp files
    rm -f "$year_img" "$year_shadow" "$chip_img" "$chip_shadow" "$user_img" "$user_shadow"
}

# Build sorted list of images with dates from CSV database
echo "Building sorted image list by normalized_date..."
SORTED_LIST="$TEMP_DIR/sorted_images.txt"
> "$SORTED_LIST"

# Create associative arrays (bash 4+) to store CSV data by p_name
declare -A csv_normalized_date
declare -A csv_username
declare -A csv_pname

# Load CSV database into associative arrays
# CSV format: mosis_file,path,username,p_name,technology,pads,size_width,size_height,description,cad_tool,cif_date,archive_date,primary_date,date_source,normalized_date,mosis_id
# Use process substitution to avoid subshell issue
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
    next(reader)  # Skip header
    for row in reader:
        if len(row) >= 16:
            pname = row[3].strip()
            username = row[2].strip()
            normalized_date = row[14].strip()
            if pname:
                print(f'{pname}|{normalized_date}|{username}')
")

# Process each image from the image list
TOTAL=0
PROCESSED=0
NO_DATE=0
MISSING=0
FILES_ADDED=0

while IFS= read -r full_image; do
    ((TOTAL++))
    
    # Convert full image path to thumbnail path
    thumbnail="${full_image/_layout.png/_layout_thumbnail.png}"
    
    # Check if thumbnail exists
    if [ ! -f "$thumbnail" ]; then
        ((MISSING++))
        continue
    fi
    
    # Extract P-NAME from image filename
    # Format: {path}/{pname}_layout.png -> {pname}
    basename=$(basename "$full_image" "_layout.png")
    pname="$basename"
    
    # Get normalized_date and username for this P-NAME
    normalized_date="${csv_normalized_date[$pname]:-}"
    username="${csv_username[$pname]:-}"
    
    # Debug: show first few lookups
    if [ $PROCESSED -le 3 ]; then
        echo "Debug: P-NAME='$pname', normalized_date='$normalized_date', username='$username'"
    fi
    
    if [ -z "$normalized_date" ]; then
        ((NO_DATE++))
        # Use a default date for sorting (will sort to end)
        normalized_date="9999-99-99"
    fi
    
    # Extract year from normalized_date (YYYY-MM-DD format)
    year=$(extract_year "$normalized_date")
    if [ -z "$year" ] || [ "$year" = "9999" ]; then
        year="?"
    fi
    
    # Extract chip name (part before first underscore)
    chip_name=$(extract_chip_name "$pname")
    
    # Use normalized_date directly for sorting (already in YYYY-MM-DD format)
    sort_date="$normalized_date"
    
    # Write to sorted list: sort_date|year|chip_name|username|pname|thumbnail_path
    echo "$sort_date|$year|$chip_name|$username|$pname|$thumbnail" >> "$SORTED_LIST"
    
    ((PROCESSED++))
done < "$IMAGE_LIST"

# Sort by normalized_date (oldest first)
sort -t'|' -k1,1 "$SORTED_LIST" > "$SORTED_LIST.sorted"

# Apply FILE_LIMIT to sorted list if specified
if [ "$FILE_LIMIT" -gt 0 ]; then
    head -n "$FILE_LIMIT" "$SORTED_LIST.sorted" > "$SORTED_LIST.sorted.limited"
    mv "$SORTED_LIST.sorted.limited" "$SORTED_LIST.sorted"
    echo "Applied FILE_LIMIT: Using first $FILE_LIMIT files (oldest chronologically)"
fi

echo "Found $TOTAL images"
echo "Processed: $PROCESSED"
echo "Missing thumbnails: $MISSING"
echo "No date: $NO_DATE"
if [ "$FILE_LIMIT" -gt 0 ]; then
    echo "Limited to: $FILE_LIMIT files (after sorting by date)"
fi
echo ""

# Create annotated images
echo "Creating annotated images..."
# Clear any existing annotated images to ensure fresh generation
rm -f "$ANNOTATED_DIR"/*.png
COUNT=0
TOTAL_ANNOTATED=$(wc -l < "$SORTED_LIST.sorted")

while IFS='|' read -r sort_date year chip_name username pname thumbnail; do
    # Check file limit during annotation (in case sorting changed order)
    if [ "$FILE_LIMIT" -gt 0 ] && [ "$COUNT" -ge "$FILE_LIMIT" ]; then
        echo "Reached file limit of $FILE_LIMIT files during annotation"
        break
    fi
    
    ((COUNT++))
    
    # Create annotated image filename
    numbered_name=$(printf "%06d.png" $COUNT)
    annotated_image="$ANNOTATED_DIR/$numbered_name"
    
    # Overlay title block: year in UL, chip_name over username in LL
    overlay_title_block "$thumbnail" "$annotated_image" "$year" "$chip_name" "$username"
    
    if [ $((COUNT % 50)) -eq 0 ]; then
        echo "Annotated $COUNT/$TOTAL_ANNOTATED images..."
    fi
done < "$SORTED_LIST.sorted"

echo "Created $COUNT annotated images"
echo ""

# Create MP4 using ffmpeg
echo "Encoding MP4 video..."
echo ""

FFMPEG_OUTPUT=$(ffmpeg -y \
    -framerate "$FRAME_RATE" \
    -pattern_type glob \
    -i "$ANNOTATED_DIR/*.png" \
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

# Check if file was created
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
            else
                echo "Duration: ~$((COUNT / FRAME_RATE)) seconds (estimated)"
            fi
        else
            echo "Duration: ~$((COUNT / FRAME_RATE)) seconds (estimated)"
        fi
        
        # Output absolute path for control-clicking
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

