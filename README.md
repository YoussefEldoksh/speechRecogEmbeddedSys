speechRecogEmbeddedSys
======================

Short project overview
- Small embedded keyword-spotting system for ATmega32A.
- Host tools (Python + librosa) extract two short-term features per file: ZCR (zero-crossing rate) and STE (short-time energy) and emit a generated C header used by the MCU.
- MCU code samples audio at 8 kHz, computes ZCR+STE over a few windows, then classifies against template features stored in PROGMEM.

Supported hardware
- MCU: ATmega32A (11.0592 MHz crystal assumed)
- Microphone: analog microphone module (e.g., MAX9814/MAX4466-based breakout). These modules provide an analog output; some boards also have a digital AGC/VAD pin.
- Peripherals used: ADC channel 0 for microphone input (default), PORTB LEDs for output, an optional INT0 pin for an external digital VAD.

Wiring (recommended)
- Microphone analog output -> ADC0 (PORTA0) on ATmega32A.
- Microphone GND -> GND.
- Microphone VCC -> +5V (check module voltage requirement).
- If your module provides a digital VAD / DOUT pin and you want immediate interrupts: DOUT -> INT0 (PD2).
- LCD connections are project-specific; follow existing `lcd.h` pin comments in the code.

Important MCU configuration notes
- ADC reference: code configures AVCC as reference (AREF with external capacitor). Keep AREF pin connected to ground through capacitor as typical.
- ADC prescaler: set to 128 in `adc/adc.c` so ADC clock is ~86 kHz for F_CPU=11.0592 MHz.
- Sampling: timer0 CTC mode triggers sampling at ~8 kHz (OCR0 = 21). This is implemented in `SpeechRecog.X/speech_processing.c` and `adc/speech_processing.c`.
- Data representation: features are 2 Q15 fixed-point integers (ZCR, STE). The host script can produce a generated header `SpeechRecog.X/generated_features.h` which the MCU includes automatically if present.

Build instructions (command-line)
- Requirements: avr-gcc, avr-libc, avrdude (or use NetBeans project `SpeechRecog.X`).
- To build using the NetBeans project, open `SpeechRecog.X` in NetBeans and build.
- To build on the CLI (example):

```bash
# Example (tweak paths/MCU as needed):
avr-gcc -mmcu=atmega32 -DF_CPU=11059200UL -Os -Wall -Wextra -I SpeechRecog.X -I adc \
	-c SpeechRecog.X/speech_processing.c -o build/speech_processing.o
# link and create .elf/.hex as usual
```

Generating / updating feature templates
- Run the host script to convert audio and generate `SpeechRecog.X/generated_features.h`:

```bash
python script.py
```

- The script writes `features.txt` and `SpeechRecog.X/generated_features.h`. When `generated_features.h` is present, the MCU code will use it at compile time.

Tuning tips for MAX mic + ATmega32A
- Thresholds: the `THRESHOLD` constants in code were set experimentally. If you get spurious activation, increase `THRESHOLD`. If you miss speech, decrease it.
- ADC range: some breakout modules output around mid-rail (Vcc/2) — code centers samples around 512.
- If your microphone module has a digital VAD output, consider wiring it to `INT0` (PD2) and enabling external interrupts for faster wake-up.

Next steps / suggestions
- If you want to reduce memory further, ensure you build with `-Os -ffunction-sections -fdata-sections -Wl,--gc-sections -flto` and keep UART/printf disabled unless needed.
- Add a small CI/build script or top-level Makefile to build the NetBeans project from CLI.
- Add wiring diagram image to repo.

Contact
- If you want, tell me whether you prefer (A) keep floats and compile as-is, (B) convert to 16-bit fixed-point (recommended for AVR), or (C) I should wire the generated header into the build and run a local compile check (I can't flash your board but I can run compilation checks).

