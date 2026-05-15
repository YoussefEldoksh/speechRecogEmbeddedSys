import os
import numpy as np
import librosa
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from scipy.io import wavfile

SAMPLE_RATE = 8000

ADC_CENTER = 63

AUDIO_DIR = 'Audio Files'
zcr = 0


np.set_printoptions(threshold=np.inf)

sr, y = wavfile.read('Audio Files/Down/output_down_1.wav')

plt.figure(figsize=(12, 4))
plt.plot(y)
plt.title("Audio Signal")
plt.xlabel("Sample")
plt.ylabel("Amplitude")
plt.tight_layout()
plt.show(block=True)
print(len(y))
adc = np.clip(np.round(y * 127) + ADC_CENTER, 0, 255).astype(np.int16)
y = (adc - ADC_CENTER).astype(np.int8)
print(y)



perv_val = y[0]

for val in range(1, len(y)-1):
    if((perv_val < 0 and y[val] >0) or (perv_val > 0 and y[val] < 0)):
        zcr +=1
    perv_val = y[val]

print(zcr)