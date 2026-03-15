#pragma once

// =============================================================================
// Debug output helpers
//
// The Arduino Uno has only ONE hardware UART (Serial / pins 0+1).
// In normal operation that UART carries MIDI at 31250 baud via MocoLUFA.
// Anything printed to Serial in that mode appears as garbage in the monitor
// because the host computer interprets MIDI bytes as ASCII.
//
// In DEBUG mode MIDI is disabled and Serial runs at 115200 baud so you can
// read the log output in the Serial Monitor.
//
// HOW TO DEBUG
//   1. Flash the ATmega16U2 back to the standard Arduino CDC (serial) firmware.
//   2. Build + upload using the [env:uno_debug] environment in platformio.ini.
//   3. Open the Serial Monitor at 115200 baud.
//   4. When done: re-flash 16U2 with MocoLUFA, then upload [env:uno].
//
// The `debug` constant below mirrors the "if (debug) { … }" style the sketch
// uses.  The compiler eliminates all dead branches at -Os, so there is zero
// code / RAM cost when debug == false.
// =============================================================================

#ifdef DEBUG_SERIAL
  static constexpr bool debug = true;
#else
  static constexpr bool debug = false;
#endif

// --------------------------------------------------------------------------
// debugBegin() – call FIRST in setup() before anything that touches Serial.
// --------------------------------------------------------------------------
inline void debugBegin() {
#ifdef DEBUG_SERIAL
  Serial.begin(115200);
  while (!Serial) { ; }           // wait for USB-CDC enumeration
  Serial.println(F(""));
  Serial.println(F("╔══════════════════════════════════╗"));
  Serial.println(F("║  MIDIControls  DEBUG MODE         ║"));
  Serial.println(F("║  MIDI output is DISABLED          ║"));
  Serial.println(F("║  Serial Monitor @ 115200 baud     ║"));
  Serial.println(F("╚══════════════════════════════════╝"));
  Serial.println(F(""));
  Serial.flush();
#endif
}

// --------------------------------------------------------------------------
// Macros – all evaluate to no-ops when DEBUG_SERIAL is not defined so they
// add zero bytes to the normal (MIDI) firmware.
// --------------------------------------------------------------------------

#ifdef DEBUG_SERIAL

  // String-literal printed from flash (saves SRAM on AVR).
  #define DBG_PRINT(msg)        Serial.print(F(msg))
  #define DBG_PRINTLN(msg)      Serial.println(F(msg))

  // "key: value\n"  – key must be a string literal; val can be any printable type.
  #define DBG_VAL(key, val) \
      do { Serial.print(F(key ": ")); Serial.println(val); } while (0)

  // "[@Xms] " timestamp prefix (prints inline, no newline).
  #define DBG_TS() \
      do { Serial.print(F("[@")); Serial.print(millis()); Serial.print(F("ms] ")); } while (0)

  // "[@Xms] message\n"
  #define DBG_TS_MSG(msg) \
      do { DBG_TS(); Serial.println(F(msg)); } while (0)

  // "[@Xms] key: value\n"
  #define DBG_TS_VAL(key, val) \
      do { DBG_TS(); Serial.print(F(key ": ")); Serial.println(val); } while (0)

  // "[@Xms] key: 0xHH\n"  – for byte-sized values printed in hex.
  #define DBG_TS_HEX(key, val) \
      do { DBG_TS(); Serial.print(F(key ": 0x")); Serial.println((val), HEX); } while (0)

  // Print two hex values on one line: "[@Xms] label  addr=0xHH  cmd=0xHH\n"
  #define DBG_TS_IR(label, addr, cmd) \
      do { DBG_TS(); Serial.print(F(label "  addr=0x")); Serial.print((addr), HEX); \
           Serial.print(F("  cmd=0x")); Serial.println((cmd), HEX); } while (0)

  // Horizontal separator for readability.
  #define DBG_SEP() \
      Serial.println(F("--------------------------------------------------"))

#else  // ---- non-debug: all macros vanish -----------------------------------

  #define DBG_PRINT(msg)              do {} while (0)
  #define DBG_PRINTLN(msg)            do {} while (0)
  #define DBG_VAL(key, val)           do {} while (0)
  #define DBG_TS()                    do {} while (0)
  #define DBG_TS_MSG(msg)             do {} while (0)
  #define DBG_TS_VAL(key, val)        do {} while (0)
  #define DBG_TS_HEX(key, val)        do {} while (0)
  #define DBG_TS_IR(label, addr, cmd) do {} while (0)
  #define DBG_SEP()                   do {} while (0)

#endif
