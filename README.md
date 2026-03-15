# MIDIControls (Arduino Uno + MocoLUFA)

Modular multi-bank MIDI controller firmware for a jailbroken Arduino Uno.

## Features

- 3-mode blue workflow: Piano -> Drum -> Control -> Piano
- Control sub-bank selector using matrix S1-S4 (top row)
- Native serial MIDI workflow for MocoLUFA-jailbroken Uno (`31250` baud)
- Momentary shift layer (Yellow button held)
- Joystick pitch bend + octave transposition control
- Blue LED event engine: Piano OFF, Drum OFF, Control ON, long-hold one-shot bank indication
- IR remote transport control integration (play/stop/record/rew/ff)
- Dedicated serial debug build (`uno_debug`) with timestamped event logs
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
| Blue mode button | D12 | Mode switch + hold action | Short press cycles Piano -> Drum -> Control -> Piano; hold 1.5s arms indication, release blinks selected control bank once |
| Blue status LED | A4 | Mode indication | Piano OFF, Drum OFF, Control ON; 120ms OFF flash on switch; one-shot blink count = selected control bank (2..5) on long-hold release |
| Red action button | D11 | Record / monitor message | Sends `RED_BUTTON_ADDRESSES[0]` (default `MCU::RECORD`) and `RED_BUTTON_ADDRESSES[1]` (default `MCU::REC_RDY_1`) using configurable type (`CC` by default, value `127/0` on press/release) |
| Red feedback LED | A5 | DAW feedback state | Without shift shows global record state; while shift held shows selected-track monitor state (feedback-authoritative) |
| Yellow shift button | D10 | Shift layer select | Momentary hold (active only while pressed) |
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

- `Config.h`: centralized pins, mode constants, bank maps, timing thresholds, IR codes
- `main.cpp`: mode/bank orchestration, selector routing, shift and red action logic
- `ModeManager`: event-driven blue LED state machine via `millis()` (no `delay`)
- `IRHandler`: IR decode, command mapping, replay protection + resume handling
- `TransposedManyAddressNoteButtonMatrix`: custom adapter for matrix routing,
  transposer offset, and selector-key event interception in control mode

## Robustness Decisions

- Button/state logic is event-driven (`Button::Falling`) to avoid repeated triggers.
- Blue short/long behavior uses explicit non-blocking timing (`millis()`) with
  separate short-press and long-release paths.
- Joystick transpose uses threshold + lock + release-window hysteresis:
  - avoids rapid repeated octave changes from analog jitter.
- Mode LED logic is non-blocking and deterministic by mode state (OFF/OFF/ON)
  with one-shot indication sequence.
- IR handling includes:
  - repeat-frame ignore option
  - minimum dispatch interval guard
  - mandatory `IrReceiver.resume()` after each decode path

## Feature Coverage vs Brainstorm

- Implemented:
  - 3-mode blue cycle (Piano -> Drum -> Control)
  - Control sub-bank selection by S1/S2/S3/S4 -> banks 2/3/4/5
  - Momentary yellow shift layer for alternate controls
  - Red button dual behavior (`RECORD` / configurable shift-monitor action)
  - Joystick X pitch bend + joystick Y modulation/transposition behavior
  - Blue LED event behavior: OFF/OFF/ON + one-shot hold-release indication
  - IR receiver transport command injection
  - Human-readable serial debug environment with timestamped event logs
- Not fully implemented yet (hardware/DAW validation pending):
  - final Waveform 13 mapping confirmation for shift-monitor action
  - physical verification steps in `IMPLEMENTATION.md`
  - advanced red LED feedback from DAW state
  - richer per-DAW control-bank mapping presets beyond current baseline

## Debug Mode

Use `uno_debug` when you need readable serial diagnostics.

- Build/upload environment: `uno_debug`
- Serial monitor speed: `115200`
- Output style: timestamped, event-driven logs (button edges, mode changes,
  control-bank selections, red/shift actions, joystick/analog significant deltas)
- In debug mode MIDI serial output is disabled intentionally.

## Waveform 13 Mapping Notes (Red/LED)

- Configure the custom control surface with both MIDI input and MIDI output.
- Red (no shift) command uses `RED_BUTTON_ADDRESSES[0]`; feedback must be sent
  by Waveform to `RED_RECORD_FEEDBACK_ADDRESS` for LED sync.
