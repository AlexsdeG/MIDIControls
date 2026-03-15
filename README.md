# MIDIControls (Arduino Uno + MocoLUFA)

Modular multi-bank MIDI controller firmware for a jailbroken Arduino Uno.

## Features

- Multi-bank 4x4 matrix controller (Piano, Drum, DAW, Track-control banks)
- Native serial MIDI workflow for MocoLUFA-jailbroken Uno (`31250` baud)
- Shift layer (Yellow button) for alternate knob/button behavior
- Joystick pitch bend + octave transposition control
- Blue LED mode-state engine (off/solid/blink patterns, non-blocking)
- IR remote transport control integration (play/stop/record/rew/ff)
- Centralized `Config.h` for fast pin/mapping tuning

## What You Need To Start

### Option A — Arduino Uno (Jailbroken for USB-MIDI)

Use this if you want Uno hardware with USB-MIDI through MocoLUFA.

- Arduino Uno flashed with MocoLUFA-compatible firmware
- This project firmware running on the Uno at serial MIDI `31250`
- Pins `0` and `1` must stay physically free

### Option B — Arduino Leonardo (or other native USB-MIDI board)

This can work out-of-the-box for USB-MIDI hardware-wise, but this project is currently configured for Uno serial MIDI.

- For Leonardo use, you would switch the MIDI interface in code (e.g., USB MIDI interface)
- Pin mapping and timing logic remain portable, but transport layer must be adjusted

## Recommended Workflow (Tested)

Use **PlatformIO in VS Code** (tested setup for this project).

- Open folder in VS Code
- Use PlatformIO sidebar/tooling
- Build with the bottom toolbar command: **PlatformIO: Build**
- Upload with: **PlatformIO: Upload**

This project is **multi-file** (`main.cpp`, `ModeManager.*`, `IRHandler.*`, custom headers in `include/`).
PlatformIO builds and links all source files automatically; do not compile single files manually.

## Component Map

| Component | Pins | Current Feature | Description |
|---|---|---|---|
| Blue mode button | D12 | Main bank/page switch | Advances through banks 0..5 (wrapped) |
| Blue status LED | A4 | Mode indication | Bank 0 off, bank 1 solid on, bank 2..5 blink count = bank index |
| Red action button | D11 | Record / Undo | Sends `MCU::RECORD` (normal) and `MCU::UNDO` (with Yellow shift) |
| Red feedback LED | A5 | Reserved/next step | Pin reserved in config for DAW feedback integration |
| Yellow shift button | D10 | Shift layer select | Toggles shift bank used by rotary/button alternate mappings |
| Matrix row R1 | D5 | 4x4 matrix scan | Note matrix row pin |
| Matrix row R2 | D4 | 4x4 matrix scan | Note matrix row pin |
| Matrix row R3 | D3 | 4x4 matrix scan | Note matrix row pin |
| Matrix row R4 | D2 | 4x4 matrix scan | Note matrix row pin |
| Matrix col C1 | D6 | 4x4 matrix scan | Note matrix column pin |
| Matrix col C2 | D7 | 4x4 matrix scan | Note matrix column pin |
| Matrix col C3 | D8 | 4x4 matrix scan | Note matrix column pin |
| Matrix col C4 | D9 | 4x4 matrix scan | Note matrix column pin |
| Rotary knob 1 | A0 | CC banked output | `Pan` (normal) / `Expression` (shift) |
| Rotary knob 2 | A1 | CC banked output | `Channel Volume` (normal) / `Effect Control 1` (shift) |
| Joystick X (VRx) | A3 | Pitch bend | Sends pitch bend on Channel 1 |
| Joystick Y (VRy) | A2 | Mod + transpose control | Mod wheel when not shifted; octave transpose trigger when shifted |
| IR receiver | D13 | Remote transport control | Decodes remote codes and triggers MCU transport notes |

## Platform & Transport

- Board: Arduino Uno (`atmelavr`)
- MIDI transport: `HardwareSerialMIDI_Interface midi {Serial, 31250};`
- MocoLUFA requirement: keep pins `0/1` physically free (serial collision risk)
- Build stack: PlatformIO + Arduino framework
- Libraries: MIDI Library 5.x, Control Surface 2.1.x, IRremote 4.6.x

## Current Architecture

- `Config.h`: centralized pins, bank maps, joystick thresholds, IR codes
- `main.cpp`: object graph + non-blocking loop orchestration
- `ModeManager`: blue LED mode state machine via `millis()` (no `delay`)
- `IRHandler`: IR decode, command mapping, replay protection + resume handling
- `TransposedManyAddressNoteButtonMatrix`: custom adapter to combine
  - 6-bank matrix addressing and
  - transposer offset

## Robustness Decisions

- Button/state logic is event-driven (`Button::Falling`) to avoid repeated triggers.
- Main-bank cycling uses explicit wrap logic: `(current + 1) % MAIN_BANK_COUNT`.
- Joystick transpose uses threshold + lock + release-window hysteresis:
  - avoids rapid repeated octave changes from analog jitter.
- Mode LED logic is non-blocking and deterministic by bank index.
- IR handling includes:
  - repeat-frame ignore option
  - minimum dispatch interval guard
  - mandatory `IrReceiver.resume()` after each decode path

## Feature Coverage vs Brainstorm

- Implemented:
  - Yellow shift bank for alternate controls
  - Red button dual behavior (`RECORD` / `UNDO`)
  - 6-bank matrix addressing (piano, drum, DAW, tracks)
  - Joystick X pitch bend + joystick Y modulation/transposition behavior
  - Blue LED bank-mode state signaling with blink patterns
  - IR receiver transport command injection
- Not fully implemented yet (hardware/DAW validation pending):
  - physical verification steps in `IMPLEMENTATION.md`
  - advanced red LED feedback from DAW state
  - richer control-bank DAW mapping beyond current baseline

## Build & Flash (CLI equivalent)

```bash
source .venv/bin/activate
pio run
pio run -t upload --upload-port /dev/ttyACM0
```

If upload fails, verify the actual port (`/dev/ttyACM*` or `/dev/ttyUSB*`) and user udev permissions.

## Verification Workflow

Use `src/IMPLEMENTATION.md` as the execution checklist.

- Code phases (2–5) are implemented and compile-clean.
- Remaining unchecked items are physical integration tests (button/LED/DAW/IR behavior on real hardware).
