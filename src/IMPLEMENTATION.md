# IMPLEMENTATION.md

## 1. Project Context & Architecture
**Goal:** Develop a modular, multi-bank MIDI controller firmware for an Arduino Uno using the MocoLUFA USB-MIDI jailbreak. The system features a 4x4 button matrix, analog inputs (joystick, rotaries), an IR receiver for remote control, and a complex state machine for nested banking (Play/Drum/Control modes) with custom LED feedback.

**Tech Stack & Dependencies:**
- **Environment:** PlatformIO (Core/IDE)
- **Framework:** Arduino
- **Dependencies (platformio.ini):**
  - `lib_deps = fortyseveneffects/MIDI Library @ ^5.0.2, z3t0/IRremote @ ^4.6.2, tttapa/Control Surface @ ^2.1.0`
- **Hardware Protocol:** `HardwareSerialMIDI_Interface` at exactly 31250 Baud (MocoLUFA requirement).

**File Structure:**

```text
MidiController/
├── platformio.ini
├── include/
│   └── Config.h           # All pin definitions, MIDI addresses, and MCU constants
└── src/
    ├── main.cpp           # Entry point, Control_Surface loop, object instantiation
    ├── ModeManager.h      # State machine for 6-bank system and LED blink logic
    ├── ModeManager.cpp    
    ├── IRHandler.h        # Wrapper for IRremote to inject MCU commands
    └── IRHandler.cpp      

```

**Attention Points:**

* **Pin 0 and Pin 1 MUST remain unconnected** to avoid hardware serial collisions with the MocoLUFA firmware.
* **Non-blocking Code:** The `ModeManager` MUST use `millis()` for LED blinking. `delay()` will block the `Control_Surface.loop()` and drop MIDI packets.
* **Joystick Transposition:** Transposing octaves must happen via the `Transposer` class linked to the Yellow Shift button + Joystick, NOT by dynamically rewriting matrix addresses.

---

## 2. Execution Phases

#### Phase 1: Project Scaffolding & Hardware Configuration

* [x] **Step 1.1:** Initialize a new PlatformIO project for `board = uno`.
* [x] **Step 1.2:** Add `Control Surface` and `IRremote` to `platformio.ini` dependencies.
* [x] **Step 1.3:** Create `include/Config.h`. Define all `constexpr pin_t` values (Matrix R1-R4 to D5-D2, C1-C4 to D6-D9, BlueBtn=D12, RedBtn=D11, YellowBtn=D10, BlueLED=A4, RedLED=A5, JoyX=A3, JoyY=A2, Rot1=A0, Rot2=A1, IR=D13).
* [x] **Step 1.4:** In `Config.h`, define the 6-bank 3D array (`AddressMatrix<4, 4>`) mapping Bank 0 (Piano), Bank 1 (Drums), Bank 2 (DAW), and Banks 3-5 (Track Controls).
* [x] **Verification:** Run `pio run`. Verify the project compiles successfully with zero warnings regarding missing headers.

#### Phase 2: Core MIDI Routing & Shift Logic

* [x] **Step 2.1:** In `src/main.cpp`, instantiate `HardwareSerialMIDI_Interface midi {Serial, 31250};`.
* [x] **Step 2.2:** Instantiate `Bank<6> mainBank;` and `Bank<2> shiftBank;`.
* [x] **Step 2.3:** Instantiate the `Bankable::NoteButtonMatrix` passing `mainBank` and the arrays from `Config.h`.
* [x] **Step 2.4:** Instantiate `IncrementSelector<2>` for the Yellow Button (Shift) tied to `shiftBank`.
* [x] **Step 2.5:** Instantiate `Bankable::CCPotentiometer` for the rotaries, mapping array index 0 to standard CC and index 1 to secondary CC (Shift active).
* [x] **Step 2.6:** Instantiate `Bankable::NoteButton` for the Red Button (Index 0: `MCU::RECORD`, Index 1: `MCU::UNDO`).
* [ ] **Verification:** Upload code (with physical jumper on D4/D6). Remove jumper, reboot, and use a MIDI Monitor. Verify pressing the Yellow button alters the CC output of the rotaries.

