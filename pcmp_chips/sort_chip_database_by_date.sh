#!/bin/bash

# Script to sort chip_database.csv by normalized_date (earliest to latest)
# Creates a new sorted CSV file: chip_database_sorted.csv

CHIP_DIR="${CHIP_DIR:-$HOME/pcmp_home}"
CSV_DATABASE="${CSV_DATABASE:-$CHIP_DIR/chip_database.csv}"
CSV_SORTED="${CSV_SORTED:-$CHIP_DIR/chip_database_sorted.csv}"

# Check if CSV database exists
if [ ! -f "$CSV_DATABASE" ]; then
    echo "Error: CSV database not found: $CSV_DATABASE"
    echo "Run generate_chip_database.sh first to create the database"
    exit 1
fi

echo "Sorting chip database by normalized_date..."
echo "Input:  $CSV_DATABASE"
echo "Output: $CSV_SORTED"
echo ""

# Use Python to safely parse CSV (handles quoted fields with commas)
# Export variables for Python to use
export CSV_DATABASE CSV_SORTED
python3 <<'PYTHON_EOF'
import csv
import sys
import os

input_file = os.environ['CSV_DATABASE']
output_file = os.environ['CSV_SORTED']

# Read all rows
rows = []
header = None

with open(input_file, 'r', encoding='utf-8') as f:
    reader = csv.reader(f)
    header = next(reader)  # Read header
    
    for row in reader:
        # Ensure row has enough columns
        if len(row) >= 16:
            rows.append(row)
        else:
            # Skip malformed rows
            print(f"Warning: Skipping row with {len(row)} columns (expected 16)", file=sys.stderr)

# Sort by normalized_date (column index 14)
# Handle empty dates by placing them at the end (use '9999-99-99' for sorting)
def sort_key(row):
    normalized_date = row[14].strip() if len(row) > 14 else ''
    # Empty dates sort last
    if not normalized_date:
        return '9999-99-99'
    # Invalid dates sort last
    if normalized_date == '?' or normalized_date == 'Unknown':
        return '9999-99-99'
    return normalized_date

rows.sort(key=sort_key)

# Write sorted CSV
with open(output_file, 'w', encoding='utf-8', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(header)  # Write header first
    writer.writerows(rows)

# Count rows with dates
rows_with_dates = sum(1 for row in rows if row[14].strip() and row[14].strip() not in ['?', 'Unknown'])
rows_without_dates = len(rows) - rows_with_dates

print(f"Sorted {len(rows)} rows")
if rows_with_dates > 0:
    first_date = rows[0][14].strip() if rows[0][14].strip() else 'None'
    last_date_row = next((r for r in reversed(rows) if r[14].strip() and r[14].strip() not in ['?', 'Unknown']), None)
    last_date = last_date_row[14].strip() if last_date_row else 'None'
    print(f"Date range: {first_date} to {last_date}")
if rows_without_dates > 0:
    print(f"Rows without dates: {rows_without_dates} (sorted to end)")
print(f"Output written to: {output_file}")
PYTHON_EOF

# Verify output file was created
if [ -f "$CSV_SORTED" ]; then
    row_count=$(tail -n +2 "$CSV_SORTED" | wc -l)
    echo ""
    echo "Successfully created sorted CSV with $row_count data rows"
else
    echo "Error: Failed to create sorted CSV file"
    exit 1
fi

