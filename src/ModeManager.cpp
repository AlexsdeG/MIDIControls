#include "ModeManager.h"

#include "Debug.h"

ModeManager::ModeManager(pin_t ledPin) : ledPin(ledPin) {}

void ModeManager::begin() {
  pinMode(ledPin, OUTPUT);
  phase = Phase::Idle;
  phaseStartedMs = millis();
  holdArmed = false;
  indicateTargetBlinks = 0;
  indicateCompletedBlinks = 0;
  setLed(getIdleLedForMode(currentMode));
}

bool ModeManager::getIdleLedForMode(uint8_t mode) const {
  if (mode == Config::MODE_CONTROL)
    return true;
  return false;
}

void ModeManager::setLed(bool on) {
  if (on != ledOn) {
    DBG_TS_VAL("LED state", on ? "ON" : "OFF");
  }
  ledOn = on;
  digitalWrite(ledPin, on ? HIGH : LOW);
}

void ModeManager::startBankBlinkIndication(setting_t bankSelection) {
  if (bankSelection < 0)
    bankSelection = 0;

  const setting_t maxBank = static_cast<setting_t>(Config::MAIN_BANK_COUNT - 1);
  if (bankSelection > maxBank)
    bankSelection = maxBank;

  indicateTargetBlinks = static_cast<uint8_t>(bankSelection);

  if (indicateTargetBlinks == 0) {
    indicateCompletedBlinks = 0;
    phase = Phase::Idle;
    setLed(getIdleLedForMode(currentMode));
    DBG_TS_VAL("ModeManager indicate target", indicateTargetBlinks);
    DBG_TS_MSG("ModeManager indicate skipped (0 blinks)");
    return;
  }

  indicateCompletedBlinks = 0;
  // Always start from OFF so the first ON edge is visible,
  // especially after long-hold where the LED may already be ON.
  setLed(false);
  phase = Phase::IndicateOff;
  phaseStartedMs = millis();

  DBG_TS_VAL("ModeManager indicate target", indicateTargetBlinks);
}

void ModeManager::onModeBankChanged(uint8_t mode,
                                    setting_t bankSelection,
                                    bool flashSwitch) {
  currentMode = mode;
  currentBank = bankSelection;
  holdArmed = false;

  DBG_SEP();
  DBG_TS_VAL("ModeManager mode", mode);
  DBG_TS_VAL("ModeManager bank", bankSelection);

  if (flashSwitch) {
    startBankBlinkIndication(currentBank);
    DBG_TS_MSG("ModeManager switch bank indication");
    return;
  }

  phase = Phase::Idle;
  setLed(getIdleLedForMode(currentMode));
}

void ModeManager::onBlueHoldArmed() {
  holdArmed = true;
  phase = Phase::Idle;

  // Hold-threshold feedback: light LED in Piano/Drum, force OFF in Control.
  if (currentMode == Config::MODE_CONTROL) {
    setLed(false);
    DBG_TS_MSG("ModeManager hold armed (control mode -> LED OFF)");
  } else {
    setLed(true);
    DBG_TS_MSG("ModeManager hold armed (piano/drum -> LED ON)");
  }
}

void ModeManager::onBlueHoldReleased(setting_t bankSelection) {
  holdArmed = false;
  startBankBlinkIndication(bankSelection);
}

void ModeManager::update() {
  const auto now = millis();

  switch (phase) {
  case Phase::Idle:
    if (!holdArmed) {
      setLed(getIdleLedForMode(currentMode));
    }
    return;

  case Phase::SwitchFlashOff:
    if (now - phaseStartedMs >= Config::BLUE_SWITCH_FLASH_MS) {
      phase = Phase::Idle;
      phaseStartedMs = now;
      setLed(getIdleLedForMode(currentMode));
      DBG_TS_MSG("ModeManager switch flash done");
    }
    return;

  case Phase::IndicateOn:
    if (now - phaseStartedMs >= Config::BLUE_INDICATE_ON_MS) {
      if (indicateCompletedBlinks >= indicateTargetBlinks) {
        phase = Phase::Idle;
        setLed(getIdleLedForMode(currentMode));
        DBG_TS_MSG("ModeManager indicate done");
      } else {
        setLed(false);
        phase = Phase::IndicateOff;
        phaseStartedMs = now;
      }
    }
    return;

  case Phase::IndicateOff:
    if (now - phaseStartedMs >= Config::BLUE_INDICATE_OFF_MS) {
      if (indicateCompletedBlinks >= indicateTargetBlinks) {
        phase = Phase::Idle;
        setLed(getIdleLedForMode(currentMode));
        DBG_TS_MSG("ModeManager indicate done");
      } else {
        setLed(true);
        indicateCompletedBlinks++;
        DBG_TS_VAL("ModeManager indicate count", indicateCompletedBlinks);
        phase = Phase::IndicateOn;
        phaseStartedMs = now;
      }
    }
    return;
  }
}
