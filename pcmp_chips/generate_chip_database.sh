#!/bin/bash

# Script to generate a CSV database of chip designs from .mosis files
# Output: chip_database.csv

# Source directory (extracted archive)
SOURCE_DIR="$HOME/pcmp_home"
# Output directory (same as source for now)
OUTPUT_DIR="$HOME/pcmp_home"
# Archive file for extracting original dates
ARCHIVE_FILE="$HOME/pcmp-home-4TB.tgz"

cd "$SOURCE_DIR" || exit 1

# Ensure output directory exists
mkdir -p "$OUTPUT_DIR"

CSV_OUTPUT="$OUTPUT_DIR/chip_database.csv"
ARCHIVE_DATE_MAP=$(mktemp)
SKIPPED_FILES_LIST=$(mktemp)
REPORT_FILES_LIST=$(mktemp)
EMAIL_FILES_LIST=$(mktemp)

# Extract archive dates if archive exists
if [ -f "$ARCHIVE_FILE" ]; then
    echo "Extracting dates from archive..."
    # Extract: path|date (YYYY-MM-DD format)
    # tar -tv output format: permissions owner size date time [pcmp] path
    # Note: "pcmp" appears as field 6, and "home/..." starts at field 7
    # Date is in fields 4-5, path is fields 6-NF (need to join "pcmp" + "home/...")
    tar -tzvf "$ARCHIVE_FILE" | awk '{
        # Extract date fields (4, 5) - date and time
        date = $4 " " $5
        # Extract path - field 6 is "pcmp", field 7+ is "home/..."
        # Reconstruct full path: "pcmp home/..."
        path = ""
        for (i = 6; i <= NF; i++) {
            if (i > 6) path = path " "
            path = path $i
        }
        # Output: path|date
        print path "|" date
    }' > "$ARCHIVE_DATE_MAP"
fi

# Clear/create output file
echo "mosis_file,path,username,p_name,technology,pads,size_width,size_height,description,cad_tool,cif_date,archive_date,primary_date,date_source,normalized_date,mosis_id" > "$CSV_OUTPUT"

echo "Generating chip database CSV..."
echo "==============================="
echo ""

