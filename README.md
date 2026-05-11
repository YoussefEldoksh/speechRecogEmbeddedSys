# speechRecogEmbeddedSys

## Overview
This repository contains an ATmega32-based embedded speech recognition prototype. Audio is sampled at ~8 kHz using the ADC, stored to external SPI SRAM, and then features are extracted and classified against per-word templates using a nearest-neighbor distance metric. The active MCU project is in `SpeechRecog.X`.

The project has several legacy/experimental pipelines (FFT-based and time-domain-only). The current active pipeline is **time-domain features only** with VAD gating, fixed-length capture, and simple nearest-neighbor classification.

This README is meant for debugging assistance. It describes the code paths, feature definitions, constants, build outputs, and known issues.

## Workspace layout (key paths)
- `SpeechRecog.X/` - active MCU project (ATmega32)
- `SpeechRecog.X/main.c` - main firmware, ISR sampling, SRAM capture, feature extraction, classifier
- `SpeechRecog.X/adc.c`, `SpeechRecog.X/ADC_interface.h` - ADC init + read
- `SpeechRecog.X/speech_data.h` - feature templates used by classifier
- `script.py` - Python template generator for the current feature pipeline
- `word_features.h` - generated output from `script.py` (must be copied into `SpeechRecog.X/speech_data.h`)
- `features/` - legacy experiments (13-feature MFCC-like pipeline). Not used by the active firmware.
- `out/SpeechRecog/default.hex` - hex produced by MPLAB CMake build

## Build / flash
The build is driven by MPLAB + CMake (ninja). Current output is:
- `out/SpeechRecog/default.hex`

Make sure the flashed hex is the one from `out/SpeechRecog/default.hex`, not any legacy MPLAB output folders.

## End-to-end signal flow (current firmware)
1. **INT0 button** toggles `recording` (ISR toggles PD7 for a quick visual cue).
2. **Timer1 ISR** samples ADC at ~8 kHz and stores into external SRAM.
3. On stop (INT0 again), firmware runs **compute_features** on the captured samples.
4. Feature vector (8 values) is printed over UART, then classified via nearest-neighbor.
5. If VAD indicates silence or distance is too high, firmware prints `silence` or `unknown`.

Key files:
- Capture: `SpeechRecog.X/main.c` (Timer1 ISR + SRAM write)
- Features: `SpeechRecog.X/main.c` (compute_features)
- Templates: `SpeechRecog.X/speech_data.h`

## Sampling rate details
Timer1: CTC mode, prescaler /8, OCR1A computed as:
```
OCR1A = (F_CPU / (8 * 8000)) - 1
```
With F_CPU = 11059200, OCR1A = 171 (integer truncation).
Actual sampling rate is slightly above 8 kHz (~8037 Hz).

## External SRAM usage
- Samples are stored as signed int8 values.
- Max capture length (for feature extraction) is `CAPTURE_SAMPLES`.
- Firmware **only processes the first CAPTURE_SAMPLES**, even if more samples were recorded.

## ADC configuration
- ADMUX: AVCC reference, left-adjusted (ADLAR=1), using ADCH for 8-bit samples.
- ADC channel: default ADC0 (A0). No channel switching occurs in code.

## UART output
- 9600 baud, 8-N-1.
- If you see `?` or garbled characters, check UBRR calculation and wiring.
- If output prints old strings, the wrong hex is likely flashed.

## Feature extraction details (time-domain)
Frame size: 128 samples
Hop: 64 samples
VAD: `log-STE >= STE_FLOOR`

Per-frame features:
- **ZCR**: count of sign changes, scaled by `ZCR_SCALE` (256 / FFT_SIZE)
- **log-STE**: ilog2-scaled energy
- **MAV**: mean absolute value
- **P2P**: max - min
- **WL**: waveform length (sum of absolute diffs / (N-1))
- **SSC**: slope sign changes (d1*d2 < 0)
- **log-TEO**: Teager energy operator, log2 scaled
- **log-VAR**: variance, log2 scaled

Features are averaged across voiced frames only.

## Template generation (Python)
`script.py` produces `word_features.h` with the exact same feature pipeline.
It simulates the ADC path by:
- scaling audio to 8-bit range
- applying `ADC_CENTER = 63` offset
- using the same VAD settings and frame sizes

**Critical**: templates must be regenerated after any pipeline or constant changes.

## Tuning guidance (important)
These constants are the primary levers for accuracy:
- `STE_FLOOR`: raises/lower silence gating
- `MIN_VOICED_FRAMES`: stability vs responsiveness
- `DIST_THRESHOLD`: how often `unknown` is produced
- `CAPTURE_SAMPLES`: duration considered

Suggested tuning order:
1. Confirm mic DC bias (ADC idle value)
2. Tune `STE_FLOOR` so silence frames are skipped but speech kept
3. Tune `MIN_VOICED_FRAMES` to avoid noisy short bursts
4. Tune `DIST_THRESHOLD` after templates are updated

## Expected ranges (ballpark)
These are rough and will vary by mic and room:
- ZCR: 5–40
- log-STE: 90–140
- MAV: 5–35
- P2P: 20–120
- WL: 2–20
- SSC: 5–40
- log-TEO: 70–130
- log-VAR: 80–140

If values are near zero, the ADC bias or VAD threshold is likely wrong.

## Debug checklist (fast triage)
1. **UART prints change** after you rebuild/flash.
2. **INT0 toggles PD7** on button press.
3. **ADC idle value** is close to 63 (for 1.5 V bias).
4. **Feature values are non-zero** and stable.
5. **Templates regenerated** after pipeline changes.
6. **Distance threshold** is not too high (always same word) or too low (always unknown).

