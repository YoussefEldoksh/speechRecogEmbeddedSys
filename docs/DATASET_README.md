Dataset Checklist
=================

Folder structure
- Audio Files/
  - on/
  - off/
  - up/
  - down/
  - left/
  - right/
  - start/
  - stop/

Recording requirements
- 20-30 samples per word
- Mono
- 8 kHz sample rate
- 8-bit resolution

Notes
- Keep recordings in a quiet room first; then add a noisy set for evaluation.
- Use the same microphone module you use on the MCU to reduce mismatch.
- If you change dataset, re-run `script.py` to regenerate templates.
