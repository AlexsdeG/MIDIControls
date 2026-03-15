#include <Control_Surface.h>

#include "Config.h"
#include "Debug.h"
#include "IRHandler.h"
#include "ModeManager.h"
#include "TransposedManyAddressNoteButtonMatrix.h"

#ifndef DEBUG_SERIAL
HardwareSerialMIDI_Interface midi {Serial, 31250};
#endif

Bank<Config::MAIN_BANK_COUNT> mainBank;
Bank<Config::SHIFT_BANK_COUNT> shiftBank;

Transposer<Config::TRANSPOSE_MIN, Config::TRANSPOSE_MAX> transposer {12};

Button blueBankButton {Config::PIN_BLUE_BUTTON};
Button yellowShiftButton {Config::PIN_YELLOW_BUTTON};
Button redActionButton {Config::PIN_RED_BUTTON};

ModeManager modeManager {Config::PIN_BLUE_LED};
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

PBPotentiometer joystickX {Config::PIN_JOYSTICK_X, Channel_1};

Bankable::ManyAddresses::CCPotentiometer<Config::SHIFT_BANK_COUNT> joystickY {
  shiftBank,
  Config::PIN_JOYSTICK_Y,
  Config::JOYSTICK_Y_ADDRESSES,
};

namespace {

int8_t currentTransposeOffset = 0;
bool joystickTransposeLocked = false;
bool shiftHeld = false;

bool bluePressed = false;
bool blueHoldArmed = false;
unsigned long bluePressStartedMs = 0;

setting_t currentControlBank = Config::BANK_CONTROL_1;

setting_t toTransposerSelection(int8_t offset) {
  return static_cast<setting_t>(offset - Config::TRANSPOSE_MIN);
}

uint8_t modeFromBank(setting_t bank) {
  if (bank == Config::BANK_PIANO)
    return Config::MODE_PIANO;
  if (bank == Config::BANK_DRUM)
    return Config::MODE_DRUM;
  return Config::MODE_CONTROL;
}

#ifdef DEBUG_SERIAL
const char *modeName(uint8_t mode) {
  if (mode == Config::MODE_PIANO)
    return "PIANO";
  if (mode == Config::MODE_DRUM)
    return "DRUM";
  return "CONTROL";
}
#endif

void selectMainBank(setting_t nextBank, bool flashSwitch, const char *reason) {
  const setting_t prev = mainBank.getSelection();
  if (prev == nextBank)
    return;

  mainBank.select(nextBank);
  if (nextBank >= Config::BANK_CONTROL_1 && nextBank <= Config::BANK_CONTROL_4) {
    currentControlBank = nextBank;
  }

  const uint8_t nextMode = modeFromBank(nextBank);

  DBG_SEP();
  DBG_TS_VAL("BANK from", prev);
  DBG_TS_VAL("BANK to  ", nextBank);
#ifdef DEBUG_SERIAL
  DBG_TS_VAL("MODE", modeName(nextMode));
#endif
  DBG_TS_VAL("reason", reason);
  DBG_SEP();

  modeManager.onModeBankChanged(nextMode, nextBank, flashSwitch);
}

void applyTransposeOffset(int8_t offset) {
  if (offset < Config::TRANSPOSE_MIN)
    offset = Config::TRANSPOSE_MIN;
  if (offset > Config::TRANSPOSE_MAX)
    offset = Config::TRANSPOSE_MAX;

  if (offset != currentTransposeOffset) {
    DBG_TS_VAL("TRANSPOSE old", currentTransposeOffset);
    DBG_TS_VAL("TRANSPOSE new", offset);
  }

  currentTransposeOffset = offset;
  transposer.select(toTransposerSelection(offset));
}

void sendMomentaryNote(MIDIAddress address) {
  constexpr uint8_t VELOCITY = 0x7F;
  DBG_TS_VAL("MIDI Note address", address.getAddress());
  DBG_TS_VAL("MIDI Note channel", address.getChannel().getOneBased());
#ifndef DEBUG_SERIAL
  Control_Surface.sendNoteOn(address, VELOCITY);
  Control_Surface.sendNoteOff(address, VELOCITY);
#else
  DBG_TS_MSG("DEBUG mode: MIDI send skipped");
#endif
}

void updateShiftState() {
  const auto event = yellowShiftButton.update();
  if (event == Button::Falling) {
    shiftHeld = true;
    shiftBank.select(Config::SHIFT_BANK_ACTIVE);
    DBG_TS_MSG("SHIFT pressed (active)");
  } else if (event == Button::Rising) {
    shiftHeld = false;
    shiftBank.select(Config::SHIFT_BANK_BASE);
    DBG_TS_MSG("SHIFT released (base)");
  }
}

void updateRedActionButton() {
  if (redActionButton.update() != Button::Falling)
    return;

  if (shiftHeld) {
    DBG_TS_MSG("RED action: SHIFT+RED -> input monitor toggle message");
    sendMomentaryNote(Config::SHIFT_RED_MONITOR_ADDRESS);
  } else {
    DBG_TS_MSG("RED action: RED -> record message");
    sendMomentaryNote(Config::RED_BUTTON_ADDRESSES[Config::SHIFT_BANK_BASE]);
  }
}

void updateJoystickTranspose() {
  if (!shiftHeld) {
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
    joystickTransposeLocked = false;
    DBG_TS_MSG("JOYSTICK_Y transpose released");
  }
}

setting_t controlBankFromSelector(uint8_t col) {
  if (col == Config::SELECTOR_COL_S1)
    return Config::BANK_CONTROL_1;
  if (col == Config::SELECTOR_COL_S2)
    return Config::BANK_CONTROL_2;
  if (col == Config::SELECTOR_COL_S3)
    return Config::BANK_CONTROL_3;
  return Config::BANK_CONTROL_4;
}

bool onMatrixButtonEvent(uint8_t row, uint8_t col, bool pressed) {
  const bool isSelector =
      row == Config::SELECTOR_ROW && col <= Config::SELECTOR_COL_S4;

  if (!isSelector)
    return false;

  const bool inControlMode = modeFromBank(mainBank.getSelection()) == Config::MODE_CONTROL;
  if (!inControlMode)
    return false;

  if (!pressed)
    return true;

  const setting_t targetControlBank = controlBankFromSelector(col);
  DBG_TS_VAL("MATRIX selector col", col);
  DBG_TS_VAL("MATRIX selector bank", targetControlBank);
  selectMainBank(targetControlBank, true, "matrix selector S1-S4");
  return true;
}

void updateBlueButtonModes() {
  const auto event = blueBankButton.update();
  const auto now = millis();

  if (event == Button::Falling) {
    bluePressed = true;
    blueHoldArmed = false;
    bluePressStartedMs = now;
    DBG_TS_MSG("BLUE pressed");
  }

  if (bluePressed && !blueHoldArmed &&
      now - bluePressStartedMs >= Config::BLUE_HOLD_THRESHOLD_MS) {
    blueHoldArmed = true;
    DBG_TS_MSG("BLUE hold threshold reached (armed)");
    modeManager.onBlueHoldArmed();
  }

  if (event != Button::Rising)
    return;

  bluePressed = false;
  DBG_TS_MSG("BLUE released");

  if (blueHoldArmed) {
    blueHoldArmed = false;
    DBG_TS_MSG("BLUE long release -> show selected control bank");
    modeManager.onBlueHoldReleased(currentControlBank);
    return;
  }

  const setting_t current = mainBank.getSelection();
  if (current == Config::BANK_PIANO) {
    selectMainBank(Config::BANK_DRUM, true, "blue short press");
  } else if (current == Config::BANK_DRUM) {
    selectMainBank(currentControlBank, true, "blue short press");
  } else {
    selectMainBank(Config::BANK_PIANO, true, "blue short press");
  }
}

void updateAnalogDebugTelemetry() {
#ifdef DEBUG_SERIAL
  static int lastRot1 = -1;
  static int lastRot2 = -1;
  static int lastJoyX = -1;
  static int lastJoyY = -1;

  const int rot1 = analogRead(Config::PIN_ROTARY_1);
  const int rot2 = analogRead(Config::PIN_ROTARY_2);
  const int joyX = analogRead(Config::PIN_JOYSTICK_X);
  const int joyY = analogRead(Config::PIN_JOYSTICK_Y);

  if (lastRot1 < 0 || abs(rot1 - lastRot1) >= 16) {
    DBG_TS_VAL("ROTARY_1 raw", rot1);
    lastRot1 = rot1;
  }
  if (lastRot2 < 0 || abs(rot2 - lastRot2) >= 16) {
    DBG_TS_VAL("ROTARY_2 raw", rot2);
    lastRot2 = rot2;
  }
  if (lastJoyX < 0 || abs(joyX - lastJoyX) >= 20) {
    DBG_TS_VAL("JOYSTICK_X raw", joyX);
    lastJoyX = joyX;
  }
  if (lastJoyY < 0 || abs(joyY - lastJoyY) >= 20) {
    DBG_TS_VAL("JOYSTICK_Y raw", joyY);
    lastJoyY = joyY;
  }
#endif
}

} // namespace

