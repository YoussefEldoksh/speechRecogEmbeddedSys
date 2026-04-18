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

sns.set_style('whitegrid')

audio_data = os.listdir('Audio Files')



print(pd.DataFrame(audio_data, columns=['Audio Files']))

mfcc_list = []

for file in audio_data:
    print(f"File: {file}, Size: {os.path.getsize(f'Audio Files/{file}')/1024:.2f} KB")
    y, sampling_rate = librosa.load(f'Audio Files/{file}', sr=None)
    y_trimmed , _ = librosa.effects.trim(y, top_db=20)


    n_mfcc = 13
    mfcc = librosa.feature.mfcc(y=y_trimmed, sr=sampling_rate, n_mfcc=n_mfcc)    
    # Normalize: mean of each coefficient across time (gives a fixed-size vector)
    mfcc_mean = np.mean(mfcc, axis=1)  # shape: (13,)
    
    # Quantize to int8 (range -128 to 127) to fit in AVR ROM
    mfcc_scaled = np.clip(mfcc_mean, -128, 127).astype(np.int8)
    mfcc_list.append(mfcc_scaled)

for i, mfcc in enumerate(mfcc_list):
    mfcc_str = ", ".join(str(v) for v in mfcc)
    print(f"File: {audio_data[i]}, MFCC Mean Vector: {mfcc_str}")

print("\n\n\n")

print(f"MFCC List Shape: {len(mfcc_list)} files, each with {n_mfcc} coefficients:")
print(pd.DataFrame(mfcc_list, columns=[f'MFCC_{i+1}' for i in range(n_mfcc)], index=[f'File_{audio_data[i]}' for i in range(len(audio_data))]))

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