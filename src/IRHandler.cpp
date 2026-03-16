#include "IRHandler.h"

// Arduino core must be first so IRremote finds __FlashStringHelper and F().
#include <Arduino.h>
// IRremote must be included BEFORE Control_Surface.
// Control_Surface defines global DEBUG/TRACE/INFO macros; if IRremote sees
// them it enables its own debug printing to Serial, which on a MIDI serial
// port appears as a burst of random MIDI messages on every IR press.
#ifdef DEBUG
#undef DEBUG
#endif
#ifdef TRACE
#undef TRACE
#endif
#ifdef INFO
#undef INFO
#endif
#include <IRremote.hpp>
#include <Control_Surface.h>

#include "Config.h"
#include "Debug.h"

namespace {
constexpr uint8_t INVALID_CC = 0;
}

void IRHandler::begin() {
  IrReceiver.begin(static_cast<uint_fast8_t>(Config::PIN_IR.pin),
                   DISABLE_LED_FEEDBACK);
  DBG_TS_VAL("IRHandler: receiver started on pin", Config::PIN_IR.pin);
  DBG_TS_MSG("IRHandler: LED feedback disabled");
}

uint8_t IRHandler::commandToCC(uint16_t command) const {
  const uint8_t cc =
      static_cast<uint8_t>((command & 0x7F) ^ ((command >> 7) & 0x7F));
  return cc == 0 ? 127 : cc;
}

void IRHandler::sendDAWCC(uint8_t cc) const {
  if (cc == INVALID_CC) {
    return;
  }

  const MIDIAddress address {cc, Config::IR_MIDI_CHANNEL};
  DBG_TS_VAL("IRHandler: sending MIDI CC", cc);
  DBG_TS_VAL("IRHandler: channel", address.getChannel().getOneBased());
  Control_Surface.sendControlChange(address, Config::IR_CC_VALUE_ON);
}

void IRHandler::update() {
  if (!IrReceiver.decode()) {
    return;
  }

  const auto command = static_cast<uint16_t>(IrReceiver.decodedIRData.command);
  const auto address = static_cast<uint16_t>(IrReceiver.decodedIRData.address);
  (void)address;
  const auto flags = IrReceiver.decodedIRData.flags;
  const bool isRepeat =
      (flags & IRDATA_FLAGS_IS_REPEAT) != 0;
  const bool isOverflow =
      (flags & IRDATA_FLAGS_WAS_OVERFLOW) != 0;
  const bool parityFailed =
      (flags & IRDATA_FLAGS_PARITY_FAILED) != 0;
  const auto now = millis();
  const bool withinDebounce =
      (command == lastSentCommand) &&
      ((now - lastSentAt) < Config::IR_MIN_COMMAND_INTERVAL_MS);

  DBG_TS_IR("IRHandler: decoded", address, command);
  if (isRepeat) DBG_PRINTLN("  → repeat flag set");
  if (isOverflow) DBG_PRINTLN("  → overflow flag set");
  if (parityFailed) DBG_PRINTLN("  → parity failed");

  if ((Config::IR_IGNORE_REPEAT && isRepeat) || isOverflow || parityFailed ||
      withinDebounce) {
    if (isRepeat)
      DBG_TS_MSG("IRHandler: ignored (repeat)");
    if (isOverflow)
      DBG_TS_MSG("IRHandler: ignored (overflow)");
    if (parityFailed)
      DBG_TS_MSG("IRHandler: ignored (parity failed)");
    if (withinDebounce)
      DBG_TS_MSG("IRHandler: ignored (debounce interval)");
    IrReceiver.resume();
    return;
  }

  const uint8_t cc = commandToCC(command);
  sendDAWCC(cc);
  lastSentCommand = command;
  lastSentAt = now;

  IrReceiver.resume();
}