# Function to escape CSV fields (handle commas and quotes)
escape_csv() {
    local field="$1"
    # Handle empty/null
    if [ -z "$field" ]; then
        echo ""
        return
    fi
    # If field contains comma, quote, or newline, wrap in quotes and escape quotes
    if [[ "$field" =~ [,\"$'\n'] ]]; then
        # Replace " with ""
        field="${field//\"/\"\"}"
        echo "\"$field\""
    else
        echo "$field"
    fi
}

# Function to normalize date to YYYY-MM-DD format
# Handles formats like: "9 Nov 93", "28 Oct 93", "May 1 1991", "1993-02-23", etc.
normalize_date() {
    local date_str="$1"
    if [ -z "$date_str" ]; then
        echo ""
        return
    fi
    
    # If already in YYYY-MM-DD format, return as-is
    if [[ "$date_str" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]; then
        echo "$date_str"
        return
    fi
    
    # Try to parse with date command (handles many formats)
    # First try common formats
    local normalized=""
    
    # Try parsing as "DD MMM YY" or "DD MMM YYYY" or "MMM DD YYYY"
    normalized=$(date -d "$date_str" "+%Y-%m-%d" 2>/dev/null)
    if [ -n "$normalized" ] && [ "$normalized" != "$date_str" ]; then
        echo "$normalized"
        return
    fi
    
    # Try parsing with different assumptions
    # Format: "9 Nov 93" -> assume 1993 (chips are from 1980s-1990s)
    if [[ "$date_str" =~ ^([0-9]+)[[:space:]]+([A-Za-z]+)[[:space:]]+([0-9]{2})$ ]]; then
        local day="${BASH_REMATCH[1]}"
        local month="${BASH_REMATCH[2]}"
        local year="${BASH_REMATCH[3]}"
        # Convert 2-digit year to 4-digit (assume 1900s for years 00-99 for these old chips)
        year="19$year"
        normalized=$(date -d "$month $day $year" "+%Y-%m-%d" 2>/dev/null)
        if [ -n "$normalized" ]; then
            echo "$normalized"
            return
        fi
    fi
    
    # Format: "MMM DD YYYY" or "MMM DD, YYYY" (e.g., "May 1 1991" or "May 1, 1991")
    # Remove comma first, then parse
    local date_clean=$(echo "$date_str" | sed 's/,//g')
    if [[ "$date_clean" =~ ^([A-Za-z]+)[[:space:]]+([0-9]+)[[:space:]]+([0-9]{4})$ ]]; then
        local month="${BASH_REMATCH[1]}"
        local day="${BASH_REMATCH[2]}"
        local year="${BASH_REMATCH[3]}"
        normalized=$(date -d "$month $day $year" "+%Y-%m-%d" 2>/dev/null)
        if [ -n "$normalized" ]; then
            echo "$normalized"
            return
        fi
    fi
    
    # If all parsing fails, return original (will sort as string)
    echo "$date_str"
}

# Function to extract P-NAME from MOSIS file (case-insensitive)
extract_pname_from_mosis() {
    local mosis_file="$1"
    awk 'BEGIN{IGNORECASE=1} /^[[:space:]]*P-NAME:/ {gsub(/^[[:space:]]*[Pp]-[Nn][Aa][Mm][Ee]:[[:space:]]*/, ""); gsub(/\r/, ""); gsub(/^[[:space:]]+|[[:space:]]+$/, ""); print; exit}' "$mosis_file" 2>/dev/null || echo ""
}

# Function to extract DESCRIPTION from MOSIS file (case-insensitive)
extract_description_from_mosis() {
    local mosis_file="$1"
    awk 'BEGIN{IGNORECASE=1} /^[[:space:]]*DESCRIPTION:/ {gsub(/^[[:space:]]*[Dd][Ee][Ss][Cc][Rr][Ii][Pp][Tt][Ii][Oo][Nn]:[[:space:]]*/, ""); gsub(/\r/, ""); gsub(/^[[:space:]]+|[[:space:]]+$/, ""); print; exit}' "$mosis_file" 2>/dev/null || echo ""
}

# Function to extract TECHNOLOGY from MOSIS file (case-insensitive)
extract_technology_from_mosis() {
    local mosis_file="$1"
    awk 'BEGIN{IGNORECASE=1} /^[[:space:]]*TECHNOLOGY:/ {gsub(/^[[:space:]]*[Tt][Ee][Cc][Hh][Nn][Oo][Ll][Oo][Gg][Yy]:[[:space:]]*/, ""); gsub(/\r/, ""); gsub(/^[[:space:]]+|[[:space:]]+$/, ""); print; exit}' "$mosis_file" 2>/dev/null || echo ""
}

# Function to extract PADS count from MOSIS file (case-insensitive)
extract_pads_from_mosis() {
    local mosis_file="$1"
    awk 'BEGIN{IGNORECASE=1} /^[[:space:]]*PADS:/ {gsub(/^[[:space:]]*[Pp][Aa][Dd][Ss]:[[:space:]]*/, ""); gsub(/\r/, ""); gsub(/^[[:space:]]+|[[:space:]]+$/, ""); print; exit}' "$mosis_file" 2>/dev/null || echo ""
}

# Function to extract SIZE (width x height) from MOSIS file (case-insensitive)
extract_size_from_mosis() {
    local mosis_file="$1"
    local size_line=$(awk 'BEGIN{IGNORECASE=1} /^[[:space:]]*SIZE:/ {gsub(/^[[:space:]]*[Ss][Ii][Zz][Ee]:[[:space:]]*/, ""); gsub(/\r/, ""); gsub(/^[[:space:]]+|[[:space:]]+$/, ""); print; exit}' "$mosis_file" 2>/dev/null)
    if [ -n "$size_line" ]; then
        # Parse various formats: "8734 x 8491", "3800 x 4308", "2222 x 2252", "3800 x " (missing height)
        # Extract first number as width
        local width=$(echo "$size_line" | grep -oE '^[[:space:]]*[0-9]+' | head -1)
        # Extract second number (after 'x') as height
        local height=$(echo "$size_line" | sed -E 's/.*[xX][[:space:]]+([0-9]+).*/\1/' | grep -oE '^[0-9]+' | head -1)
        # If no height found, leave empty
        [ -z "$height" ] && height=""
        echo "$width|$height"
    else
        echo "|"
    fi
}

# Function to extract MOSIS ID from REQUEST: FABRICATE section (case-insensitive)
extract_mosis_id() {
    local mosis_file="$1"
    # Look for ID: in REQUEST: FABRICATE section
    # ID can be a number or "*"
    local id=$(awk 'BEGIN{IGNORECASE=1; found=0} /^[[:space:]]*REQUEST:[[:space:]]*FABRICATE/ {found=1; next} found && /^[[:space:]]*ID:/ {gsub(/^[[:space:]]*[Ii][Dd]:[[:space:]]*/, ""); gsub(/\r/, ""); gsub(/^[[:space:]]+|[[:space:]]+$/, ""); print; exit}' "$mosis_file" 2>/dev/null)
    echo "${id:-}"
}

# Function to check if file is a REPORT type (should be skipped) (case-insensitive)
is_report_file() {
    local mosis_file="$1"
    if grep -qiE "^REQUEST:[[:space:]]*REPORT" "$mosis_file"; then
        return 0
    fi
    return 1
}

# Function to check if file is an email/mail file (should be skipped)
is_email_file() {
    local mosis_file="$1"
    # Check for common email headers at the start of the file
    if head -5 "$mosis_file" | grep -qiE "^(From|Return-Path|Received|Message-Id|Date|Subject|To|Cc):"; then
        return 0
    fi
    return 1
}

# Function to extract P-PASSWORD from MOSIS file (fallback when P-NAME missing) (case-insensitive)
extract_ppassword_from_mosis() {
    local mosis_file="$1"
    # Look for P-PASSWORD: in REQUEST: FABRICATE section first
    local ppassword=$(awk 'BEGIN{IGNORECASE=1; found=0} /^[[:space:]]*REQUEST:[[:space:]]*FABRICATE/ {found=1; next} found && /^[[:space:]]*P-PASSWORD:/ {gsub(/^[[:space:]]*[Pp]-[Pp][Aa][Ss][Ss][Ww][Oo][Rr][Dd]:[[:space:]]*/, ""); gsub(/\r/, ""); gsub(/^[[:space:]]+|[[:space:]]+$/, ""); print; exit}' "$mosis_file" 2>/dev/null)
    if [ -z "$ppassword" ]; then
        # Try anywhere in file as fallback
        ppassword=$(awk 'BEGIN{IGNORECASE=1} /^[[:space:]]*P-PASSWORD:/ {gsub(/^[[:space:]]*[Pp]-[Pp][Aa][Ss][Ss][Ww][Oo][Rr][Dd]:[[:space:]]*/, ""); gsub(/\r/, ""); gsub(/^[[:space:]]+|[[:space:]]+$/, ""); print; exit}' "$mosis_file" 2>/dev/null)
    fi
    echo "${ppassword:-}"
}

# Function to detect CAD tool and extract date
detect_cad_tool_and_date() {
    local mosis_file="$1"
    local header=$(head -50 "$mosis_file")
    
    # Check for L-Edit
    if echo "$header" | grep -q "CIF written by the Tanner Research layout editor, L-Edit"; then
        # Extract DATE from L-Edit format: (DATE: 12 Dec 90);
        local date=$(echo "$header" | grep -m1 "(DATE:" | sed -E 's/.*\(DATE:[[:space:]]*([^)]+)\).*/\1/' | sed 's/;$//' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
        echo "L-Edit|$date"
        return
    fi
    
    # Check for WOL
    if echo "$header" | grep -q "WOL CIF"; then
        # Extract date from WOL format: created by ... on May  1 1991 11:28 am
        local wol_line=$(echo "$header" | grep -m1 "WOL CIF")
        local date=$(echo "$wol_line" | sed -E 's/.*[Oo]n ([^;)]+)[;)].*/\1/' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
        echo "WOL|$date"
        return
    fi
    
    # Check for Magic (look for magic-specific patterns)
    if echo "$header" | grep -qi "magic"; then
        local date=$(echo "$header" | grep -m1 -i "date" | sed -E 's/.*[Dd][Aa][Tt][Ee][[:space:]]*:?[[:space:]]*([^;)]+).*/\1/' | sed 's/;$//' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
        echo "Magic|$date"
        return
    fi
    
    # Unknown tool - try to extract any date we can find
    local date=""
    # Try various date patterns
    if echo "$header" | grep -q "(DATE:"; then
        date=$(echo "$header" | grep -m1 "(DATE:" | sed -E 's/.*\(DATE:[[:space:]]*([^)]+)\).*/\1/' | sed 's/;$//' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
    fi
    
    # Try looking for date patterns in comments or other formats
    if [ -z "$date" ]; then
        # Look for date-like patterns in comments: (date: ...) or (Date: ...)
        if echo "$header" | grep -qiE "\([Dd]ate[[:space:]]*:"; then
            date=$(echo "$header" | grep -m1 -iE "\([Dd]ate[[:space:]]*:" | sed -E 's/.*\([Dd]ate[[:space:]]*:[[:space:]]*([^)]+)\).*/\1/' | sed 's/;$//' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
        fi
    fi
    
    if [ -n "$date" ]; then
        echo "Unknown|$date"
    else
        echo "Unknown|"
    fi
}

# Process all .mosis files
total=0
processed=0
skipped=0

while IFS= read -r mosis_file; do
    ((total++))
    
    # Get relative path (remove leading ./)
    rel_path="${mosis_file#./}"
    mosis_filename=$(basename "$mosis_file")
    
    # Extract username from path (first directory component)
    username=$(echo "$rel_path" | cut -d'/' -f1)
    
    # Initialize pname variable (will be set later if needed)
    pname=""
    
    # Special handling for lloyd's chipXXX folders with .cif files
    # If path is like lloyd/chips/chipXXX/chipXXX.cif.gz, extract p-name from folder
    if [[ "$username" == "lloyd" ]] && [[ "$rel_path" =~ ^lloyd/chips/chip([0-9]+)/ ]]; then
        # Extract chip number from folder name (e.g., lloyd/chips/chip024/chip024.cif.gz -> 024)
        chip_num=$(echo "$rel_path" | sed -E 's|lloyd/chips/chip([0-9]+)/.*|\1|')
        # Check if filename matches chipXXX.cif pattern where XXX matches the folder number
        # Remove .gz extension first for comparison
        file_basename_no_ext=$(basename "$mosis_filename" .gz)
        file_basename_no_ext=$(basename "$file_basename_no_ext" .cif)
        # Check if filename is chipXXX where XXX matches the folder number, and it's a .cif file
        if [[ "$file_basename_no_ext" == "chip${chip_num}" ]] && [[ "$mosis_filename" =~ \.cif ]] && [[ ! "$rel_path" =~ /lib/ ]]; then
            # Use the chip number from the folder to construct p-name (preserves leading zeros)
            pname="chip${chip_num}"
        fi
    fi
    
    # Handle compressed files - decompress to temp file if needed
    local_file="$mosis_file"
    temp_file=""
    if [[ "$mosis_file" == *.gz ]]; then
        temp_file=$(mktemp)
        gunzip -c "$mosis_file" > "$temp_file" 2>/dev/null
        if [ $? -eq 0 ] && [ -f "$temp_file" ]; then
            local_file="$temp_file"
        else
            rm -f "$temp_file"
            ((skipped++))
            echo "$rel_path (decompression failed)" >> "$SKIPPED_FILES_LIST"
            continue
        fi
    fi
    
    # For .cif files, check if there's a corresponding .mosis file in the same directory
    # If .mosis exists, skip the .cif file (prefer .mosis)
    if [[ "$mosis_file" == *.cif ]] || [[ "$mosis_file" == *.cif.gz ]]; then
        file_dir=$(dirname "$mosis_file")
        file_basename=$(basename "$mosis_file" .gz)
        file_basename=$(basename "$file_basename" .cif)
        # Check for .mosis file with same base name
        if [ -f "$file_dir/$file_basename.mosis" ] || [ -f "$file_dir/$file_basename.mosis.gz" ]; then
            # Skip this .cif file, .mosis takes precedence
            [ -n "$temp_file" ] && rm -f "$temp_file"
            continue
        fi
    fi
    
    # Skip REPORT files (not chip designs)
    if is_report_file "$local_file"; then
        echo "$rel_path" >> "$REPORT_FILES_LIST"
        [ -n "$temp_file" ] && rm -f "$temp_file"
        continue
    fi
    
    # Skip email/mail files (MOSIS fabrication progress emails)
    if is_email_file "$local_file"; then
        echo "$rel_path" >> "$EMAIL_FILES_LIST"
        [ -n "$temp_file" ] && rm -f "$temp_file"
        continue
    fi
    
    # Extract P-NAME (unless already set for lloyd's chipXXX)
    if [ -z "$pname" ]; then
        pname=$(extract_pname_from_mosis "$local_file")
    fi
    
    # If no P-NAME, try P-PASSWORD as fallback (for REQUEST: FABRICATE files)
    if [ -z "$pname" ]; then
        # Check if this is a FABRICATE request file (case-insensitive)
        if grep -qiE "^REQUEST:[[:space:]]*FABRICATE" "$local_file"; then
            pname=$(extract_ppassword_from_mosis "$local_file")
        fi
    fi
    
    # For .cif files without MOSIS header, try to extract from filename or folder
    if [ -z "$pname" ] && [[ "$mosis_file" == *.cif* ]]; then
        # Try to extract from filename (e.g., chip024.cif.gz -> chip024)
        file_basename=$(basename "$mosis_file" .gz)
        file_basename=$(basename "$file_basename" .cif)
        # If filename matches pattern chipXXX, use it
        if [[ "$file_basename" =~ ^chip[0-9]+$ ]]; then
            pname="$file_basename"
        fi
    fi
    
    # For .cif files, many fields may not be available (no MOSIS header)
    # Set defaults for missing fields
    if [[ "$mosis_file" == *.cif* ]] && [ -z "$pname" ]; then
        # If we still don't have a pname for .cif file, skip it
        ((skipped++))
        echo "$rel_path (CIF file, no P-NAME found)" >> "$SKIPPED_FILES_LIST"
        [ -n "$temp_file" ] && rm -f "$temp_file"
        continue
    fi
    
    if [ -z "$pname" ]; then
        ((skipped++))
        echo "$rel_path" >> "$SKIPPED_FILES_LIST"
        [ -n "$temp_file" ] && rm -f "$temp_file"
        continue
    fi
    
    # Extract all fields
    description=$(extract_description_from_mosis "$local_file")
    technology=$(extract_technology_from_mosis "$local_file")
    pads=$(extract_pads_from_mosis "$local_file")
    size_info=$(extract_size_from_mosis "$local_file")
    size_width=$(echo "$size_info" | cut -d'|' -f1)
    size_height=$(echo "$size_info" | cut -d'|' -f2)
    
    # Extract MOSIS ID
    mosis_id=$(extract_mosis_id "$local_file")
    
    # Detect CAD tool and extract date
    tool_and_date=$(detect_cad_tool_and_date "$local_file")
    cad_tool=$(echo "$tool_and_date" | cut -d'|' -f1)
    cif_date=$(echo "$tool_and_date" | cut -d'|' -f2)
    
    # Get archive date (original file date from archive)
    archive_date=""
    if [ -f "$ARCHIVE_DATE_MAP" ]; then
        # Look up in archive map - try multiple path variations
        # Archive paths have "pcmp home/" prefix, our paths don't
        archive_path="pcmp home/$rel_path"
        # Use awk for exact match at start of line (handles spaces correctly)
        archive_date_line=$(awk -v path="$archive_path" -F'|' '$1 == path {print; exit}' "$ARCHIVE_DATE_MAP")
        
        if [ -z "$archive_date_line" ]; then
            # Try with .gz extension if original doesn't match
            archive_path_gz="pcmp home/${rel_path}.gz"
            archive_date_line=$(awk -v path="$archive_path_gz" -F'|' '$1 == path {print; exit}' "$ARCHIVE_DATE_MAP")
        fi
        
        if [ -z "$archive_date_line" ]; then
            # Try without .gz extension if we have .gz
            if [[ "$rel_path" == *.gz ]]; then
                archive_path_no_gz="pcmp home/${rel_path%.gz}"
                archive_date_line=$(awk -v path="$archive_path_no_gz" -F'|' '$1 == path {print; exit}' "$ARCHIVE_DATE_MAP")
            fi
        fi
        
        if [ -z "$archive_date_line" ]; then
            # Try searching by filename only (files might be in different directories)
            # This is a fallback for files that were moved/reorganized
            filename_only=$(basename "$rel_path")
            # Match path ending with /filename before the |
            archive_date_line=$(awk -v fname="$filename_only" -F'|' '$1 ~ "/" fname "$" {print; exit}' "$ARCHIVE_DATE_MAP")
            if [ -z "$archive_date_line" ]; then
                # Try with .gz added/removed
                if [[ "$filename_only" == *.gz ]]; then
                    filename_no_gz="${filename_only%.gz}"
                    archive_date_line=$(awk -v fname="$filename_no_gz" -F'|' '$1 ~ "/" fname "$" {print; exit}' "$ARCHIVE_DATE_MAP")
                else
                    archive_date_line=$(awk -v fname="${filename_only}.gz" -F'|' '$1 ~ "/" fname "$" {print; exit}' "$ARCHIVE_DATE_MAP")
                fi
            fi
        fi
        
        if [ -n "$archive_date_line" ]; then
            # Extract date part (everything after |)
            # Format is: path|YYYY-MM-DD HH:MM
            # Extract just the date part (YYYY-MM-DD)
            archive_date=$(echo "$archive_date_line" | cut -d'|' -f2 | awk '{print $1}')
        fi
    fi
    
    # Determine primary date and date source
    # Primary date: CIF date (authoritative) if available, otherwise archive date
    primary_date=""
    date_source=""
    if [ -n "$cif_date" ]; then
        primary_date="$cif_date"
        date_source="CIF"
    elif [ -n "$archive_date" ]; then
        primary_date="$archive_date"
        date_source="archive"
    else
        # No date available - should not happen, but handle gracefully
        primary_date=""
        date_source="none"
    fi
    
    # Normalize primary date to YYYY-MM-DD for sorting
    normalized_date=$(normalize_date "$primary_date")
    
    # Clean up temp file if used
    [ -n "$temp_file" ] && rm -f "$temp_file"
    
    # Escape CSV fields and write to CSV
    mosis_file_escaped=$(escape_csv "$mosis_filename")
    path_escaped=$(escape_csv "$rel_path")
    username_escaped=$(escape_csv "$username")
    pname_escaped=$(escape_csv "$pname")
    technology_escaped=$(escape_csv "$technology")
    pads_escaped=$(escape_csv "$pads")
    size_width_escaped=$(escape_csv "$size_width")
    size_height_escaped=$(escape_csv "$size_height")
    description_escaped=$(escape_csv "$description")
    cad_tool_escaped=$(escape_csv "$cad_tool")
    cif_date_escaped=$(escape_csv "$cif_date")
    archive_date_escaped=$(escape_csv "$archive_date")
    primary_date_escaped=$(escape_csv "$primary_date")
    date_source_escaped=$(escape_csv "$date_source")
    normalized_date_escaped=$(escape_csv "$normalized_date")
    mosis_id_escaped=$(escape_csv "$mosis_id")
    
    echo "$mosis_file_escaped,$path_escaped,$username_escaped,$pname_escaped,$technology_escaped,$pads_escaped,$size_width_escaped,$size_height_escaped,$description_escaped,$cad_tool_escaped,$cif_date_escaped,$archive_date_escaped,$primary_date_escaped,$date_source_escaped,$normalized_date_escaped,$mosis_id_escaped" >> "$CSV_OUTPUT"
    
    ((processed++))
    
    if [ $((processed % 100)) -eq 0 ]; then
        echo "Processed $processed files..."
    fi
done < <(find . \( -name "*.mosis" -o -name "*.mosis.gz" -o -name "*.cif" -o -name "*.cif.gz" \) -type f)

# Count report files
report_count=0
if [ -f "$REPORT_FILES_LIST" ]; then
    report_count=$(wc -l < "$REPORT_FILES_LIST" | tr -d ' ')
fi

# Count email files
email_count=0
if [ -f "$EMAIL_FILES_LIST" ]; then
    email_count=$(wc -l < "$EMAIL_FILES_LIST" | tr -d ' ')
fi

echo ""
echo "Summary:"
echo "========"
echo "Total .mosis and .cif files: $total"
echo "Files processed: $processed"
echo "Files skipped (no P-NAME): $skipped"
echo "Report files skipped: $report_count"

# Count email files
email_count=0
if [ -f "$EMAIL_FILES_LIST" ]; then
    email_count=$(wc -l < "$EMAIL_FILES_LIST" | tr -d ' ')
fi
echo "Email files skipped: $email_count"
echo "Email files skipped: $email_count"
if [ $skipped -gt 0 ] && [ -f "$SKIPPED_FILES_LIST" ]; then
    echo ""
    echo "Skipped files list saved to: $OUTPUT_DIR/skipped_files_no_pname.txt"
    mv "$SKIPPED_FILES_LIST" "$OUTPUT_DIR/skipped_files_no_pname.txt"
    echo "  (First 20 files:)"
    head -20 "$OUTPUT_DIR/skipped_files_no_pname.txt" | sed 's/^/    /'
    if [ $skipped -gt 20 ]; then
        echo "    ... and $((skipped - 20)) more (see file for complete list)"
    fi
else
    rm -f "$SKIPPED_FILES_LIST"
fi
if [ $report_count -gt 0 ] && [ -f "$REPORT_FILES_LIST" ]; then
    echo ""
    echo "Report files list saved to: $OUTPUT_DIR/report_files.txt"
    mv "$REPORT_FILES_LIST" "$OUTPUT_DIR/report_files.txt"
    echo "  (First 20 files:)"
    head -20 "$OUTPUT_DIR/report_files.txt" | sed 's/^/    /'
    if [ $report_count -gt 20 ]; then
        echo "    ... and $((report_count - 20)) more (see file for complete list)"
    fi
else
    rm -f "$REPORT_FILES_LIST"
fi
if [ $email_count -gt 0 ] && [ -f "$EMAIL_FILES_LIST" ]; then
    echo ""
    echo "Email files list saved to: $OUTPUT_DIR/email_files.txt"
    mv "$EMAIL_FILES_LIST" "$OUTPUT_DIR/email_files.txt"
    echo "  (First 20 files:)"
    head -20 "$OUTPUT_DIR/email_files.txt" | sed 's/^/    /'
    if [ $email_count -gt 20 ]; then
        echo "    ... and $((email_count - 20)) more (see file for complete list)"
    fi
else
    rm -f "$EMAIL_FILES_LIST"
fi
echo ""
echo "CSV database created: $CSV_OUTPUT"
echo "  - Total entries: $(($(wc -l < "$CSV_OUTPUT") - 1))"
echo ""
echo "CAD tool distribution:"
tail -n +2 "$CSV_OUTPUT" | cut -d',' -f9 | sort | uniq -c | sort -rn
echo ""
echo "Date source distribution:"
python3 -c "import csv; f=open('$CSV_OUTPUT'); r=csv.reader(f); rows=list(r); sources={}; [sources.update({row[12]: sources.get(row[12],0)+1}) for row in rows[1:]]; [print(f'  {k}: {v}') for k,v in sorted(sources.items())]"

echo ""
echo "Generated files:"
echo "================"
echo "$(realpath "$CSV_OUTPUT")"
if [ -f "$OUTPUT_DIR/skipped_files_no_pname.txt" ]; then
    echo "$(realpath "$OUTPUT_DIR/skipped_files_no_pname.txt")"
fi
if [ -f "$OUTPUT_DIR/report_files.txt" ]; then
    echo "$(realpath "$OUTPUT_DIR/report_files.txt")"
fi
if [ -f "$OUTPUT_DIR/email_files.txt" ]; then
    echo "$(realpath "$OUTPUT_DIR/email_files.txt")"
fi

# Clean up temporary file
rm -f "$ARCHIVE_DATE_MAP"
