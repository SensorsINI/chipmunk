#!/bin/bash

# Configuration
CHIP_DIR="$HOME/chip_collection"
IMAGE_LIST="$CHIP_DIR/layout_images.txt"
CONVERTED_FILES="$CHIP_DIR/converted_mosis_files.txt"
NO_PNAME_FILES="$CHIP_DIR/no_pname_mosis_files.txt"
DESCRIPTION_MAP="$CHIP_DIR/chip_descriptions.txt"
ERROR_LOG="$CHIP_DIR/layout_conversion_errors.log"
SCRIPT_DIR="/home/tobi/chipmunk/pcmp_chips"
LAYER_PROPS="$CHIP_DIR/klayout-red-green-yellow-fets.lyp"
KLAYOUT="flatpak run de.klayout.KLayout"

# Image parameters
IMAGE_WIDTH="${IMAGE_WIDTH:-1920}"
IMAGE_HEIGHT="${IMAGE_HEIGHT:-1080}"
THUMBNAIL_WIDTH="${THUMBNAIL_WIDTH:-240}"
THUMBNAIL_HEIGHT="${THUMBNAIL_HEIGHT:-180}"
FILE_LIMIT="${FILE_LIMIT:-0}"  # 0 = no limit, otherwise stop after N files

# Clean up any stale temp files from previous runs
rm -f /tmp/pname_seen_*

# Clear/create output files
> "$IMAGE_LIST"
> "$CONVERTED_FILES"
> "$NO_PNAME_FILES"
> "$DESCRIPTION_MAP"
> "$ERROR_LOG"

echo "Starting unique MOSIS layout conversion at $(date)" | tee -a "$ERROR_LOG"
echo "Image size: ${IMAGE_WIDTH}x${IMAGE_HEIGHT}" | tee -a "$ERROR_LOG"
echo "Thumbnail size: ${THUMBNAIL_WIDTH}x${THUMBNAIL_HEIGHT}" | tee -a "$ERROR_LOG"
if [ "$FILE_LIMIT" -gt 0 ]; then
    echo "File limit: $FILE_LIMIT files" | tee -a "$ERROR_LOG"
else
    echo "File limit: unlimited" | tee -a "$ERROR_LOG"
fi

# Function to extract P-NAME from MOSIS file (ignore any leading whitespace)
extract_pname_from_mosis() {
    local mosis_file="$1"
    # Extract P-NAME value, ignore any leading whitespace
    grep -m1 -E "^[[:space:]]*P-NAME:" "$mosis_file" | sed -E 's/^[[:space:]]*P-NAME:[[:space:]]*//' | tr -d '\r' || echo ""
}

# Function to extract DESCRIPTION from MOSIS file (ignore any leading whitespace)
extract_description_from_mosis() {
    local mosis_file="$1"
    # Extract DESCRIPTION value, ignore any leading whitespace
    grep -m1 -E "^[[:space:]]*DESCRIPTION:" "$mosis_file" | sed -E 's/^[[:space:]]*DESCRIPTION:[[:space:]]*//' | tr -d '\r' || echo ""
}

# Function to extract CIF from MOSIS file
extract_cif_from_mosis() {
    local mosis_file="$1"
    local cif_file="$2"
    
    # MOSIS files have CIF data after "CIF:" line until "E" (CIF End command) or "REQUEST:  END"
    # Extract from line after "CIF:" to "E" or "REQUEST", then remove leading blank lines
    awk '/^CIF:/{flag=1; next} flag{print} /^E$|^REQUEST:  END/{if(flag) exit}' "$mosis_file" | sed '/./,$!d' > "$cif_file"
    
    # Check if extraction was successful (file should contain DS command somewhere)
    if [ -s "$cif_file" ] && grep -q "^DS" "$cif_file"; then
        return 0
    else
        rm -f "$cif_file"
        return 1
    fi
}

echo "Finding all MOSIS files..." | tee -a "$ERROR_LOG"

# Track unique P-NAMEs and files processed
FILES_PROCESSED=0

