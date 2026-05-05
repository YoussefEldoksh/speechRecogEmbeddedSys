import speech_recognition as sr
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
# import glob as glob

import librosa
import librosa.display as ld
import os 

import sounddevice as sd
import soundfile as sf
from pydub import AudioSegment





# sns.set_style('whitegrid')

audio_data = sorted(os.listdir('Audio Files'))

for folder in audio_data:
    print(f"""Folder: {folder},
            Files in Folder: {os.listdir(f'Audio Files/{folder}') }
          
          """)


zcr_list = []
short_term_energy_list = []
spectral_centroid_list = []

features = []
number_of_files = 0

# def convert_audio(input_file):
#     audio = AudioSegment.from_file(input_file)
#     audio = audio.set_channels(1)  # Convert to mono
#     audio = audio.set_frame_rate(8000)
#     audio = audio.set_sample_width(1)
#     return audio

# for folder in audio_data:
#     os.makedirs(f'Converted Audio Files/{folder}', exist_ok=True)
#     for file in os.listdir(f'Audio Files/{folder}'):    
#         audio = convert_audio(f'Audio Files/{folder}/{file}')
#         fname = os.path.splitext(file)[0] + '.wav'
#         audio.export(f'Converted Audio Files/{folder}/{fname}', format='wav')



frame_length = 64
hop_length = 64

# { 'yes': {'zcr': [], 'ste': [], 'sc': []}, 'no': {...}, ... }
word_features = {folder.lower(): {'zcr': [], 'ste': []} for folder in audio_data}

for folder in audio_data:
    for file in os.listdir(f'Audio Files/{folder}'):
        y, sr = librosa.load(f'Audio Files/{folder}/{file}', sr=8000, mono=True)
        y_int8 = (y * 128).clip(-128, 127).astype(np.float32)

        y_trimmed, _ = librosa.effects.trim(y, top_db=50)
        # ZCR rate (crossings per sample)
        zero_crossings = np.sum(np.diff(np.sign(y_int8)) != 0)
        zcr = zero_crossings / len(y_int8)

        # STE mean square in int8 scale
        ste = np.sum(y_int8 ** 2) / len(y_int8)
        
        print(f"zcr of file {folder} : {np.mean(zcr)}")


        # sc  = librosa.feature.spectral_centroid(y=y_trimmed, sr=sr, n_fft=frame_length, hop_length=hop_length)

        # One mean value per file → collect across all files of this word
        word_features[folder.lower()]['zcr'].append(np.mean(zcr))
        word_features[folder.lower()]['ste'].append(np.mean(ste)/100000)
        # word_features[folder]['sc'].append(np.mean(sc))

# Now average across all files per word → single vector per word
final_features = []
for folder, feats in word_features.items():
    final_features.append({
        'word':     folder.lower(),
        'zcr_mean': np.mean(feats['zcr']),
        'ste_mean': np.mean(feats['ste']),
        # 'sc_mean':  np.mean(feats['sc']),
    })

# Write to file
with open("features.txt", "w") as f:
    f.write("word | zcr_mean | ste_mean | sc_mean\n")
    for row in final_features:
        line = f" static const float PROGMEM features_{row['word']}[] = {{ {row['zcr_mean']:.4f}, {row['ste_mean']:.4f} }};"
        f.write(line + "\n")
        print(line)


#--------------------------------------------------------------


# y, sampling_rate = librosa.load(f'Audio Files/{audio_data[1]}', sr=None)
# y_trimmed , _ = librosa.effects.trim(y, top_db=20)  # Trim the audio signal to remove silence, with a threshold of 20 dB
    # y is the raw data of the audio file, and sr is the sampling rate (number of samples per second)

# print(f"Sampling Rate: {sampling_rate} Hz")
# print(f"Audio Data Type: {type(y)}")
# print(f"First 10 Samples of Audio Data: {y[:10]}")
# print(f"Audio Data Shape: {y.shape}")

#--------------------------------------------------------------



#--------------------------------------------------------------

# series = pd.Series(y).plot( figsize=(14, 5), lw=1, title='Audio Signal Waveform')
    # Plotting the audio data
# plt.show()
#--------------------------------------------------------------


#--------------------------------------------------------------

# series = pd.Series(y_trimmed).plot( figsize=(14, 5), lw=1, title='Audio Signal Waveform (Trimmed)', color='orange')
# plt.show()
    # Plotting the audio data
#--------------------------------------------------------------



#--------------------------------------------------------------
# D  = librosa.stft(y)
# Sound_dB = librosa.amplitude_to_db(np.abs(D), ref=np.max)

# librosa.display.specshow(Sound_dB, y_axis='log', x_axis='time')
# plt.title('Spectrogram')
# plt.colorbar(format='%+2.0f dB')
# plt.tight_layout()
# plt.show()
#--------------------------------------------------------------





#--------------------------------------------------------------
# n_mfcc = 13
# mfcc = librosa.feature.mfcc(y=y_trimmed, sr=sampling_rate, n_mfcc=n_mfcc)
# print(f"MFCC Shape: {mfcc.shape}") # (n_mfcc, n_frames)

# # Normalize: mean of each coefficient across time (gives a fixed-size vector)
# mfcc_mean = np.mean(mfcc, axis=1)  # shape: (13,)
# print(f"MFCC Mean Vector: {mfcc_mean}")

# # Quantize to int8 (range -128 to 127) to fit in AVR ROM
# mfcc_scaled = np.clip(mfcc_mean, -128, 127).astype(np.int8)
# print(f"Quantized MFCC: {mfcc_scaled}")
#--------------------------------------------------------------

















# r = sr.Recognizer()


# with sr.Microphone() as source:
#     while True:
#         print("Say something!")
#         audio = r.listen(source)
#         try:
#             output = r.recognize_google(audio)
#             print("You said: " + output)
#         except sr.UnknownValueError:
#             print("Sorry, I could not understand what you said.")
#         except sr.RequestError:
#             print("Sorry, there was an error with the speech recognition service.")