#### Phase 3: Joystick & Octave Transposition

* [x] **Step 3.1:** In `main.cpp`, instantiate a `Transposer<-2, 2>` object.
* [x] **Step 3.2:** Link the `Transposer` to the `Bankable::NoteButtonMatrix` to enable octave shifting.
* [x] **Step 3.3:** Create custom logic in the `loop()`: Read the Joystick Y analog value. If `shiftBank.getActiveBank() == 1` AND Joystick > 800, increment Transposer. If Joystick < 200, decrement Transposer. Add a boolean debounce lock to prevent rapid firing.
* [x] **Step 3.4:** Map Joystick X to a `PBPotentiometer` (Pitch Bend) and Joystick Y (when Shift is NOT pressed) to a `CCPotentiometer` (Modulation).
* [ ] **Verification:** Open DAW. Verify Joystick X bends pitch. Hold Yellow Button and push Joystick Y up. Verify Matrix outputs shift up by 12 semitones.

#### Phase 4: Mode Manager & LED Blink Logic

* [x] **Step 4.1:** Create `src/ModeManager.h` and `src/ModeManager.cpp`.
* [x] **Step 4.2:** Implement a class `ModeManager`. Pass the `mainBank` reference to it.
* [x] **Step 4.3:** Implement an `update()` method using `millis()` state machine logic.
* If Bank == 0 (Play): LED OFF.
* If Bank == 1 (Drum): LED ON.
* If Bank >= 2 (Control): LED blinks N times corresponding to the bank number (2, 3, 4, or 5), pauses, and repeats.


* [x] **Step 4.4:** In `main.cpp`, link the Blue Button to iterate `mainBank.selectNext()`.
* [x] **Step 4.5:** Call `modeManager.update()` inside the `main.cpp` `loop()`.
* [ ] **Verification:** Press the Blue Button. Verify the Blue LED correctly transitions between OFF, Solid ON, and complex blink patterns without delaying the MIDI output of the matrix.

#### Phase 5: IR Receiver Integration

* [x] **Step 5.1:** Create `src/IRHandler.h` and `src/IRHandler.cpp`.
* [x] **Step 5.2:** Initialize `IrReceiver.begin(Config::PIN_IR)` in the class setup.
* [x] **Step 5.3:** Implement an `update()` method. If `IrReceiver.decode()` is true, map specific hex values (hardcoded in `Config.h` based on the user's remote) to `Control_Surface.sendNoteOn(MCU::PLAY)`, etc.
* [x] **Step 5.4:** Call `IrReceiver.resume()` after processing.
* [x] **Step 5.5:** Inject `irHandler.update()` into the `main.cpp` `loop()`.
* [ ] **Verification:** Press 'Play' on the physical IR remote. Verify the DAW starts playback via MCU integration.

---

## 3. Global Testing Strategy

* **Matrix Ghosting Test:** Press pads S1, S2, and S5 simultaneously. Verify S6 is NOT falsely triggered (requires hardware diodes if ghosting occurs, software cannot fix physical shorts).
* **Concurrency Test:** Rapidly rotate both knobs while mashing the Yellow Shift button and pressing matrix pads. Ensure the `loop()` execution time remains under 5ms and no MIDI CC packets are dropped.
* **IR Blocking Test:** Spam the IR remote while actively playing a melody on the matrix. Ensure `IRremote` interrupts do not cause jitter or missed `NoteOff` messages in the `Control_Surface` stream.
* **DAW Sync Test:** Manually click 'Record' and 'Input Monitor' inside Waveform 13 using the mouse. Verify the Red LED on the Arduino immediately updates its state via MCU feedback.
