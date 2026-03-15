#include "ModeManager.h"
#include "Debug.h"

namespace {
constexpr unsigned long BLINK_ON_MS = 120;
constexpr unsigned long BLINK_OFF_MS = 120;
constexpr unsigned long BLINK_PAUSE_MS = 700;
}

ModeManager::ModeManager(const OutputBank &bank, pin_t ledPin)
    : bank(bank), ledPin(ledPin) {}

void ModeManager::begin() {
  pinMode(ledPin, OUTPUT);
  setLed(false);
  hasLastBankSelection = false;
  lastBankSelection = 0;
  completedBlinks = 0;
  inPause = false;
  lastTransitionMs = millis();
}

void ModeManager::setLed(bool on) {
  if (on != ledOn) {
    DBG_TS_VAL("LED state", on ? "ON" : "OFF");
  }
  ledOn = on;
  digitalWrite(ledPin, on ? HIGH : LOW);
}

void ModeManager::resetBlinkPattern(setting_t bankSelection, unsigned long now) {
  completedBlinks = 0;
  inPause = false;
  lastTransitionMs = now;

  DBG_TS_VAL("ModeManager: resetBlinkPattern for bank", bankSelection);

  if (bankSelection >= 2) {
    DBG_TS_VAL("ModeManager: blink pattern – total blinks", bankSelection);
    setLed(true);
  } else if (bankSelection == 1) {
    DBG_TS_MSG("ModeManager: bank 1 – LED solid ON");
    setLed(true);
  } else {
    DBG_TS_MSG("ModeManager: bank 0 – LED OFF");
    setLed(false);
  }
}

void ModeManager::update() {
  const auto bankSelection = bank.getSelection();
  const auto now = millis();

  if (!hasLastBankSelection || bankSelection != lastBankSelection) {
    DBG_SEP();
    DBG_TS_VAL("ModeManager: bank changed from", lastBankSelection);
    DBG_TS_VAL("ModeManager: bank changed to  ", bankSelection);
    hasLastBankSelection = true;
    lastBankSelection = bankSelection;
    resetBlinkPattern(bankSelection, now);
  }

  if (bankSelection == 0) {
    if (ledOn)
      setLed(false);
    return;
  }

  if (bankSelection == 1) {
    if (!ledOn)
      setLed(true);
    return;
  }

  const uint8_t targetBlinks = static_cast<uint8_t>(bankSelection);

  if (inPause) {
    if (now - lastTransitionMs >= BLINK_PAUSE_MS) {
      DBG_TS_MSG("ModeManager: pause done – restarting blink cycle");
      inPause = false;
      completedBlinks = 0;
      setLed(true);
      lastTransitionMs = now;
    }
    return;
  }

  if (ledOn) {
    if (now - lastTransitionMs >= BLINK_ON_MS) {
      setLed(false);
      lastTransitionMs = now;
      completedBlinks++;

      DBG_TS_VAL("ModeManager: blink completed", completedBlinks);
      DBG_TS_VAL("ModeManager: target blinks  ", targetBlinks);

      if (completedBlinks >= targetBlinks) {
        DBG_TS_MSG("ModeManager: entering pause");
        inPause = true;
      }
    }
    return;
  }

  if (now - lastTransitionMs >= BLINK_OFF_MS) {
    setLed(true);
    lastTransitionMs = now;
  }
}