void setup() {
  debugBegin();
  DBG_SEP();
  DBG_TS_MSG("setup() starting");

  DBG_TS_MSG("setup() step: buttons begin");
  blueBankButton.begin();
  yellowShiftButton.begin();
  redActionButton.begin();

  DBG_TS_MSG("setup() step: initial bank + transpose state");
  shiftBank.select(Config::SHIFT_BANK_BASE);
  applyTransposeOffset(0);
  mainBank.select(Config::BANK_PIANO);
  currentControlBank = Config::BANK_CONTROL_1;

  DBG_TS_MSG("setup() step: matrix callback");
  noteMatrix.setButtonEventCallback(onMatrixButtonEvent);

#ifdef DEBUG_SERIAL
  DBG_TS_MSG("setup() step: begin elements directly (debug path)");
  noteMatrix.begin();
  rotary1.begin();
  rotary2.begin();
  joystickX.begin();
  joystickY.begin();
#else
  DBG_TS_MSG("setup() step: Control_Surface.begin()");
  Control_Surface.begin();
#endif

  DBG_TS_MSG("setup() step: ModeManager.begin()");
  modeManager.begin();

  DBG_TS_MSG("setup() step: ModeManager.onModeBankChanged()");
  modeManager.onModeBankChanged(Config::MODE_PIANO, Config::BANK_PIANO, false);

  DBG_TS_MSG("setup() step: IRHandler.begin()");
  irHandler.begin();

  DBG_TS_MSG("Mode cycle: Piano -> Drum -> Control -> Piano");
  DBG_TS_MSG("Control selectors: S1,S2,S3,S4 -> banks 2,3,4,5");
  DBG_TS_MSG("LED idle: Piano OFF, Drum OFF, Control ON");
  DBG_TS_VAL("SHIFT+RED monitor note", Config::SHIFT_RED_MONITOR_ADDRESS.getAddress());
  DBG_TS_VAL("SHIFT+RED monitor ch", Config::SHIFT_RED_MONITOR_ADDRESS.getChannel().getOneBased());
  DBG_TS_MSG("setup() complete");
  DBG_SEP();
}

void loop() {
#ifdef DEBUG_SERIAL
  static unsigned long lastHeartbeatMs = 0;
  const auto now = millis();
  if (now - lastHeartbeatMs >= 1000) {
    DBG_TS_MSG("loop heartbeat");
    DBG_TS_VAL("heartbeat mode", modeFromBank(mainBank.getSelection()));
    DBG_TS_VAL("heartbeat bank", mainBank.getSelection());
    DBG_TS_VAL("heartbeat shiftHeld", shiftHeld ? 1 : 0);
    lastHeartbeatMs = now;
  }
#endif

#ifdef DEBUG_SERIAL
  noteMatrix.update();
  rotary1.update();
  rotary2.update();
  joystickX.update();
  joystickY.update();
#else
  Control_Surface.loop();
#endif
  updateShiftState();
  updateRedActionButton();
  updateBlueButtonModes();
  updateJoystickTranspose();
  updateAnalogDebugTelemetry();
  modeManager.update();
  irHandler.update();
}
