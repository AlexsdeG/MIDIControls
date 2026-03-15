#include <Control_Surface.h>

#include "Config.h"
#include "Debug.h"
#include "IRHandler.h"
#include "ModeManager.h"
#include "TransposedManyAddressNoteButtonMatrix.h"

// In normal (MIDI) mode Hardware Serial carries MIDI at 31250 baud via MocoLUFA.
// In debug mode we omit the MIDI interface so Serial is free for debug output.
// Build [env:uno_debug] to enter debug mode; [env:uno] for normal MIDI operation.
#ifndef DEBUG_SERIAL
HardwareSerialMIDI_Interface midi {Serial, 31250};
#endif

Bank<Config::MAIN_BANK_COUNT> mainBank;
Bank<Config::SHIFT_BANK_COUNT> shiftBank;

Transposer<Config::TRANSPOSE_MIN, Config::TRANSPOSE_MAX> transposer {12};

Button blueBankButton {Config::PIN_BLUE_BUTTON};
ModeManager modeManager {mainBank, Config::PIN_BLUE_LED};
IRHandler irHandler;

MIDIControls::TransposedManyAddressNoteButtonMatrix<Config::MAIN_BANK_COUNT, 4,
                                                    4>
  noteMatrix {
    mainBank,
    Config::MATRIX_ROW_PINS,
    Config::MATRIX_COL_PINS,
    Config::MATRIX_BANK_ADDRESSES,
    Config::MATRIX_BANK_CHANNELS,
    transposer,
  };

IncrementSelector<Config::SHIFT_BANK_COUNT> shiftSelector {
  shiftBank,
  Config::PIN_YELLOW_BUTTON,
};

Bankable::ManyAddresses::CCPotentiometer<Config::SHIFT_BANK_COUNT> rotary1 {
  shiftBank,
  Config::PIN_ROTARY_1,
  Config::ROTARY_1_ADDRESSES,
};

Bankable::ManyAddresses::CCPotentiometer<Config::SHIFT_BANK_COUNT> rotary2 {
  shiftBank,
  Config::PIN_ROTARY_2,
  Config::ROTARY_2_ADDRESSES,
};

Bankable::ManyAddresses::NoteButton<Config::SHIFT_BANK_COUNT> redButton {
  shiftBank,
  Config::PIN_RED_BUTTON,
  Config::RED_BUTTON_ADDRESSES,
};

PBPotentiometer joystickX {Config::PIN_JOYSTICK_X, Channel_1};

Bankable::ManyAddresses::CCPotentiometer<Config::SHIFT_BANK_COUNT> joystickY {
  shiftBank,
  Config::PIN_JOYSTICK_Y,
  Config::JOYSTICK_Y_ADDRESSES,
};

namespace {

int8_t currentTransposeOffset = 0;
bool joystickTransposeLocked = false;

setting_t toTransposerSelection(int8_t offset) {
  return static_cast<setting_t>(offset - Config::TRANSPOSE_MIN);
}

void applyTransposeOffset(int8_t offset) {
  if (offset < Config::TRANSPOSE_MIN)
    offset = Config::TRANSPOSE_MIN;
  if (offset > Config::TRANSPOSE_MAX)
    offset = Config::TRANSPOSE_MAX;

  if (offset != currentTransposeOffset) {
    DBG_TS_VAL("TRANSPOSE old", currentTransposeOffset);
    DBG_TS_VAL("TRANSPOSE new", offset);
    DBG_TS_VAL("TRANSPOSE selector", toTransposerSelection(offset));
  }

  currentTransposeOffset = offset;
  transposer.select(toTransposerSelection(offset));
}

void updateJoystickTranspose() {
  if (shiftBank.getSelection() != Config::SHIFT_BANK_ACTIVE) {
    joystickTransposeLocked = false;
    return;
  }

  const auto joystickValue = analogRead(Config::PIN_JOYSTICK_Y);

  if (!joystickTransposeLocked) {
    if (joystickValue > Config::JOYSTICK_TRANSPOSE_HIGH_THRESHOLD) {
      DBG_TS_VAL("JOYSTICK_Y transpose UP   value", joystickValue);
      applyTransposeOffset(currentTransposeOffset + 1);
      joystickTransposeLocked = true;
    } else if (joystickValue < Config::JOYSTICK_TRANSPOSE_LOW_THRESHOLD) {
      DBG_TS_VAL("JOYSTICK_Y transpose DOWN value", joystickValue);
      applyTransposeOffset(currentTransposeOffset - 1);
      joystickTransposeLocked = true;
    }
    return;
  }

  if (joystickValue >= Config::JOYSTICK_TRANSPOSE_RELEASE_LOW &&
      joystickValue <= Config::JOYSTICK_TRANSPOSE_RELEASE_HIGH) {
    DBG_TS_VAL("JOYSTICK_Y released (center) value", joystickValue);
    joystickTransposeLocked = false;
  }
}

void updateMainBankSelection() {
  if (blueBankButton.update() == Button::Falling) {
    const setting_t prev = mainBank.getSelection();
    const setting_t nextSelection =
      static_cast<setting_t>((prev + 1) % Config::MAIN_BANK_COUNT);
    mainBank.select(nextSelection);
    DBG_SEP();
    DBG_TS_VAL("BANK switch from", prev);
    DBG_TS_VAL("BANK switch to  ", nextSelection);
    static const char* bankNames[] = {
      "Bank 0 – Piano",
      "Bank 1 – Drums",
      "Bank 2 – DAW",
      "Bank 3 – Track A",
      "Bank 4 – Track B",
      "Bank 5 – Track C",
    };
    if (nextSelection < Config::MAIN_BANK_COUNT)
      DBG_TS_VAL("BANK name", bankNames[nextSelection]);
    DBG_SEP();
  }
}

} // namespace

void setup() {
  debugBegin();    // must be first – initialises Serial in debug mode

  DBG_SEP();
  DBG_TS_MSG("setup() starting");
  DBG_TS_VAL("MAIN_BANK_COUNT ", Config::MAIN_BANK_COUNT);
  DBG_TS_VAL("SHIFT_BANK_COUNT", Config::SHIFT_BANK_COUNT);
  DBG_TS_VAL("TRANSPOSE_MIN   ", Config::TRANSPOSE_MIN);
  DBG_TS_VAL("TRANSPOSE_MAX   ", Config::TRANSPOSE_MAX);
  DBG_TS_MSG("Pins: Blue btn=12  Yellow btn=10  Red btn=11");
  DBG_TS_MSG("Pins: Blue LED=A4  Red LED=A5  IR=13");
  DBG_TS_MSG("Pins: Rotary1=A0  Rotary2=A1  JoyX=A3  JoyY=A2");
#ifdef DEBUG_SERIAL
  DBG_TS_MSG("NOTE: MIDI interface disabled in debug build");
#else
  DBG_TS_MSG("MIDI interface: HardwareSerial @ 31250 baud");
#endif
  DBG_SEP();

  applyTransposeOffset(0);
  DBG_TS_MSG("applyTransposeOffset(0) done");

  Control_Surface.begin();
  DBG_TS_MSG("Control_Surface.begin() done");

  modeManager.begin();
  DBG_TS_MSG("ModeManager.begin() done");

  irHandler.begin();
  DBG_TS_MSG("IRHandler.begin() done");

  DBG_TS_MSG("setup() complete – entering loop()");
  DBG_SEP();
}

void loop() {
  Control_Surface.loop();
  updateMainBankSelection();
  updateJoystickTranspose();
  modeManager.update();
  irHandler.update();
}