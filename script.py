import os
import numpy as np
import librosa

# ========================= CONFIG =========================
SAMPLE_RATE = 8000
N_FEATURES = 10        # 8 band energies + total energy + centroid
FFT_SIZE = 128
HOP_LENGTH = 64
N_BINS = 64            # FFT_SIZE / 2

# Band boundaries in bins (bin_width = 8000/128 = 62.5 Hz)
# Bands: 0-250, 250-500, 500-1k, 1k-1.5k, 1.5k-2k, 2k-2.5k, 2.5k-3k, 3k-4k Hz
BAND_STARTS = [0,  4,  8, 16, 24, 32, 40, 48]
BAND_ENDS   = [4,  8, 16, 24, 32, 40, 48, 64]

AUDIO_DIR = 'Audio Files'
CLASSES = ["down", "left", "off", "on", "right", "start", "stop", "up"]  # must match feat_table order

# =========================================================

def save_hamming_window(filename="hamming_window.h"):
    window = np.hamming(FFT_SIZE)
    with open(filename, "w") as f:
        f.write("#ifndef HAMMING_WINDOW_H\n#define HAMMING_WINDOW_H\n\n")
        f.write("#include <avr/pgmspace.h>\n\n")
        f.write(f"static const float hamming_window[{FFT_SIZE}] PROGMEM = {{\n")
        for i in range(0, FFT_SIZE, 8):
            line = ", ".join(f"{val:.6f}f" for val in window[i:i+8])
            f.write(f"    {line}{',' if i + 8 < FFT_SIZE else ''}\n")
        f.write("};\n\n#endif\n")
    print("✅ Hamming window saved to hamming_window.h")


def extract_features(y, sr=SAMPLE_RATE):
    """
    Mirrors exactly what the C code does:
      pre-emphasis → hamming → FFT → L1 magnitude → band energies + total energy + centroid
    Averaged across frames. Returns uint16-scaled vector.
    """
    # Pre-emphasis (matches C: x[i] -= 0.97 * x[i-1], x[0] = 0)
    y = y.copy().astype(np.float32)
    y[1:] -= 0.97 * y[:-1]
    y[0] = 0.0

    frame_features = []
    i = 0
    while i + FFT_SIZE <= len(y):
        frame = y[i:i + FFT_SIZE].copy()

        # Hamming window
        frame *= np.hamming(FFT_SIZE)

        # FFT — use numpy rfft, take only first N_BINS bins
        F = np.fft.rfft(frame, FFT_SIZE)          # shape: FFT_SIZE//2 + 1 = 65
        F = F[:N_BINS]                             # bins 0..63

        # L1 magnitude — matches C: abs(re) + abs(im)
        mag = np.abs(F.real) + np.abs(F.imag)     # shape: 64

        feats = []

        # Features 0–7: band energies (mean magnitude per band)
        for ks, ke in zip(BAND_STARTS, BAND_ENDS):
            feats.append(mag[ks:ke].mean())

        # Feature 8: total energy (skip DC bin 0), scaled like C (>> 6 = / 64)
        total_energy = mag[1:].sum() / 64.0
        feats.append(total_energy)

        # Feature 9: spectral centroid in bins
        denom = mag[1:].sum()
        centroid = (np.arange(1, N_BINS) * mag[1:]).sum() / denom if denom > 0 else 0.0
        feats.append(centroid)

        frame_features.append(feats)
        i += HOP_LENGTH

    if not frame_features:
        return np.zeros(N_FEATURES, dtype=np.float32)

    # Average across frames (same as C: accum[k] / n_frames)
    avg = np.mean(frame_features, axis=0).astype(np.float32)

    # Scale to uint16 range to match what the C nearest_neighbor expects
    # Use the same fixed scale factor your C code uses (adjust if needed)
    avg_scaled = np.clip(avg * 256.0, 0, 65535).astype(np.uint16)

    return avg_scaled


# ====================== MAIN PROCESSING ======================

print("Starting feature extraction...\n")

save_hamming_window()

word_features = {cls: [] for cls in CLASSES}

for folder in sorted(os.listdir(AUDIO_DIR)):
    folder_lower = folder.lower()
    if folder_lower not in word_features:
        print(f"⚠️  Skipping unknown folder: {folder}")
        continue

    folder_path = os.path.join(AUDIO_DIR, folder)
    print(f"Processing folder: {folder}")

    for file in os.listdir(folder_path):
        if not file.lower().endswith(('.wav', '.mp3', '.m4a')):
            continue

        file_path = os.path.join(folder_path, file)
        y, sr = librosa.load(file_path, sr=SAMPLE_RATE, mono=True)

        y_trimmed, _ = librosa.effects.trim(y, top_db=20)

        if len(y_trimmed) < FFT_SIZE:
            print(f"  ⚠️  {file} too short, skipping.")
            continue

        feat_vec = extract_features(y_trimmed, sr)
        word_features[folder_lower].append(feat_vec)
        print(f"  ✓ {file} → features: {feat_vec}")

# ====================== AVERAGE PER CLASS & WRITE HEADER ======================

print("\nGenerating speech_data.h ...")

with open("speech_data.h", "w") as f:
    f.write("#ifndef SPEECH_DATA_H\n#define SPEECH_DATA_H\n\n")
    f.write("#include <avr/pgmspace.h>\n\n")
    f.write(f"#define N_FEATURES {N_FEATURES}\n")
    f.write(f"#define PRE_EMPHASIS 0.97f\n\n")

    final_features = {}

    for word in CLASSES:
        vectors = word_features[word]
        if not vectors:
            print(f"⚠️  No samples for '{word}', writing zeros.")
            avg = np.zeros(N_FEATURES, dtype=np.uint16)
        else:
            # Average the already-uint16-scaled vectors
            avg = np.mean(vectors, axis=0).astype(np.uint16)

        final_features[word] = avg

        values = ", ".join(str(v) for v in avg)
        f.write(f"static const uint16_t feat_{word}[{N_FEATURES}] PROGMEM = {{{values}}};\n")
        print(f"✓ {word:10} → {avg}")

    f.write("\n// Pointer table — order must match labels[] in main.c\n")
    f.write("static const uint16_t* const feat_table[] PROGMEM = {\n")
    for word in CLASSES:
        f.write(f"    feat_{word},\n")
    f.write("};\n\n#endif\n")

print("\n✅ Done! Generated:")
print("   • hamming_window.h")
print("   • speech_data.h")