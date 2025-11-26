#!/bin/bash

# Configuration
SOURCE_DIR="$HOME/pcmp_home"
CSV_DATABASE="$SOURCE_DIR/chip_database.csv"
SCRIPT_DIR="/home/tobi/chipmunk/pcmp_chips"
LAYER_PROPS="$SCRIPT_DIR/klayout-red-green-yellow-fets.lyp"
KLAYOUT="flatpak run de.klayout.KLayout"

# Image parameters
IMAGE_WIDTH="${IMAGE_WIDTH:-1920}"
IMAGE_HEIGHT="${IMAGE_HEIGHT:-1080}"
THUMBNAIL_WIDTH="${THUMBNAIL_WIDTH:-640}"
THUMBNAIL_HEIGHT="${THUMBNAIL_HEIGHT:-480}"
FILE_LIMIT="${FILE_LIMIT:-0}"  # 0 = no limit, otherwise stop after N files

# Output files (in source directory)
IMAGE_LIST="$SOURCE_DIR/layout_images.txt"
THUMBNAIL_LIST="$SOURCE_DIR/layout_thumbnails.txt"
CONVERTED_FILES="$SOURCE_DIR/converted_files.txt"
ERROR_LOG="$SOURCE_DIR/layout_conversion_errors.log"

cd "$SOURCE_DIR" || exit 1

# Convert to absolute paths for clickability
IMAGE_LIST="$(realpath -m "$IMAGE_LIST")"
THUMBNAIL_LIST="$(realpath -m "$THUMBNAIL_LIST")"
CONVERTED_FILES="$(realpath -m "$CONVERTED_FILES")"
ERROR_LOG="$(realpath -m "$ERROR_LOG")"

# Clear/create output files
> "$IMAGE_LIST"
> "$THUMBNAIL_LIST"
> "$CONVERTED_FILES"
> "$ERROR_LOG"

echo "Starting layout conversion from CSV database at $(date)" | tee -a "$ERROR_LOG"
echo "CSV Database: $CSV_DATABASE" | tee -a "$ERROR_LOG"
echo "Image size: ${IMAGE_WIDTH}x${IMAGE_HEIGHT}" | tee -a "$ERROR_LOG"
echo "Thumbnail size: ${THUMBNAIL_WIDTH}x${THUMBNAIL_HEIGHT}" | tee -a "$ERROR_LOG"
if [ "$FILE_LIMIT" -gt 0 ]; then
    echo "File limit: $FILE_LIMIT files" | tee -a "$ERROR_LOG"
else
    echo "File limit: unlimited" | tee -a "$ERROR_LOG"
fi

# Check if CSV database exists
if [ ! -f "$CSV_DATABASE" ]; then
    echo "Error: CSV database not found at $CSV_DATABASE" | tee -a "$ERROR_LOG"
    echo "Please run generate_chip_database.sh first" | tee -a "$ERROR_LOG"
    exit 1
fi

# Function to extract CIF from MOSIS file
extract_cif_from_mosis() {
    local mosis_file="$1"
    local cif_file="$2"
    
    # MOSIS files have CIF data after "CIF:" line until "E" (CIF End command) or "REQUEST:  END"
    # Extract from line after "CIF:" to first "E" or "REQUEST", including the "E" command, then remove leading blank lines
    awk '/^CIF:/{flag=1; next} flag{if(/^E$|^REQUEST:  END/) {print; exit} else print}' "$mosis_file" | sed '/./,$!d' > "$cif_file"
    
    # Check if extraction was successful (file should contain DS command somewhere)
    if [ -s "$cif_file" ] && grep -q "^DS" "$cif_file"; then
        return 0
    else
        rm -f "$cif_file"
        return 1
    fi
}

