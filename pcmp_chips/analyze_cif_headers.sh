#!/bin/bash

# Script to analyze CIF headers in .mosis files
# Counts files with and without DATE, and reports header type variety

cd ~/pcmp_chips || exit 1

echo "Analyzing CIF headers in .mosis files..."
echo "=========================================="
echo ""

# Count total .mosis files
total_files=$(find . -name "*.mosis" -type f | wc -l)
echo "Total .mosis files: $total_files"
echo ""

# Count files with CIF: marker
files_with_cif=$(find . -name "*.mosis" -type f -exec grep -l "^CIF:" {} \; | wc -l)
echo "Files with 'CIF:' marker: $files_with_cif"
echo ""

# Count files with DATE in header (first 50 lines)
files_with_date=0
files_without_date=0
declare -A header_types

while IFS= read -r file; do
    if head -50 "$file" | grep -q "^CIF:"; then
        if head -50 "$file" | grep -q "(DATE:"; then
            ((files_with_date++))
        else
            ((files_without_date++))
        fi
    fi
done < <(find . -name "*.mosis" -type f)

echo "Files with CIF: and (DATE: in header: $files_with_date"
echo "Files with CIF: but NO (DATE: in header: $files_without_date"
echo ""

# Analyze header types
echo "Analyzing header type variety..."
echo "================================="
echo ""

# Extract header information from files with CIF:
echo "Header type patterns found:"
echo ""

# Count L-Edit headers
ledit_count=$(find . -name "*.mosis" -type f -exec sh -c 'head -50 "$1" | grep -q "^CIF:" && head -50 "$1" | grep -q "L-Edit"' _ {} \; | wc -l)
echo "Files with L-Edit headers: $ledit_count"

# Count different L-Edit version patterns
echo ""
echo "L-Edit header variations:"
find . -name "*.mosis" -type f -exec sh -c '
    if head -50 "$1" | grep -q "^CIF:"; then
        header=$(head -50 "$1" | sed -n "/^CIF:/,/^DS/p" | head -10 | grep "L-Edit" | head -1)
        if [ -n "$header" ]; then
            echo "$header"
        fi
    fi
' _ {} \; | sort | uniq -c | sort -rn | head -10

echo ""
echo "Sample headers WITHOUT DATE:"
echo "-----------------------------"
count=0
while IFS= read -r file && [ $count -lt 5 ]; do
    if head -50 "$file" | grep -q "^CIF:" && ! head -50 "$file" | grep -q "(DATE:"; then
        echo "File: $file"
        head -50 "$file" | sed -n "/^CIF:/,/^DS/p" | head -8
        echo ""
        ((count++))
    fi
done < <(find . -name "*.mosis" -type f)

echo ""
echo "Sample headers WITH DATE:"
echo "--------------------------"
count=0
while IFS= read -r file && [ $count -lt 5 ]; do
    if head -50 "$file" | grep -q "^CIF:" && head -50 "$file" | grep -q "(DATE:"; then
        echo "File: $file"
        head -50 "$file" | sed -n "/^CIF:/,/^DS/p" | head -8
        echo ""
        ((count++))
    fi
done < <(find . -name "*.mosis" -type f)

