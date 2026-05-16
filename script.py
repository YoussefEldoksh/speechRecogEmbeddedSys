import os
import numpy as np
import librosa
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from scipy.io import wavfile
# from mic testing.uart_rec (1) import audio_folders

SAMPLE_RATE = 8000

ADC_CENTER = 63

AUDIO_DIR = 'Audio Files'
zcr = 0
ste = 0
np.set_printoptions(threshold=np.inf)

audio_folders = os.listdir("Audio Files")

features_of_folder = []
word_features = []
for folder in audio_folders:
    features_of_folder = []  # reset per folder
    for file in os.listdir(f'Audio Files/{folder}'):  # fix #1
        y, sr = librosa.load(f'Audio Files/{folder}/{file}', sr=SAMPLE_RATE)
        mean = np.mean(y)
        y = y - mean
        adc = np.clip(np.round(y * 127) + ADC_CENTER, 0, 255).astype(np.int16)
        y = (adc - ADC_CENTER).astype(np.int8)

        number_of_samples = len(y)
        samples_per_frame = number_of_samples // 8  # fix #2

        file_features = []
        for frame in range(8):  # fix #6
            zcr = 0  # fix #3
            ste = 0
            prev_val = y[samples_per_frame * frame]
            for sample in range(samples_per_frame * frame, samples_per_frame * (frame + 1)):
                if (prev_val < 0 and y[sample] > 0) or (prev_val > 0 and y[sample] < 0):
                    zcr += 1
                prev_val = y[sample]
                ste += int(y[sample]) ** 2  # cast to int to avoid int8 overflow
            file_features.append([zcr, ste])
        features_of_folder.append(file_features)

    # Average across files, per frame
    zcr_acc = [0] * 8
    ste_acc = [0] * 8
    for arr in features_of_folder:       # fix #4
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

with open('features.txt', 'w') as f:
    for word in word_features:
        f.write(f"// {word['folder']}\n")
        zcr_str = ', '.join(f'{z:.2f}' for z in word['avg_zcr'])
        ste_str = ', '.join(f'{s:.2f}' for s in word['avg_ste'])
        f.write(f"float {word['folder'].lower()}_feat[] = {{{zcr_str}}} , {{{ste_str}}};\n")

print("Saved to features.txt")