# Use process substitution instead of pipe to avoid subshell issue
while read mosis_file; do
    # Extract P-NAME from MOSIS header first
    pname=$(extract_pname_from_mosis "$mosis_file")
    
    if [ -z "$pname" ]; then
        echo "$mosis_file" >> "$NO_PNAME_FILES"
        echo "Skipped (no P-NAME): $mosis_file"
        continue
    fi
    
    # Skip if we've already seen this P-NAME (duplicate)
    if [ -f "/tmp/pname_seen_${pname}" ]; then
        echo "Skipped (duplicate P-NAME: $pname): $mosis_file"
        continue
    fi
    
    # Check file limit BEFORE processing this unique P-NAME
    if [ "$FILE_LIMIT" -gt 0 ] && [ "$FILES_PROCESSED" -ge "$FILE_LIMIT" ]; then
        echo "Reached file limit of $FILE_LIMIT files" | tee -a "$ERROR_LOG"
        break
    fi
    
    # Mark this P-NAME as seen and increment counter for unique chips
    touch "/tmp/pname_seen_${pname}"
    FILES_PROCESSED=$((FILES_PROCESSED + 1))
    
    # Get directory
    dir=$(dirname "$mosis_file")
    
    # Create PNG filenames from P-NAME
    png_file="$dir/${pname}_layout.png"
    thumbnail_file="$dir/${pname}_layout_thumbnail.png"
    
    # Skip if PNG already exists (but still count it)
    if [ -f "$png_file" ] && [ -f "$thumbnail_file" ]; then
        echo "$png_file" >> "$IMAGE_LIST"
        echo "$mosis_file" >> "$CONVERTED_FILES"
        echo "[$FILES_PROCESSED/$FILE_LIMIT] Skipped (PNG exists): $png_file (P-NAME: $pname)"
        continue
    fi
    
    echo "[$FILES_PROCESSED/$FILE_LIMIT] Processing: $mosis_file (P-NAME: $pname)"
    
    # Extract CIF from MOSIS
    temp_cif="$dir/.${pname}_extracted.cif"
    if extract_cif_from_mosis "$mosis_file" "$temp_cif"; then
        # Generate full-size image
        if $KLAYOUT -zz -nc -rx \
            -r "$SCRIPT_DIR/export_cif_png.rb" \
            -rd input="$temp_cif" \
            -rd output="$png_file" \
            -rd session="$LAYER_PROPS" \
            -rd width="$IMAGE_WIDTH" \
            -rd height="$IMAGE_HEIGHT" 2>>"$ERROR_LOG"; then
            
            if [ -f "$png_file" ]; then
                echo "[$FILES_PROCESSED/$FILE_LIMIT] Created: $png_file"
                
                # Generate thumbnail
                if $KLAYOUT -zz -nc -rx \
                    -r "$SCRIPT_DIR/export_cif_png.rb" \
                    -rd input="$temp_cif" \
                    -rd output="$thumbnail_file" \
                    -rd session="$LAYER_PROPS" \
                    -rd width="$THUMBNAIL_WIDTH" \
                    -rd height="$THUMBNAIL_HEIGHT" 2>>"$ERROR_LOG"; then
                    
                    if [ -f "$thumbnail_file" ]; then
                        echo "[$FILES_PROCESSED/$FILE_LIMIT] Created thumbnail: $thumbnail_file"
                        echo "$png_file" >> "$IMAGE_LIST"
                        echo "$mosis_file" >> "$CONVERTED_FILES"
                        
                        # Extract and log DESCRIPTION if present
                        description=$(extract_description_from_mosis "$mosis_file")
                        if [ -n "$description" ]; then
                            echo "$pname|$description" >> "$DESCRIPTION_MAP"
                        fi
                    else
                        echo "Warning: Thumbnail not created for $mosis_file" >> "$ERROR_LOG"
                        # Still log the full image even if thumbnail failed
                        echo "$png_file" >> "$IMAGE_LIST"
                        echo "$mosis_file" >> "$CONVERTED_FILES"
                        
                        # Extract and log DESCRIPTION if present
                        description=$(extract_description_from_mosis "$mosis_file")
                        if [ -n "$description" ]; then
                            echo "$pname|$description" >> "$DESCRIPTION_MAP"
                        fi
                    fi
                else
                    echo "Warning: Error creating thumbnail for: $mosis_file" >> "$ERROR_LOG"
                    # Still log the full image even if thumbnail failed
                    echo "$png_file" >> "$IMAGE_LIST"
                    echo "$mosis_file" >> "$CONVERTED_FILES"
                    
                    # Extract and log DESCRIPTION if present
                    description=$(extract_description_from_mosis "$mosis_file")
                    if [ -n "$description" ]; then
                        echo "$pname|$description" >> "$DESCRIPTION_MAP"
                    fi
                fi
            else
                echo "Error: PNG not created for $mosis_file" >> "$ERROR_LOG"
            fi
        else
            echo "Error converting extracted CIF from: $mosis_file" >> "$ERROR_LOG"
        fi
        
        # Clean up temporary CIF
        rm -f "$temp_cif"
    else
        echo "Error extracting CIF from: $mosis_file" >> "$ERROR_LOG"
    fi
done < <(find "$CHIP_DIR" -type f -name "*.mosis" | sort)

# Clean up temporary marker files
rm -f /tmp/pname_seen_*

echo "Conversion complete at $(date)" | tee -a "$ERROR_LOG"
echo "Total unique chips processed: $FILES_PROCESSED" | tee -a "$ERROR_LOG"
echo "Images created: $(wc -l < "$IMAGE_LIST")" | tee -a "$ERROR_LOG"
echo "Chips with descriptions: $(wc -l < "$DESCRIPTION_MAP")" | tee -a "$ERROR_LOG"
echo "MOSIS files without P-NAME: $(wc -l < "$NO_PNAME_FILES")" | tee -a "$ERROR_LOG"
echo "" | tee -a "$ERROR_LOG"
echo "Output files:" | tee -a "$ERROR_LOG"
echo "  - Converted MOSIS files: $CONVERTED_FILES" | tee -a "$ERROR_LOG"
echo "  - Generated PNG images: $IMAGE_LIST" | tee -a "$ERROR_LOG"
echo "  - Chip descriptions: $DESCRIPTION_MAP" | tee -a "$ERROR_LOG"
echo "  - Files without P-NAME: $NO_PNAME_FILES" | tee -a "$ERROR_LOG"
echo "  - Errors: $ERROR_LOG" | tee -a "$ERROR_LOG"