# Function to get CIF file path (either extracted from .mosis or direct .cif)
get_cif_file() {
    local source_file="$1"
    local pname="$2"
    local dir="$3"
    local temp_cif="$dir/.${pname}_extracted.cif"
    local temp_mosis=""
    
    # If source is .cif file, use it directly (may need decompression)
    if [[ "$source_file" == *.cif ]] || [[ "$source_file" == *.cif.gz ]]; then
        if [[ "$source_file" == *.gz ]]; then
            # Decompress to temp file
            gunzip -c "$source_file" > "$temp_cif" 2>/dev/null
            if [ $? -eq 0 ] && [ -f "$temp_cif" ]; then
                echo "$temp_cif"
                return 0
            else
                return 1
            fi
        else
            # Use .cif file directly
            echo "$source_file"
            return 0
        fi
    else
        # Extract CIF from .mosis file (may be compressed)
        local mosis_file="$source_file"
        if [[ "$source_file" == *.gz ]]; then
            # Decompress .mosis.gz to temp file first
            temp_mosis=$(mktemp)
            gunzip -c "$source_file" > "$temp_mosis" 2>/dev/null
            if [ $? -ne 0 ] || [ ! -f "$temp_mosis" ]; then
                rm -f "$temp_mosis"
                return 1
            fi
            mosis_file="$temp_mosis"
        fi
        
        # Extract CIF from .mosis file
        if extract_cif_from_mosis "$mosis_file" "$temp_cif"; then
            # Clean up temp mosis file if we created one
            [ -n "$temp_mosis" ] && rm -f "$temp_mosis"
            echo "$temp_cif"
            return 0
        else
            # Clean up temp mosis file if we created one
            [ -n "$temp_mosis" ] && rm -f "$temp_mosis"
            return 1
        fi
    fi
}

FILES_PROCESSED=0

# Read CSV file using Python to handle quoted fields correctly
python3 -c "
import csv
import sys

with open('$CSV_DATABASE', 'r', encoding='utf-8') as f:
    reader = csv.reader(f)
    next(reader)  # Skip header
    for row in reader:
        if len(row) >= 16:
            # Join fields with | separator for bash to parse
            print('|'.join(row))
