# IMPLEMENTATION.md

## 1. Current Runtime Model

### Modes (Blue short press)
- `Piano` (`bank 0`) -> `Drum` (`bank 1`) -> `Control` (`bank 2..5`) -> `Piano`
- Blue button no longer cycles all banks directly.

### Control sub-banks (Matrix selectors)
- In `Control` mode only:
  - `S1` (row1,col1) => `bank 2`
  - `S2` (row1,col2) => `bank 3`
  - `S3` (row1,col3) => `bank 4`
  - `S4` (row1,col4) => `bank 5`
- Selector keys are intercepted and used for bank switching in control mode.

### Blue LED behavior
- `Piano`: LED OFF
- `Drum`: LED OFF
- `Control`: LED ON
- Any mode/bank switch: LED forced OFF for `120ms` then returns to mode idle state
- Blue long-hold (`>= 1500ms`):
  - while held: LED ON (armed indicator)
  - on release: one-shot blink count = selected control bank (`2..5`)
  - then returns to idle state for current mode

### Yellow shift + red action
- Yellow is **momentary** (active only while held)
- Red action:
  - without shift: sends `RED_BUTTON_ADDRESSES[0]` (default `MCU::RECORD`)
  - with shift: sends `RED_BUTTON_ADDRESSES[1]` (default `MCU::REC_RDY_1`)
  - message type is configurable; default is Control Change (`127` on press, `0` on release)
- Red LED (feedback-authoritative):
  - without shift: shows DAW global record state from `RED_RECORD_FEEDBACK_ADDRESS`
  - while shift held: shows selected-track monitor state from `RED_MONITOR_FEEDBACK_ADDRESS`
  - no local optimistic toggle is applied if DAW feedback is missing

### Joystick transpose
- Shift held + joystick Y high/low triggers transpose up/down with lock+hysteresis
- Shift released resets transpose lock

## 2. Debug Environment

### Build environments
- `uno` = production MIDI mode (`Serial` used for MIDI at `31250`)
- `uno_debug` = serial debug mode (`115200`, human-readable logs, MIDI disabled)

### Logged events
- Blue/Yellow/Red button edges
- Blue hold arm and release actions
- Mode + bank transitions and trigger reason
- Control selector S1-S4 events
- Red action routing and MIDI payload
- Joystick transpose state transitions
- ModeManager LED phase transitions
- Significant analog deltas for rotaries/joystick
- IR decode dispatch and filtering decisions

## 3. DAW Validation Script (Waveform 13)

1. Flash `uno_debug`, open serial monitor `115200`.
2. Press Blue quickly: verify `Piano -> Drum -> Control -> Piano` in log.
3. Enter Control, press S1/S2/S3/S4: verify bank `2/3/4/5` selections.
4. Hold Blue for ~1.5s: verify `armed` log while held, then one-shot blink log on release.
5. Hold Yellow and move rotaries/joystick: verify shift activity + analog events.
6. Press Red without shift: verify `MCU::RECORD` dispatch.
7. Press Shift+Red: verify shifted message dispatch (`RED_BUTTON_ADDRESSES[1]`).
8. Verify red LED state follows DAW feedback:
  - global record when shift not held
  - selected-track monitor while shift is held
9. If LED does not update, confirm Waveform sends matching feedback for
  `RED_RECORD_FEEDBACK_ADDRESS` and `RED_MONITOR_FEEDBACK_ADDRESS`.
10. If monitor action fails in Waveform, update `RED_BUTTON_ADDRESSES[1]` in
   `Config.h`, rebuild, retest.

## 4. Known Open Points

- Waveform profile mapping for input-monitor toggle may need tuning per project/session setup.
- Red LED DAW feedback loop is still reserved/not implemented.
- Optional future work: track selection + explicit selected-track monitor toggle semantics.
