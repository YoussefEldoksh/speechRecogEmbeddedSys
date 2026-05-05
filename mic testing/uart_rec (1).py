import serial
import numpy as np
from scipy.io.wavfile import write
import time
import os
import sys


start = 0
end = 0

ser = serial.Serial('COM6', 115200, timeout=1)

samples = []
recording = False


audio_folders = os.listdir("./Audio Files");

print(audio_folders)

print('''
Choose Word You'r going to Record or (8) to quit:
      0) Down
      1) Left
      2) Off
      3) On
      4) Right
      5) Start
      6) Stop
      7) Up
      8) Quit

''')

choice = int(input("Your Choice: "))

if(choice == 8):
    print("Ending Program")
    sys.exit(0)  


word = audio_folders[choice]



for folder in audio_folders:
    if folder == word:
        for i in range(5):

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
            audio = np.array(samples, dtype=np.uint8)

            print("Samples:", len(samples))
            # fs = int(len(samples)/(end-start))
            fs =  8000
            print("Fs:", fs)
            # Convert to 16-bit PCM
            audio = (audio.astype(np.int16) - 128) * 256

            write(f"./Audio Files/{folder}/output_{folder.lower()}_{i}.wav", fs, audio)  # match your sampling rate

            print(f"./Audio Files/{folder}/output_{folder.lower()}_{i}.wav")
            samples = []
            byte = b""
            recording = False
