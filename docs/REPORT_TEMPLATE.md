Report Template - Embedded Speech Recognition (ATmega32A)
=========================================================

1. System Architecture
- Block diagram (microphone -> preamp/filter -> ADC -> DSP -> classifier -> outputs)
- MCU: ATmega32A, clock frequency
- Sampling rate and resolution

2. Feature Extraction Method
- Window size, frame size, and number of frames
- Feature definitions: ZCR, STE (and any additional features if used)
- Offline training approach and average template creation

3. Classification Method
- Template matching method (Euclidean distance or DTW-lite)
- How minimum distance is selected

4. Accuracy Results
- Dataset summary (number of samples per word)
- Accuracy table for each word
- Confusion matrix (optional)

5. Optimization Techniques
- Memory usage (Flash/RAM), PROGMEM usage
- CPU considerations (8 kHz ISR, per-frame computations)
- Any fixed-point or simplified math strategies

6. Testing & Evaluation
- Latency (time from speech end to output)
- False detection rate
- Noise robustness testing (quiet vs noisy)

7. Conclusions & Future Work
- What worked, what didn't
- Proposed improvements

Appendix
- Wiring diagram or schematic
- Build/flash instructions
