#!/usr/bin/env python3
"""
Plot chip image timing vs beat times to verify alignment.

Usage:
    python3 plot_chip_beat_alignment.py <timing_data_file>
    python3 plot_chip_beat_alignment.py <video_file>_timing_data.txt

The timing data file should contain:
- Segment timing: segment_index|chip_num|chip_year|start_time|end_time|duration|frame_count
- Beat times: one per line
"""

import sys
import matplotlib.pyplot as plt
import numpy as np

def parse_timing_data(filename):
    """Parse timing data file."""
    segments = []
    beats = []
    
    with open(filename, 'r') as f:
        in_beats = False
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                if 'Beat times' in line:
                    in_beats = True
                continue
            
            if in_beats:
                try:
                    beat_time = float(line)
                    beats.append(beat_time)
                except ValueError:
                    continue
            else:
                # Parse segment line: segment_index|chip_num|chip_year|start_time|end_time|duration|frame_count
                parts = line.split('|')
                if len(parts) >= 5:
                    try:
                        segment_idx = int(parts[0])
                        chip_num = int(parts[1])
                        start_time = float(parts[3])
                        end_time = float(parts[4])
                        segments.append((segment_idx, chip_num, start_time, end_time))
                    except (ValueError, IndexError):
                        continue
    
    return segments, beats

def plot_alignment(segments, beats, output_file=None):
    """Plot chip timing vs beat times."""
    if not segments:
        print("Error: No segments found in timing data")
        return
    
    if not beats:
        print("Warning: No beats found in timing data")
    
    fig, ax = plt.subplots(figsize=(14, 8))
    
    # Plot chip segments as horizontal bars (stepped)
    chip_nums = [s[1] for s in segments]
    min_chip = min(chip_nums)
    max_chip = max(chip_nums)
    chip_range = max_chip - min_chip + 1
    
    # Plot each segment as a horizontal line
    for segment_idx, chip_num, start_time, end_time in segments:
        # Normalize chip number to y-axis (0 to chip_range)
        y_pos = chip_num - min_chip
        ax.plot([start_time, end_time], [y_pos, y_pos], 
                linewidth=2, alpha=0.7, color='steelblue')
        # Add small vertical line at start to show step
        if segment_idx > 0:
            prev_end = segments[segment_idx - 1][3]
            if abs(start_time - prev_end) > 0.001:  # Gap detected
                ax.plot([prev_end, start_time], [y_pos - 1, y_pos], 
                       'k--', linewidth=1, alpha=0.3)
    
    # Plot beat times as vertical lines
    if beats:
        for beat_time in beats:
            ax.axvline(x=beat_time, color='red', linestyle='--', 
                      linewidth=1.5, alpha=0.6, label='Beat' if beat_time == beats[0] else '')
    
    # Formatting
    ax.set_xlabel('Time (seconds)', fontsize=12)
    ax.set_ylabel('Chip Number', fontsize=12)
    ax.set_title('Chip Image Timing vs Beat Times\n(Blue: chip display periods, Red: beat times)', 
                 fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend(['Chip segments', 'Beats'], loc='upper right')
    
    # Set y-axis to show chip numbers
    ax.set_yticks(range(chip_range))
    ax.set_yticklabels([str(min_chip + i) for i in range(chip_range)])
    
    # Add text annotation showing alignment
    if beats and segments:
        # Check alignment: beats should align with segment boundaries
        misalignments = []
        for beat in beats:
            # Find closest segment boundary
            closest_start = min(segments, key=lambda s: abs(s[2] - beat))
            closest_end = min(segments, key=lambda s: abs(s[3] - beat))
            dist_start = abs(closest_start[2] - beat)
            dist_end = abs(closest_end[3] - beat)
            min_dist = min(dist_start, dist_end)
            if min_dist > 0.05:  # More than 50ms misalignment
                misalignments.append((beat, min_dist))
        
        if misalignments:
            text = f"Warning: {len(misalignments)} beats misaligned >50ms"
            ax.text(0.02, 0.98, text, transform=ax.transAxes, 
                   fontsize=10, verticalalignment='top',
                   bbox=dict(boxstyle='round', facecolor='yellow', alpha=0.5))
        else:
            text = "Alignment: Good (all beats within 50ms of segment boundaries)"
            ax.text(0.02, 0.98, text, transform=ax.transAxes, 
                   fontsize=10, verticalalignment='top',
                   bbox=dict(boxstyle='round', facecolor='lightgreen', alpha=0.5))
    
    plt.tight_layout()
    
    if output_file:
        plt.savefig(output_file, dpi=150, bbox_inches='tight')
        print(f"Plot saved to: {output_file}")
    else:
        plt.show()

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 plot_chip_beat_alignment.py <timing_data_file>")
        print("   or: python3 plot_chip_beat_alignment.py <video_file>_timing_data.txt")
        sys.exit(1)
    
    timing_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else timing_file.replace('.txt', '_alignment.png')
    
    try:
        segments, beats = parse_timing_data(timing_file)
        print(f"Loaded {len(segments)} segments and {len(beats)} beats")
        plot_alignment(segments, beats, output_file)
    except FileNotFoundError:
        print(f"Error: Timing data file not found: {timing_file}")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()
