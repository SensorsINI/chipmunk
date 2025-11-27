#!/usr/bin/env python3
"""
Plot audio envelope with beat ticks for the first 10 seconds
"""

import sys
import subprocess
import tempfile
import os
import numpy as np
import matplotlib.pyplot as plt

def extract_beats(audio_file):
    """Extract beat times using aubioonset"""
    beats = []
    try:
        result = subprocess.run(
            ['aubioonset', '-i', audio_file, '-t', '0.3'],
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
    # Create temporary wav file
    with tempfile.NamedTemporaryFile(suffix='.wav', delete=False) as tmp_wav:
        tmp_wav_path = tmp_wav.name
    
    try:
        # Convert audio to WAV format with ffmpeg (first 'duration' seconds)
        subprocess.run(
            ['ffmpeg', '-y', '-i', audio_file, '-t', str(duration),
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

def plot_audio_beats(audio_file, max_duration=10.0):
    """Plot audio envelope with beat ticks"""
    print(f"Extracting beats from {audio_file}...")
    beats = extract_beats(audio_file)
    
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
    output_file = 'audio_beats_plot.png'
    try:
        plt.savefig(output_file, dpi=150, bbox_inches='tight')
        print(f"Plot saved to {os.path.abspath(output_file)}")
    except Exception as e:
        print(f"Error saving plot: {e}", file=sys.stderr)
        sys.exit(1)
    
    # Try to display if possible (suppress errors)
    try:
        plt.show()
    except Exception:
        pass  # Display not available, that's fine
    
    plt.close()  # Close figure to free memory

if __name__ == '__main__':
    if len(sys.argv) < 2:
        audio_file = os.path.expanduser('~/pcmp_home/04 Reaching Dub.m4a')
    else:
        audio_file = sys.argv[1]
    
    max_duration = float(sys.argv[2]) if len(sys.argv) > 2 else 10.0
    
    if not os.path.exists(audio_file):
        print(f"Error: Audio file not found: {audio_file}", file=sys.stderr)
        sys.exit(1)
    
    plot_audio_beats(audio_file, max_duration)

