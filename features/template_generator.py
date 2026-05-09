import numpy as np
import librosa
import os

# The 8 words you chose
WORDS = ["ON", "OFF", "START", "STOP", "LEFT", "RIGHT", "UP", "DOWN"]
SAMPLES_PER_WORD = 20

def extract_13_features(audio_path):
    # Load audio (8kHz as per your project requirement)
    y, sr = librosa.load(audio_path, sr=8000)
    
    # Pre-processing: Normalization
    y = y / np.max(np.abs(y))
    
    features = []
    
    # 1. Energy (Scaled to 0-255)
    features.append(np.mean(librosa.feature.rms(y=y)) * 255)
    
    # 2. Zero Crossing Rate (Scaled)
    features.append(np.mean(librosa.feature.zero_crossing_rate(y)) * 255)
    
    # 3. Spectral Centroid (Fourier-based)
    sc = librosa.feature.spectral_centroid(y=y, sr=sr)[0]
    features.append((np.mean(sc) / (sr/2)) * 255)
    
    # 4. Spectral Rolloff
    rolloff = librosa.feature.spectral_rolloff(y=y, sr=sr)[0]
    features.append((np.mean(rolloff) / (sr/2)) * 255)
    
    # 5-13. Mel-Frequency Bands (MFCC-lite)
    mfccs = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=9)
    for m in mfccs:
        # Normalize MFCCs to 0-255 range for the MCU
        m_norm = (np.mean(m) + 100) / 200 * 255 
        features.append(np.clip(m_norm, 0, 255))
        
    return np.array(features, dtype=np.uint8)

# Logic to loop through 160 audios
master_templates = []

for word in WORDS:
    word_data = []
    for i in range(1, SAMPLES_PER_WORD + 1):
        file_path = f"dataset/{word}_{i}.wav" # Adjust to your folder structure
        if os.path.exists(file_path):
            feat = extract_13_features(file_path)
            word_data.append(feat)
    
    # Average the 20 samples into 1 template
    avg_template = np.mean(word_data, axis=0).astype(np.uint8)
    master_templates.append(avg_template)

# Print the C code for your header file
print("const uint8_t word_templates[8][13] PROGMEM = {")
for i, template in enumerate(master_templates):
    line = ", ".join(map(str, template))
    print(f"    {{{line}}}, // Word {i}: {WORDS[i]}")
print("};")