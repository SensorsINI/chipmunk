#!/bin/bash

# Output files
OUTPUT_DIR="/home/tobi/pcmp_chips"
PDF_LIST="$OUTPUT_DIR/converted_pdfs.txt"
ERROR_LOG="$OUTPUT_DIR/conversion_errors.log"

# Clear/create output files
> "$PDF_LIST"
> "$ERROR_LOG"

echo "Starting PS to PDF conversion at $(date)" | tee -a "$ERROR_LOG"

# Find all .ps files in chip-related directories
find "$OUTPUT_DIR" -type f -name "*.ps" | while read ps_file; do
    # Get the directory and filename
    dir=$(dirname "$ps_file")
    base=$(basename "$ps_file" .ps)
    pdf_file="$dir/$base.pdf"
    
    # Skip if PDF already exists
    if [ -f "$pdf_file" ]; then
        echo "$pdf_file" >> "$PDF_LIST"
        continue
    fi
    
    # Try to convert with ps2pdf (ghostscript)
    if ps2pdf -dPDFSETTINGS=/ebook -dCompressPages=true -dUseFlateCompression=true "$ps_file" "$pdf_file" 2>/dev/null; then
        echo "$pdf_file" >> "$PDF_LIST"
        echo "Converted: $ps_file -> $pdf_file"
    else
        echo "Error converting: $ps_file" >> "$ERROR_LOG"
    fi
done

echo "Conversion complete at $(date)" | tee -a "$ERROR_LOG"
echo "Total PDFs created: $(wc -l < "$PDF_LIST")" | tee -a "$ERROR_LOG"
