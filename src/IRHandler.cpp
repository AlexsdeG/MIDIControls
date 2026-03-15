#include "IRHandler.h"

#include <Control_Surface.h>
#include <IRremote.hpp>

#include "Config.h"
#include "Debug.h"

namespace {
constexpr uint8_t TRANSPORT_VELOCITY = 0x7F;
}

void IRHandler::begin() {
  // BUG-FIX: ENABLE_LED_FEEDBACK with USE_DEFAULT_FEEDBACK_LED_PIN resolves to
  // LED_BUILTIN (pin 13 on Uno), which is the SAME pin as the IR receiver.
  // Having IRremote drive pin 13 as OUTPUT while also reading it as INPUT
  // caused spurious decodes and feedback loops.  Always use DISABLE_LED_FEEDBACK.
  IrReceiver.begin(static_cast<uint_fast8_t>(Config::PIN_IR.pin),
                   DISABLE_LED_FEEDBACK);
  DBG_TS_MSG("IRHandler: receiver started on pin 13 (LED feedback disabled)");
}

void IRHandler::sendMCUTransport(uint8_t noteNumber) const {
  const MIDIAddress address {noteNumber, Channel_1};
  DBG_TS_VAL("IRHandler: sending MIDI NoteOn  note", noteNumber);
  Control_Surface.sendNoteOn(address, TRANSPORT_VELOCITY);
  DBG_TS_VAL("IRHandler: sending MIDI NoteOff note", noteNumber);
  Control_Surface.sendNoteOff(address, TRANSPORT_VELOCITY);
}

void IRHandler::update() {
  if (!IrReceiver.decode()) {
    return;
  }

  const auto command = static_cast<uint8_t>(IrReceiver.decodedIRData.command);
  const auto address = static_cast<uint16_t>(IrReceiver.decodedIRData.address);
  const bool isRepeat =
      (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) != 0;
  const auto now = millis();

  DBG_TS_IR("IRHandler: decoded", address, command);
  if (isRepeat) DBG_PRINTLN("  → repeat flag set");

  if (Config::IR_IGNORE_REPEAT && isRepeat) {
    DBG_TS_MSG("IRHandler: ignored (repeat)");
    IrReceiver.resume();
    return;
  }

  if (command == lastCommand &&
      (now - lastDispatchMs) < Config::IR_MIN_COMMAND_INTERVAL_MS) {
    DBG_TS_MSG("IRHandler: ignored (debounce interval)");
    IrReceiver.resume();
    return;
  }

  if (address != Config::IR_REMOTE_ADDRESS) {
    DBG_TS_IR("IRHandler: WRONG address (expected 0x0):", address, command);
    IrReceiver.resume();
    return;
  }

  // Address matches – dispatch transport command.
  if (command == Config::IR_CMD_PLAY) {
    DBG_TS_MSG("IRHandler: PLAY");
    sendMCUTransport(MCU::PLAY);
    lastCommand = command;
    lastDispatchMs = now;
  } else if (command == Config::IR_CMD_STOP) {
    DBG_TS_MSG("IRHandler: STOP");
    sendMCUTransport(MCU::STOP);
    lastCommand = command;
    lastDispatchMs = now;
  } else if (command == Config::IR_CMD_RECORD) {
    DBG_TS_MSG("IRHandler: RECORD");
    sendMCUTransport(MCU::RECORD);
    lastCommand = command;
    lastDispatchMs = now;
  } else if (command == Config::IR_CMD_REWIND) {
    DBG_TS_MSG("IRHandler: REWIND");
    sendMCUTransport(MCU::REWIND);
    lastCommand = command;
    lastDispatchMs = now;
  } else if (command == Config::IR_CMD_FAST_FORWARD) {
    DBG_TS_MSG("IRHandler: FAST_FORWARD");
    sendMCUTransport(MCU::FAST_FWD);
    lastCommand = command;
    lastDispatchMs = now;
  } else {
    DBG_TS_HEX("IRHandler: unknown command", command);
  }

  IrReceiver.resume();
}
