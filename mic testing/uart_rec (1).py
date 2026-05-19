import serial
import numpy as np
from scipy.io.wavfile import write
import time
import os
import sys
import matplotlib.pyplot as plt
import librosa
start = 0
end = 0
counter = 0

ser = serial.Serial('COM6', 115200, timeout=1)

samples = []
recording = False


audio_folders = os.listdir("./Audio Files");

print(audio_folders)

print('''
Choose Word You'r going to Record or (8) to quit:
      0) BACK
      1) GO
      2) ONE
      3) SHIP
      8) Quit

''')

choice = int(input("Your Choice: "))

if(choice == 8):
    print("Ending Program")
    sys.exit(0)  


word = audio_folders[choice]
zcr_arr = []
ste_arr = []
folder_zcr_arr = []
folder_ste_arr = []

for folder in audio_folders:
    if folder == word:
        for i in range(0,10):
            zcr_arr = []  # reset here
            ste_arr = []  # reset here

            print(f"file {i} in {folder} Waiting for START...")

            while True:
                if not recording:
                    # Read line until START
                    line = ser.readline()

                    if b"START" in line:
                        print("Recording started")
                        start = time.time()
                        # record samples...

                        samples = []
                        recording = True

                else:
                    # While recording → read raw bytes
                    byte = ser.read(1)

                    if not byte:
                        continue

                    # Detect STOP manually
                    if byte == b'S':  # possible start of STOP
                        rest = ser.read(4)  # read remaining "TOP\n"
                        if rest == b"TOP\n":
                            print("Recording stopped")
                            end = time.time()

                            break
                        else:
                            # Not STOP → treat as audio
                            samples.append(ord(byte))
                            for b in rest:
                                samples.append(b)
                    else:
                        samples.append(byte[0])

            # ---------- Convert to audio ----------



            
            # fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6))
            # ax1.plot(samples)
            # ax1.set_title("Raw Audio")
            number_of_samples = len(samples)
            samples_per_frame = number_of_samples // 8      

            print(samples)

            file_features = []
            zcr = 0 
            ste = 0
            for frame in range(8): 
                zcr = 0 
                ste = 0
                prev_val = samples[samples_per_frame * frame]
                for sample in range(samples_per_frame * frame, samples_per_frame * (frame + 1)):
                    if (prev_val < 60 and samples[sample] > 60) or (prev_val > 60 and samples[sample] < 60):
                        zcr += 1
                    prev_val = samples[sample]
                    ste += int(samples[sample] ** 2 )
                zcr_arr.append(zcr)
                ste_arr.append(ste)
            folder_zcr_arr.append(zcr_arr)
            folder_ste_arr.append(ste_arr)
            

            audio = np.array(samples)


            # audio = (audio.astype(np.int8) - 60)
            print("Samples:", len(samples))
            # fs = int(len(samples)/(end-start))
            fs =  8000
            print("Fs:", fs)
            # Convert to 16-bit PCM
            audio = (audio.astype(np.int16) - 61) * 256





            # write(f"./output_{folder.lower()}_{i}.wav", fs, audio)  # match your sampling rate
            write(f"./Audio Files/{folder}/output_{folder.lower()}_{i}.wav", fs, audio)  # match your sampling rate

            print(f"./Audio Files/{folder}/output_{folder.lower()}_{i}.wav")

            # y, _ = librosa.load(f"./Audio Files/{folder}/output_{folder.lower()}_{i}.wav", sr=8000)
            # y = y*128
            

            # ax2.plot(y)
            # ax2.set_title("Librosa Loaded")
            # plt.tight_layout()
            # plt.show()
            samples = []
            byte = b""
            recording = False

zcr_avg_arr = []
zcr_acc = 0

for i in range(len(folder_zcr_arr[0])):
    zcr_acc = 0  
    for j in range(len(folder_zcr_arr)):
        zcr_acc += folder_zcr_arr[j][i]
    zcr_acc /= (len(folder_zcr_arr)-1)
    zcr_avg_arr.append(zcr_acc)

zcr_min = min(zcr_avg_arr)
zcr_max = max(zcr_avg_arr)
zcr_avg_arr = [(z - zcr_min) / (zcr_max - zcr_min) for z in zcr_avg_arr]

ste_avg_arr = []
ste_acc = 0

for i in range(len(folder_ste_arr[0])):
    ste_acc = 0  
    for j in range(len(folder_ste_arr)):
        ste_acc += folder_ste_arr[j][i]
    ste_acc /= (len(folder_ste_arr)-1)
    ste_avg_arr.append(ste_acc)

ste_min = min(ste_avg_arr)
ste_max = max(ste_avg_arr)
ste_avg_arr = [(s - ste_min) / (ste_max - ste_min) for s in ste_avg_arr]

# Write to features.txt
with open('features.txt', 'a') as f:
    f.write("// ---- ZCR reference arrays (one per word) ----\n")
    zcr_str = ', '.join(f'{z:.3f}' for z in zcr_avg_arr)
    f.write(f"static const float {word.lower()}_zcr[N_FRAMES] PROGMEM = {{{zcr_str}}};\n")


    f.write("\n\n")

    f.write("// ---- STE reference arrays (one per word) ----\n")
    ste_str = ', '.join(f'{s:.3f}' for s in ste_avg_arr)
    f.write(f"static const float {word.lower()}_ste[N_FRAMES] PROGMEM = {{{ste_str}}};\n")

    f.write("\n\n")

print("Saved to features.txt")