- Shift+Red command uses `RED_BUTTON_ADDRESSES[1]`; feedback must be sent by
  Waveform to `RED_MONITOR_FEEDBACK_ADDRESS` for monitor-state LED preview.
- LED policy is feedback-authoritative: no DAW feedback means no LED state
  update.

## Matrix Layouts

The physical matrix is 4 rows × 4 columns. **Col 0 is the rightmost physical
column.** In piano mode col 0 holds octave controls (no note sent). In control
mode col 0 selects the active sub-bank (S1–S4 top to bottom, no note sent).
All tables show physical orientation: left = Col 3, right = Col 0.

### Piano mode — Channel 1

Base notes at transpose offset 0. Transpose range ±2 octaves via S1/S2.
S1 and S2 change the octave only — no MIDI note is sent when pressed.

| | ← Left (Col 3) | Col 2 | Col 1 | Right (Col 0) → |
|---|---|---|---|---|
| **Row 0** | 65 — F4 | 72 — C5 | 56 — G#3 | **S1 — Octave UP** |
| **Row 1** | 64 — E4 | 71 — B4 | 73 — C#5 | **S2 — Octave DOWN** |
| **Row 2** | 62 — D4 | 69 — A4 | 63 — D#4 | 68 — G#4 |
| **Row 3** | 60 — C4 | 67 — G4 | 61 — C#4 | 66 — F#4 |

### Drum mode — Channel 1

Standard GM drum map. All 16 pads send notes normally in drum mode.

| | ← Left (Col 3) | Col 2 | Col 1 | Right (Col 0) → |
|---|---|---|---|---|
| **Row 0** | 37 — Side Stick | 44 — Pedal Hi-Hat | 56 — Cowbell | 52 — Chinese Cym |
| **Row 1** | 38 — Acoustic Snare | 42 — Closed Hi-Hat | 46 — Open Hi-Hat | 51 — Ride Cym 1 |
| **Row 2** | 35 — Acoustic BD | 40 — Electric Snare | 47 — Low-Mid Tom | 49 — Crash Cym 2 |
| **Row 3** | 36 — Bass Drum 1 | 39 — Hand Clap | 45 — Low Tom | 50 — High Tom |

### Control banks — Channel 16

Col 0 (right column) = bank sub-selector buttons S1–S4 — these switch the
active control bank and send **no note**. Cols 1–3 send notes for DAW
MIDI-learn mapping.

#### Bank 2 (DAW) — selected by S1

| | ← Left (Col 3) | Col 2 | Col 1 | Right (Col 0) → |
|---|---|---|---|---|
| **Row 0** | 124 | 125 | 126 | S1 → Bank 2 |
| **Row 1** | 120 | 121 | 122 | S2 → Bank 3 |
| **Row 2** | 116 | 117 | 118 | S3 → Bank 4 |
| **Row 3** | 112 | 113 | 114 | S4 → Bank 5 |

#### Bank 3 (Track A) — selected by S2

| | ← Left (Col 3) | Col 2 | Col 1 | Right (Col 0) → |
|---|---|---|---|---|
| **Row 0** | 108 | 109 | 110 | S1 → Bank 2 |
| **Row 1** | 104 | 105 | 106 | S2 → Bank 3 |
| **Row 2** | 100 | 101 | 102 | S3 → Bank 4 |
| **Row 3** | 96 | 97 | 98 | S4 → Bank 5 |

#### Bank 4 (Track B) — selected by S3

| | ← Left (Col 3) | Col 2 | Col 1 | Right (Col 0) → |
|---|---|---|---|---|
| **Row 0** | 92 | 93 | 94 | S1 → Bank 2 |
| **Row 1** | 88 | 89 | 90 | S2 → Bank 3 |
| **Row 2** | 84 | 85 | 86 | S3 → Bank 4 |
| **Row 3** | 80 | 81 | 82 | S4 → Bank 5 |

#### Bank 5 (Track C) — selected by S4

| | ← Left (Col 3) | Col 2 | Col 1 | Right (Col 0) → |
|---|---|---|---|---|
| **Row 0** | 76 | 77 | 78 | S1 → Bank 2 |
| **Row 1** | 72 | 73 | 74 | S2 → Bank 3 |
| **Row 2** | 68 | 69 | 70 | S3 → Bank 4 |
| **Row 3** | 64 | 65 | 66 | S4 → Bank 5 |

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
