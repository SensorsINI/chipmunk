# Dub Chip Movie Setup Guide

## Summary

✅ **This is possible with CLI tools!**

I've created `create_dub_chip_movie.sh` which:
- Detects beats in the audio using `aubio`
- Creates variable frame rate video synchronized to beats
- Adds pauses every 4th beat
- Fades audio out over 1 second at the end
- Creates a ~45 second video

## Required Installation

**One package needs to be installed:**

```bash
sudo apt install aubio-tools
```

This provides the `aubioonset` command for beat detection.

## Already Available Tools

✅ **ffmpeg** - Installed (4.4.2)  
✅ **ffprobe** - Installed  
✅ **ImageMagick (convert)** - Installed  
✅ **bc** - Installed (for calculations)  
✅ **Python3** - Installed  

## How It Works

1. **Beat Detection**: Uses `aubioonset` to detect beats in the audio segment
2. **Timing Map**: Python script maps beats to frame display times
   - Fast transitions on beats (0.15s per frame)
   - Slower between beats (0.3-0.5s per frame)
   - Pauses every 4th beat (1.2s)
3. **Video Segments**: Creates individual video segments for each frame with variable duration
4. **Concatenation**: Combines segments into final video
5. **Audio**: Adds audio with 1-second fade out at the end

## Usage

```bash
# Basic usage (uses first 45 seconds of audio)
./create_dub_chip_movie.sh dub_chips.mp4

# Specify audio start time and duration
./create_dub_chip_movie.sh dub_chips.mp4 30 45  # Start at 30s, 45s duration

# Test with limited images
FILE_LIMIT=50 ./create_dub_chip_movie.sh test.mp4
```

## Parameters

- **Audio start**: Which part of the 183.5s audio to use (default: 0)
- **Video duration**: Target video length (default: 45s)
- **FILE_LIMIT**: Limit number of images (for testing)

## Technical Details

### Beat Detection
- Uses `aubioonset` with onset detection threshold of 0.3
- Falls back to uniform timing if no beats detected

### Frame Timing Strategy
- **On beats**: Fast transition (0.15s)
- **Between beats**: Slower (0.3-0.5s)
- **Every 4th beat**: Pause (1.2s, same frame repeated)

### Audio Fade
- Fade out starts 1 second before end
- Duration: 1.0 second
- Uses ffmpeg's `afade` filter

## Testing

Before running on all images, test with a small limit:

```bash
FILE_LIMIT=20 ./create_dub_chip_movie.sh test_dub.mp4
```

This will:
- Use first 20 images
- Create a short test video
- Verify beat detection and timing work correctly

## Troubleshooting

**No beats detected?**
- Script falls back to uniform timing
- Try adjusting aubio threshold (edit script, change `-t 0.3`)

**Video too fast/slow?**
- Adjust timing multipliers in Python script section
- Change `0.15` (beat speed) or `0.3` (between beats)

**Audio sync issues?**
- Ensure video duration matches audio segment duration
- Check that `-shortest` flag is working in ffmpeg

## Next Steps

1. Install aubio-tools: `sudo apt install aubio-tools`
2. Test with small FILE_LIMIT
3. Adjust timing if needed
4. Run full conversion

