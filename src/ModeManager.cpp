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
    phase = Phase::SwitchFlashOff;
    phaseStartedMs = millis();
    setLed(false);
    DBG_TS_MSG("ModeManager switch flash OFF");
    return;
  }

  phase = Phase::Idle;
  setLed(getIdleLedForMode(currentMode));
}

void ModeManager::onBlueHoldArmed() {
  holdArmed = true;
  phase = Phase::Idle;
  setLed(true);
  DBG_TS_MSG("ModeManager hold armed");
}

void ModeManager::onBlueHoldReleased(setting_t controlBank) {
  holdArmed = false;

  if (controlBank < Config::BANK_CONTROL_1 ||
      controlBank > Config::BANK_CONTROL_4) {
    controlBank = Config::BANK_CONTROL_1;
  }

  indicateTargetBlinks = static_cast<uint8_t>(controlBank);
  indicateCompletedBlinks = 0;
  phase = Phase::IndicateOn;
  phaseStartedMs = millis();
  setLed(true);

  DBG_TS_VAL("ModeManager indicate target", indicateTargetBlinks);
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
      setLed(false);
      phase = Phase::IndicateOff;
      phaseStartedMs = now;
      indicateCompletedBlinks++;
      DBG_TS_VAL("ModeManager indicate count", indicateCompletedBlinks);
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
        phase = Phase::IndicateOn;
        phaseStartedMs = now;
      }
    }
    return;
  }
}
