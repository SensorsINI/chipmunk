#!/bin/bash

# PDF Inspector - iterate through PDFs one at a time
# Usage: ./inspect_pdfs.sh [pdf_list_file]

PDF_LIST="${1:-/home/tobi/chipmunk/pcmp_chips/converted_pdfs.txt}"

# Check if PDF list exists
if [ ! -f "$PDF_LIST" ]; then
    echo "Error: PDF list file not found: $PDF_LIST"
    echo "Usage: $0 [pdf_list_file]"
    exit 1
fi

# Check if PDF viewer is available
if command -v evince >/dev/null 2>&1; then
    VIEWER="evince"
elif command -v okular >/dev/null 2>&1; then
    VIEWER="okular"
elif command -v xdg-open >/dev/null 2>&1; then
    VIEWER="xdg-open"
else
    echo "Error: No PDF viewer found (tried: evince, okular, xdg-open)"
    exit 1
fi

echo "PDF Inspector"
echo "============="
echo "Using viewer: $VIEWER"
echo "PDF list: $PDF_LIST"
echo ""
echo "Controls:"
echo "  - Close the PDF viewer to see the next PDF"
echo "  - Press Ctrl+C to abort"
echo ""

# Count total PDFs
TOTAL=$(wc -l < "$PDF_LIST")
CURRENT=0

# Read and process each PDF
while IFS= read -r pdf_file; do
    CURRENT=$((CURRENT + 1))
    
    # Check if file exists
    if [ ! -f "$pdf_file" ]; then
        echo "[$CURRENT/$TOTAL] SKIP (not found): $pdf_file"
        continue
    fi
    
    echo ""
    echo "[$CURRENT/$TOTAL] Opening: $pdf_file"
    echo "---"
    
    # Open PDF viewer and wait for it to close
    $VIEWER "$pdf_file" 2>/dev/null
    
    # Check exit status
    if [ $? -ne 0 ]; then
        echo "Warning: Viewer exited with error"
    fi
    
done < "$PDF_LIST"

echo ""
echo "Finished reviewing all PDFs!"
