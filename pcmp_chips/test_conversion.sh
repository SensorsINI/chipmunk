#!/bin/bash

# Test conversion with small sample
# Usage examples:
#   ./test_conversion.sh                    # Convert 5 files at 800x600
#   IMAGE_WIDTH=1920 IMAGE_HEIGHT=1080 FILE_LIMIT=10 ./test_conversion.sh

export IMAGE_WIDTH="${IMAGE_WIDTH:-800}"
export IMAGE_HEIGHT="${IMAGE_HEIGHT:-600}"
export FILE_LIMIT="${FILE_LIMIT:-5}"

echo "Test Conversion Settings:"
echo "========================="
echo "Image size: ${IMAGE_WIDTH}x${IMAGE_HEIGHT}"
echo "File limit: ${FILE_LIMIT} files"
echo ""

./convert_layouts_to_images.sh
