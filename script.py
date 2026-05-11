import os
import numpy as np
import librosa

# ========================= CONFIG =========================
SAMPLE_RATE = 8000
FFT_SIZE    = 64
HOP_LENGTH  = 32
N_MEL_BANDS = 6
N_FEATURES  = N_MEL_BANDS + 2  # ZCR, log-STE, mel0..mel5

BAND_START = [0, 2, 4, 8, 16, 24]
BAND_END   = [2, 4, 8, 16, 24, 32]
ADC_CENTER = 63

AUDIO_DIR = 'Audio Files'
# =========================================================


def ilog2_scaled(x):
    if x == 0:
        return 0
    shift = 0
    v = int(x)
    while v > 1:
        v >>= 1
        shift += 1
    frac = ((int(x) >> (shift - 3)) & 0x07) if shift >= 3 else 0
    return (shift * 8) + frac


def pre_emphasis(x):
    if len(x) < 2:
        return x
    y = x.astype(np.float32).copy()
    for i in range(len(y) - 1, 0, -1):
        y[i] = y[i] - (0.97 * y[i - 1])
    y[0] = 0
    return y


def extract_features(y, sr=SAMPLE_RATE):
    """
    Returns a 1-D feature vector of length N_FEATURES:
      [zcr_mean, log_ste_mean, mel_band_0, ..., mel_band_5]
    Matches current C pipeline (no FFT, ilog2 scaling).
    """
    adc = np.clip(np.round(y * 127) + ADC_CENTER, 0, 255).astype(np.int16)
    y = (adc - ADC_CENTER).astype(np.int8)

    accum_zcr = 0
    accum_ste = 0
    accum_mel = np.zeros(N_MEL_BANDS, dtype=np.int64)
    n_frames = 0

    addr = 0
    while addr + FFT_SIZE <= len(y):
        frame = y[addr:addr + FFT_SIZE]

        # ZCR scaled by 4
        zcr = np.sum((frame[1:] >= 0) != (frame[:-1] >= 0))
        accum_zcr += int(zcr) * 4

        # STE log2 scaled
        s = frame.astype(np.int16)
        frame_energy = int(np.sum(s * s))
        accum_ste += ilog2_scaled(frame_energy)

        # Match MCU: scale, pre-emphasis, no FFT
        fft_real = (frame.astype(np.int16) << 6).astype(np.int32)
        fft_real = pre_emphasis(fft_real)

        for b in range(N_MEL_BANDS):
            k0 = BAND_START[b]
            k1 = BAND_END[b]
            band_energy = 0
            for k in range(k0, k1):
                re = int(fft_real[k])
                band_energy += ((re * re) >> 4)
            accum_mel[b] += ilog2_scaled(band_energy)

        n_frames += 1
        addr += HOP_LENGTH

    if n_frames == 0:
        return np.zeros(N_FEATURES, dtype=np.uint16)

    feat = np.zeros(N_FEATURES, dtype=np.uint16)
    feat[0] = accum_zcr // n_frames
    feat[1] = accum_ste // n_frames
    for b in range(N_MEL_BANDS):
        feat[b + 2] = accum_mel[b] // n_frames
    return feat


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

        if len(y) < FFT_SIZE:
            print(f"  ⚠️  {file} too short, skipping.")
            continue

        feat_vec = extract_features(y, sr)
        word_features[folder.lower()].append(feat_vec)

        preview = " ".join(str(int(v)) for v in feat_vec)
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

        avg_vec = np.mean(vectors, axis=0).round().astype(np.uint16)
        final_features[word] = avg_vec

        values = ", ".join(str(int(x)) for x in avg_vec)
        f.write(f"// ZCR, log-STE, Mel[0..{N_MEL_BANDS-1}]\n")
        f.write(f"static const uint16_t feat_{word}[N_FEATURES] PROGMEM = {{{values}}};\n\n")
        print(f"  ✓ {word:12} → {avg_vec}")

    # Lookup table (pointer array in Flash)
    words_sorted = sorted(final_features.keys())

    f.write("// Pointer table — index matches LABELS[] below\n")
    f.write("static const uint16_t* const feat_table[] PROGMEM = {\n")
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
    vals = "  ".join(f"{int(v):8d}" for v in vec)
    print(f"{word:12}  {vals}")