" | while IFS='|' read -r mosis_file path username p_name technology pads size_width size_height description cad_tool cif_date archive_date primary_date date_source normalized_date mosis_id; do
    # Skip if no p_name
    if [ -z "$p_name" ]; then
        continue
    fi
    
    # Check file limit
    if [ "$FILE_LIMIT" -gt 0 ] && [ "$FILES_PROCESSED" -ge "$FILE_LIMIT" ]; then
        echo "Reached file limit of $FILE_LIMIT files" | tee -a "$ERROR_LOG"
        break
    fi
    
    # Construct full path to source file
    source_file="$SOURCE_DIR/$path"
    
    # Check if source file exists
    if [ ! -f "$source_file" ]; then
        echo "Warning: Source file not found: $source_file" | tee -a "$ERROR_LOG"
        continue
    fi
    
    # Get directory for output images
    dir=$(dirname "$source_file")
    
    # Create PNG filenames from P-NAME
    png_file="$dir/${p_name}_layout.png"
    thumbnail_file="$dir/${p_name}_layout_thumbnail.png"
    
    # Skip if PNG already exists
    if [ -f "$png_file" ] && [ -f "$thumbnail_file" ]; then
        echo "$(realpath "$png_file")" >> "$IMAGE_LIST"
        echo "$(realpath "$thumbnail_file")" >> "$THUMBNAIL_LIST"
        echo "$path" >> "$CONVERTED_FILES"
        echo "[$((FILES_PROCESSED + 1))] Skipped (PNG exists): $png_file (P-NAME: $p_name)"
        continue
    fi
    
    FILES_PROCESSED=$((FILES_PROCESSED + 1))
    echo "[$FILES_PROCESSED] Processing: $path (P-NAME: $p_name)"
    
    # Get CIF file (extracted or direct)
    cif_file=$(get_cif_file "$source_file" "$p_name" "$dir")
    if [ $? -ne 0 ] || [ -z "$cif_file" ]; then
        echo "Error: Could not get CIF file from: $path" | tee -a "$ERROR_LOG"
        continue
    fi
    
    # Track if we need to clean up temp CIF file
    cleanup_temp=false
    if [[ "$cif_file" == *".${p_name}_extracted.cif" ]]; then
        cleanup_temp=true
    fi
    
    # Generate full-size image
    if $KLAYOUT -zz -nc -rx \
        -r "$SCRIPT_DIR/export_cif_png.rb" \
        -rd input="$cif_file" \
        -rd output="$png_file" \
        -rd session="$LAYER_PROPS" \
        -rd width="$IMAGE_WIDTH" \
        -rd height="$IMAGE_HEIGHT" 2>>"$ERROR_LOG"; then
        
        if [ -f "$png_file" ]; then
            echo "[$FILES_PROCESSED] Created: $png_file"
            
            # Generate thumbnail
            if $KLAYOUT -zz -nc -rx \
                -r "$SCRIPT_DIR/export_cif_png.rb" \
                -rd input="$cif_file" \
                -rd output="$thumbnail_file" \
                -rd session="$LAYER_PROPS" \
                -rd width="$THUMBNAIL_WIDTH" \
                -rd height="$THUMBNAIL_HEIGHT" 2>>"$ERROR_LOG"; then
                
                if [ -f "$thumbnail_file" ]; then
                    echo "[$FILES_PROCESSED] Created thumbnail: $thumbnail_file"
                    echo "$(realpath "$png_file")" >> "$IMAGE_LIST"
                    echo "$(realpath "$thumbnail_file")" >> "$THUMBNAIL_LIST"
                    echo "$path" >> "$CONVERTED_FILES"
                else
                    echo "Warning: Thumbnail not created for $path" >> "$ERROR_LOG"
                    # Still log the full image even if thumbnail failed
                    echo "$(realpath "$png_file")" >> "$IMAGE_LIST"
                    echo "$path" >> "$CONVERTED_FILES"
                fi
            else
                echo "Warning: Error creating thumbnail for: $path" >> "$ERROR_LOG"
                # Still log the full image even if thumbnail failed
                echo "$(realpath "$png_file")" >> "$IMAGE_LIST"
                echo "$path" >> "$CONVERTED_FILES"
            fi
        else
            echo "Error: PNG not created for $path" >> "$ERROR_LOG"
        fi
    else
        echo "Error converting CIF from: $path" >> "$ERROR_LOG"
    fi
    
    # Clean up temporary CIF if we created one
    if [ "$cleanup_temp" = true ] && [ -f "$cif_file" ]; then
        rm -f "$cif_file"
    fi
done

echo "" | tee -a "$ERROR_LOG"
echo "Conversion complete at $(date)" | tee -a "$ERROR_LOG"
echo "Total chips processed: $FILES_PROCESSED" | tee -a "$ERROR_LOG"
echo "Images created: $(wc -l < "$IMAGE_LIST")" | tee -a "$ERROR_LOG"
echo "" | tee -a "$ERROR_LOG"
echo "Output files:" | tee -a "$ERROR_LOG"
echo "  - Converted files: $CONVERTED_FILES" | tee -a "$ERROR_LOG"
echo "  - Generated PNG images: $IMAGE_LIST" | tee -a "$ERROR_LOG"
echo "  - Generated thumbnails: $THUMBNAIL_LIST" | tee -a "$ERROR_LOG"
echo "  - Errors: $ERROR_LOG" | tee -a "$ERROR_LOG"
echo "" | tee -a "$ERROR_LOG"
echo "Generated files:" | tee -a "$ERROR_LOG"
echo "================" | tee -a "$ERROR_LOG"
echo "$IMAGE_LIST" | tee -a "$ERROR_LOG"
echo "$THUMBNAIL_LIST" | tee -a "$ERROR_LOG"
echo "$CONVERTED_FILES" | tee -a "$ERROR_LOG"
echo "$ERROR_LOG" | tee -a "$ERROR_LOG"
