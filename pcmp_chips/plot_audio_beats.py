#!/usr/bin/env python3
"""
Plot audio envelope with beat ticks for the first 10 seconds
"""

import sys
import subprocess
import tempfile
import os
import shutil
import argparse
import numpy as np
import matplotlib.pyplot as plt

def extract_beats(audio_file, method='energy'):
    """Extract beat times using aubioonset"""
    beats = []
    try:
        result = subprocess.run(
            ['aubioonset', '-i', audio_file, '-t', '0.3', '-O', method], # extract beats
            capture_output=True,
            text=True,
            check=True
        )
        for line in result.stdout.strip().split('\n'):
            if line.strip():
                try:
                    beat_time = float(line.strip())
                    beats.append(beat_time)
                except ValueError:
                    continue
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"Error extracting beats: {e}", file=sys.stderr)
        sys.exit(1)
    return beats

def load_audio_envelope(audio_file, duration=10.0):
    """Load audio envelope using ffmpeg and numpy"""
    # Find ffmpeg executable
    ffmpeg_path = shutil.which('ffmpeg')
    if not ffmpeg_path:
        print("Error: ffmpeg not found in PATH", file=sys.stderr)
        print("Install with: sudo apt install ffmpeg", file=sys.stderr)
        sys.exit(1)
    
    # Create temporary wav file
    with tempfile.NamedTemporaryFile(suffix='.wav', delete=False) as tmp_wav:
        tmp_wav_path = tmp_wav.name
    
    try:
        # Convert audio to WAV format with ffmpeg (first 'duration' seconds)
        subprocess.run(
            [ffmpeg_path, '-y', '-i', audio_file, '-t', str(duration),
             '-ar', '44100', '-ac', '1', '-f', 'wav', tmp_wav_path],
            capture_output=True,
            check=True
        )
        
        # Read WAV file using scipy.io.wavfile or wave
        try:
            from scipy.io import wavfile
            sample_rate, audio_data = wavfile.read(tmp_wav_path)
        except (ImportError, AttributeError, ModuleNotFoundError):
            # Fallback to wave module if scipy is not available or has issues
            import wave
            import struct
            with wave.open(tmp_wav_path, 'rb') as wav_file:
                sample_rate = wav_file.getframerate()
                n_frames = wav_file.getnframes()
                audio_bytes = wav_file.readframes(n_frames)
                audio_data = np.frombuffer(audio_bytes, dtype=np.int16)
                # Convert to mono if stereo
                if wav_file.getnchannels() == 2:
                    audio_data = audio_data.reshape(-1, 2).mean(axis=1)
        
        # Normalize to float [-1, 1]
        if audio_data.dtype == np.int16:
            audio_data = audio_data.astype(np.float32) / 32768.0
        elif audio_data.dtype == np.int32:
            audio_data = audio_data.astype(np.float32) / 2147483648.0
        
        # Create time axis
        time_axis = np.arange(len(audio_data)) / sample_rate
        
        # Calculate envelope (RMS energy in windows)
        window_size = int(sample_rate * 0.01)  # 10ms windows
        envelope = []
        envelope_times = []
        for i in range(0, len(audio_data), window_size // 4):  # 75% overlap
            window = audio_data[i:i+window_size]
            if len(window) > 0:
                rms = np.sqrt(np.mean(window**2))
                envelope.append(rms)
                envelope_times.append(i / sample_rate)
        
        return np.array(envelope_times), np.array(envelope)
    
    finally:
        # Clean up temporary file
        if os.path.exists(tmp_wav_path):
            os.unlink(tmp_wav_path)

def plot_audio_beats(audio_file, max_duration=10.0, method='energy', show=False):
    """Plot audio envelope with beat ticks"""
    print(f"Extracting beats from {audio_file}...")
    beats = extract_beats(audio_file, method)
    
    # Filter beats within duration
    beats_in_range = [b for b in beats if b <= max_duration]
    print(f"Found {len(beats_in_range)} beats in first {max_duration}s")
    
    print(f"Loading audio envelope for first {max_duration}s...")
    time_axis, envelope = load_audio_envelope(audio_file, max_duration)
    
    # Create plot
    fig, ax = plt.subplots(figsize=(14, 6))
    
    # Plot envelope
    ax.plot(time_axis, envelope, 'b-', linewidth=0.5, alpha=0.7, label='Audio Envelope')
    ax.fill_between(time_axis, 0, envelope, alpha=0.3, color='blue')
    
    # Plot beat ticks below the envelope
    if beats_in_range:
        # Set y position for ticks (below the plot area)
        y_min, y_max = ax.get_ylim()
        tick_y = y_min - (y_max - y_min) * 0.1
        ax.set_ylim(y_min - (y_max - y_min) * 0.15, y_max)
        
        # Draw vertical lines for each beat
        for beat_time in beats_in_range:
            ax.axvline(x=beat_time, color='red', linestyle='--', linewidth=1.5, alpha=0.7)
        
        # Add ticks at bottom
        ax.scatter(beats_in_range, [tick_y] * len(beats_in_range), 
                  marker='|', s=200, color='red', linewidth=2, label='Beats')
        
        # Label first few beats
        for i, beat_time in enumerate(beats_in_range[:10]):
            ax.text(beat_time, tick_y - (y_max - y_min) * 0.05, 
                   f'{beat_time:.3f}s', ha='center', va='top', 
                   fontsize=8, rotation=90, color='red')
    
    ax.set_xlabel('Time (seconds)', fontsize=12)
    ax.set_ylabel('Audio Envelope (RMS)', fontsize=12)
    ax.set_title(f'Audio Envelope with Beat Detections (First {max_duration}s)', fontsize=14)
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper right')
    ax.set_xlim(0, max_duration)
    
    plt.tight_layout()
    
    # Save plot
    output_file = '/tmp/audio_beats_plot.png'
    try:
        plt.savefig(output_file, dpi=150, bbox_inches='tight')
        print(f"Plot saved to {os.path.abspath(output_file)}")
    except Exception as e:
        print(f"Error saving plot: {e}", file=sys.stderr)
        sys.exit(1)
    
    # Only show window if explicitly requested
    if show:
        try:
            plt.show()
        except Exception:
            pass  # Display not available, that's fine
    
    plt.close()  # Close figure to free memory (prevents window from staying open)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Plot audio envelope with beat ticks using aubioonset',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Available onset detection methods:
  default    - Energy-based method (default)
  energy     - Uses energy to detect onsets
  hfc        - High-Frequency Content method
  complex    - Complex domain method
  phase      - Phase-based method
  specdiff   - Spectral difference method
  kl         - Kullback-Liebler method
  mkl        - Modified Kullback-Liebler method
  specflux   - Spectral flux method

Examples:
  %(prog)s audio.m4a
  %(prog)s audio.m4a 15.0
  %(prog)s audio.m4a 10.0 --method hfc
        """
    )
    parser.add_argument(
        'audio_file',
        nargs='?',
        default=os.path.expanduser('~/pcmp_home/04 Reaching Dub.m4a'),
        help='Path to audio file (default: ~/pcmp_home/04 Reaching Dub.m4a)'
    )
    parser.add_argument(
        'max_duration',
        type=float,
        nargs='?',
        default=10.0,
        help='Maximum duration in seconds to plot (default: 10.0)'
    )
    parser.add_argument(
        '--method',
        type=str,
        default='energy',
        choices=['default', 'energy', 'hfc', 'complex', 'phase', 'specdiff', 'kl', 'mkl', 'specflux'],
        help='Onset detection method to use (default: energy)'
    )
    parser.add_argument(
        '--show',
        action='store_true',
        help='Display the plot in a window (default: only save to file)'
    )
    
    args = parser.parse_args()
    
    if not os.path.exists(args.audio_file):
        print(f"Error: Audio file not found: {args.audio_file}", file=sys.stderr)
        sys.exit(1)
    
    plot_audio_beats(args.audio_file, args.max_duration, args.method, args.show)

