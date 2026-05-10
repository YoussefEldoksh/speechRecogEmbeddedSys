import os
import numpy as np
import librosa

# ========================= CONFIG =========================
SAMPLE_RATE = 8000
FFT_SIZE    = 128
HOP_LENGTH  = 64
N_MEL_BANDS = 6       # Number of Mel filter bank channels
N_FEATURES  = N_MEL_BANDS + 2  # +1 ZCR, +1 STE  →  8 total

AUDIO_DIR = 'Audio Files'
# =========================================================


def extract_features(y, sr=SAMPLE_RATE):
    """
    Returns a 1-D feature vector of length N_FEATURES:
      [zcr_mean, ste_mean, mel_band_0, ..., mel_band_5]
    All computed per-frame then averaged across time.
    """
    # --- Pre-emphasis ---
    y = np.append(y[0], y[1:] - 0.97 * y[:-1])

    # --- Zero-Crossing Rate (1 value) ---
    zcr_frames = librosa.feature.zero_crossing_rate(
        y, frame_length=FFT_SIZE, hop_length=HOP_LENGTH, center=False
    )  # shape: (1, n_frames)
    zcr_mean = np.mean(zcr_frames)

    # --- Short-Time Energy (1 value) ---
    # Compute per-frame energy: mean of squared samples
    frames = librosa.util.frame(y, frame_length=FFT_SIZE, hop_length=HOP_LENGTH)
    # frames shape: (FFT_SIZE, n_frames)
    ste_frames = np.mean(frames ** 2, axis=0)   # shape: (n_frames,)
    ste_mean   = np.mean(ste_frames)

    # --- Mel Filter Bank Log-Energies (N_MEL_BANDS values) ---
    mel_spec = librosa.feature.melspectrogram(
        y=y,
        sr=sr,
        n_fft=FFT_SIZE,
        hop_length=HOP_LENGTH,
        n_mels=N_MEL_BANDS,
        window='hamming',
        center=False,
        power=2.0
    )  # shape: (N_MEL_BANDS, n_frames)

    # Log-compress (mirrors what the ear does, stabilises dynamic range)
    log_mel = np.log(mel_spec + 1e-9)           # +eps avoids log(0)
    mel_mean = np.mean(log_mel, axis=1)          # shape: (N_MEL_BANDS,)

    # --- Normalise STE to same ballpark as other features ---
    # log-scale STE so it doesn't dwarf the Mel energies
    ste_log = np.log(ste_mean + 1e-9)

    # --- Concatenate into one vector ---
    feature_vec = np.concatenate([[zcr_mean, ste_log], mel_mean])
    return feature_vec                           # shape: (8,)


# ====================== MAIN PROCESSING ======================

print("Starting feature extraction...\n")

audio_data    = sorted(os.listdir(AUDIO_DIR))
word_features = {folder.lower(): [] for folder in audio_data}

for folder in audio_data:
    folder_path = os.path.join(AUDIO_DIR, folder)
    if not os.path.isdir(folder_path):
        continue
    print(f"Processing folder: {folder}")

    for file in sorted(os.listdir(folder_path)):
        if not file.lower().endswith(('.wav', '.mp3', '.m4a')):
            continue

        file_path = os.path.join(folder_path, file)
        y, sr = librosa.load(file_path, sr=SAMPLE_RATE, mono=True)

        # Trim leading/trailing silence
        y_trimmed, _ = librosa.effects.trim(y, top_db=20)

        if len(y_trimmed) < FFT_SIZE:
            print(f"  ⚠️  {file} too short, skipping.")
            continue

        feat_vec = extract_features(y_trimmed, sr)
        word_features[folder.lower()].append(feat_vec)

        preview = " ".join(f"{v:.4f}" for v in feat_vec)
        print(f"  ✓ {file} → [{preview}]")

# ====================== AVERAGE PER CLASS ======================

print("\nGenerating final reference features...")

final_features = {}

with open("word_features.h", "w") as f:
    f.write("#ifndef WORD_FEATURES_H\n")
    f.write("#define WORD_FEATURES_H\n\n")
    f.write("#include <avr/pgmspace.h>\n\n")
    f.write(f"#define N_FEATURES {N_FEATURES}\n\n")

    for word, vectors in sorted(word_features.items()):
        if len(vectors) == 0:
            print(f"  ⚠️  No valid recordings for '{word}', skipping.")
            continue

        avg_vec = np.mean(vectors, axis=0)       # shape: (N_FEATURES,)
        final_features[word] = avg_vec

        values = ", ".join(f"{x:.6f}f" for x in avg_vec)
        f.write(f"// ZCR, log-STE, Mel[0..{N_MEL_BANDS-1}]\n")
        f.write(f"static const float feat_{word}[N_FEATURES] PROGMEM = {{{values}}};\n\n")
        print(f"  ✓ {word:12} → {np.round(avg_vec, 4)}")

    # Lookup table (pointer array in Flash)
    words_sorted = sorted(final_features.keys())

    f.write("// Pointer table — index matches LABELS[] below\n")
    f.write("static const float* const feat_table[] PROGMEM = {\n")
    for word in words_sorted:
        f.write(f"    feat_{word},\n")
    f.write("};\n\n")

    # Label strings in Flash
    f.write("// Word labels\n")
    label_list = ", ".join(f'"{w}"' for w in words_sorted)
    f.write(f"static const char* const LABELS[] PROGMEM = {{{label_list}}};\n\n")

    f.write(f"#define N_WORDS {len(words_sorted)}\n\n")
    f.write("#endif // WORD_FEATURES_H\n")

print("\n✅ Done! Generated: word_features.h")

# ---- Debug dump ----
print("\n--- Final averaged feature vectors ---")
np.set_printoptions(precision=4, suppress=True)
header = f"{'Word':12}  {'ZCR':>8}  {'logSTE':>8}  " + \
         "  ".join(f"Mel{i}" for i in range(N_MEL_BANDS))
print(header)
print("-" * len(header))
for word, vec in sorted(final_features.items()):
    vals = "  ".join(f"{v:8.4f}" for v in vec)
    print(f"{word:12}  {vals}")