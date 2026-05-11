import serial
import numpy as np
from scipy.io.wavfile import write
import time
import os
import sys


start = 0
end = 0

ser = serial.Serial('COM4', 115200, timeout=1)

samples = []
recording = False


# audio_folders = os.listdir("./Audio Files")
base_dir = "./new_samples"
audio_folders = sorted([
    name for name in os.listdir(base_dir)
    if os.path.isdir(os.path.join(base_dir, name))
])

def next_index(folder):
    prefix = f"output_{folder.lower()}_"
    existing = []
    folder_path = os.path.join(base_dir, folder)
    for name in os.listdir(folder_path):
        if not name.startswith(prefix) or not name.endswith(".wav"):
            continue
        num = name[len(prefix):-4]
        if num.isdigit():
            existing.append(int(num))
    return max(existing) + 1 if existing else 0

def record_one(folder, index):
    global recording, samples, start, end

    print(f"file {index} in {folder} Waiting for START...")

    while True:
        if not recording:
            # Read line until START
            line = ser.readline()

            if b"START" in line:
                print("Recording started")
                start = time.time()
                samples = []
                recording = True

        else:
            # While recording -> read raw bytes
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
                    # Not STOP -> treat as audio
                    samples.append(ord(byte))
                    for b in rest:
                        samples.append(b)
            else:
                samples.append(byte[0])

    # ---------- Convert to audio ----------
    audio = np.array(samples, dtype=np.uint8)

    print("Samples:", len(samples))
    fs = 8000
    print("Fs:", fs)
    # Convert to 16-bit PCM
    audio = (audio.astype(np.int16) - 128) * 256

    out_path = f"{base_dir}/{folder}/output_{folder.lower()}_{index}.wav"
    write(out_path, fs, audio)  # match your sampling rate

    print(out_path)
    samples = []
    recording = False

while True:
    print("\nAvailable words:")
    for i, name in enumerate(audio_folders):
        print(f"  {i}) {name}")
    print("  8) Quit\n")

    choice_text = input("Your Choice: ").strip()
    if not choice_text.isdigit():
        print("Invalid choice")
        continue
    choice = int(choice_text)
    if choice == 8:
        print("Ending Program")
        sys.exit(0)
    if choice < 0 or choice >= len(audio_folders):
        print("Invalid choice")
        continue

    word = audio_folders[choice]

    count_text = input("How many samples? (default 1): ").strip()
    count = int(count_text) if count_text.isdigit() else 1

    idx = next_index(word)
    for _ in range(count):
        record_one(word, idx)
        idx += 1
