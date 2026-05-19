import os
import numpy as np
import librosa
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

SAMPLE_RATE = 8000
AUDIO_DIR = 'Audio Files'

np.set_printoptions(threshold=np.inf)
counter = 0
audio_folders = os.listdir(AUDIO_DIR)

word_features = []

for folder in audio_folders:
    features_of_folder = []

    for file in os.listdir(f'{AUDIO_DIR}/{folder}'):
        # librosa loads as float32 in [-1.0, 1.0]
        y, sr = librosa.load(f'{AUDIO_DIR}/{folder}/{file}', sr=SAMPLE_RATE)

        # Match AVR pipeline: 10-bit ADC >> 2 → [0,255] → subtract 128 → [-128,127]
        # librosa gives [-1,1], so just scale to [-128,127] — no DC offset
        y = y * 127.0  # now in [-127, 127], matching int8_t range

        if counter == 0:
            plt.plot(y)
            plt.title(f'{folder}/{file}')
            plt.show()
            counter += 1

        number_of_samples = len(y)
        window_size = number_of_samples // 8

        file_features = []
        for frame in range(8):
            zcr = 0.0
            ste = 0.0

            start = window_size * frame
            end   = window_size * (frame + 1)

            # Start prev from first sample, loop from second — matches fixed C code
            prev_val = y[start]
            for j in range(start + 1, end):
                sample = y[j]
                if (prev_val < 0 and sample >= 0) or (prev_val >= 0 and sample < 0):
                    zcr += 1.0
                prev_val = sample

                # Normalize to [-1, 1] before squaring — matches C: s = sample/128.0
                s = sample / 128.0
                ste += s * s

            # Divide by (window_size - 1) — matches fixed C code
            norm = float(window_size - 1)
            zcr /= norm   # in [0, 1]: max crossings = window_size - 1
            ste /= norm   # in [0, 1]: max energy per sample = 1.0

            file_features.append([zcr, ste])

        features_of_folder.append(file_features)

    # Average across files per frame
    zcr_acc = [0.0] * 8
    ste_acc = [0.0] * 8
    for arr in features_of_folder:
        for frame_idx, (z, s) in enumerate(arr):
            zcr_acc[frame_idx] += z
            ste_acc[frame_idx] += s

    n = len(features_of_folder)
    word_features.append({
        'folder': folder,
        'avg_zcr': [z / n for z in zcr_acc],
        'avg_ste': [s / n for s in ste_acc],
    })
    print(word_features[-1])

# Write to features.txt
with open('features.txt', 'w') as f:
    f.write("// ---- ZCR reference arrays (one per word) ----\n")
    for word in word_features:
        zcr_str = ', '.join(f'{z:.4f}' for z in word['avg_zcr'])
        f.write(f"static const float {word['folder'].lower()}_zcr[N_FRAMES] PROGMEM = {{{zcr_str}}};\n")

    f.write("\n\n")

    f.write("// ---- STE reference arrays (one per word) ----\n")
    for word in word_features:
        ste_str = ', '.join(f'{s:.4f}' for s in word['avg_ste'])
        f.write(f"static const float {word['folder'].lower()}_ste[N_FRAMES] PROGMEM = {{{ste_str}}};\n")

print("Saved to features.txt")