## Common mismatches to avoid
- Mixing templates from `features/` with `SpeechRecog.X`.
- Running an old `word_features.h` after changing `script.py`.
- Recording 12000 samples but only processing 4096 (ensure settings align).
- Using a different ADC center in firmware vs Python.

## Recommended debug outputs
To help debug quickly, enable (temporarily):
- `printf("Processing...\n");`
- `printf("feat: ...\n");`
- `printf("label=%s dist=%lu\n", labels[label], min_dist);`

Then remove or disable to avoid UART stalls during capture.

## MCU configuration summary
- Target: ATmega32
- F_CPU: 11059200
- Sampling: Timer1 CTC, prescaler /8, OCR1A computed for ~8 kHz
- ADC: left adjusted, AVCC reference, ADCH used (8-bit)
- External SRAM: SPI (CS on PB4, SCK PB7, MOSI PB5, MISO PB6)
- Recording trigger: INT0 (PD2) falling edge, toggles `recording` and PD7
- UART: 9600 baud (UBRR computed inside UART_init)

## Wiring notes (critical for debugging)
- **UART**: TX (adapter) -> PD0 (RXD), RX (adapter) -> PD1 (TXD), GND -> GND. No DTR/RTS.
- **INT0 button**: PD2 to GND when pressed, with pull-up enabled.
- **ADC input**: A0 (ADC0). Mic output biased around 1.5 V (ADC ~= 63).
- **SRAM SPI**: CS PB4, MOSI PB5, MISO PB6, SCK PB7.

## Exact build outputs
The MPLAB CMake build emits:
- HEX: `out/SpeechRecog/default.hex`
- ELF: `out/SpeechRecog/default.elf`

If you see old UART strings, you are likely flashing the wrong hex.

## How to regenerate templates (required after any pipeline change)
1. Run `script.py` in the repository root.
2. It writes `word_features.h` in the repo root.
3. Copy the entire contents of `word_features.h` into `SpeechRecog.X/speech_data.h`.
4. Rebuild and flash.

## Audio path / signal assumptions
- Mic output is connected to ADC0 (A0).
- Mic DC bias is around 1.5 V, which maps to about ADC=63 on an 8-bit read.
- Firmware subtracts 63 from ADC samples and clamps to int8 range.

## Active feature pipeline (time-domain)
Defined in `SpeechRecog.X/main.c`:

Constants:
- `FFT_SIZE = 128`
- `HOP_SIZE = 64`
- `CAPTURE_SAMPLES = 4096` (fixed-length segment used for features)
- `STE_FLOOR = 85` (VAD gate)
- `MIN_VOICED_FRAMES = 5`
- `DIST_THRESHOLD = 2500` (unknown rejection)

Feature vector length: `N_FEATURES = 8`

Per-frame features (averaged across voiced frames only):
1. ZCR (zero-crossing rate), scaled by `ZCR_SCALE`
2. log-STE (log2-scaled frame energy)
3. MAV (mean absolute value)
4. P2P (peak-to-peak amplitude)
5. WL (waveform length)
6. SSC (slope sign changes)
7. log-TEO (Teager energy operator, log2 scaled)
8. log-variance (log2 scaled)

VAD: frames with `log-STE < STE_FLOOR` are skipped. If fewer than `MIN_VOICED_FRAMES`, output is "silence" and classification is skipped.

Classification: nearest-neighbor Euclidean-like distance on feature vectors. If min distance is above `DIST_THRESHOLD`, result is "unknown".

## Python template generation
Use `script.py` to build templates for the **current** feature pipeline.

Important:
- The script uses the same sample rate (8 kHz), frame size (128), hop (64), and VAD gate as firmware.
- It converts audio to simulated 8-bit ADC values and centers around ADC_CENTER=63.
- It outputs `word_features.h` with `uint16_t` templates and a pointer table.

After running `script.py`, copy the contents of `word_features.h` into `SpeechRecog.X/speech_data.h` and rebuild/flash.

## Common failure modes observed
- **Wrong hex flashed**: Output strings not matching source changes. Verify `out/SpeechRecog/default.hex` is used.
- **UART silent/garbled**: UBRR must be computed from the argument; wrong baud or wiring causes garbage.
- **INT0 not firing**: PD2 must be pulled up and button must pull to GND (falling edge).
- **Always same word**: Templates do not match live feature scale or stale `speech_data.h`.
- **Zeros or tiny features**: VAD floor too high, mic bias wrong, or DC offset incorrect.
- **Board unresponsive with UART**: Wrong voltage levels or RX/TX wiring. Only connect GND/TX/RX; no DTR/RTS.

## Current values used in firmware (verify if debugging)
- ADC center subtraction: 63
- CAPTURE_SAMPLES: 4096
- STE_FLOOR: 85
- MIN_VOICED_FRAMES: 5
- DIST_THRESHOLD: 2500

## What to tell a debugging assistant
This README is meant to be self-contained. If you can include extra data, add:
- Latest `SpeechRecog.X/main.c`
- Latest `SpeechRecog.X/speech_data.h`
- UART output sample (Processing/feat lines)
- Exact mic wiring and idle ADC value (if measured)
- Which hex is flashed and how

## Notes on legacy code
The `features/` directory contains an older 13-feature MFCC-like pipeline and should not be mixed with the active time-domain pipeline unless explicitly switching back.
