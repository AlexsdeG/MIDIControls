#pragma once

#include <Control_Surface.h>

#include "Config.h"

class ModeManager {
 public:
  explicit ModeManager(pin_t ledPin);

  void begin();
  void onModeBankChanged(uint8_t mode, setting_t bankSelection, bool flashSwitch);
  void onBlueHoldArmed();
  void onBlueHoldReleased(setting_t controlBank);
  void update();

 private:
  void setLed(bool on);
  bool getIdleLedForMode(uint8_t mode) const;

  enum class Phase : uint8_t {
    Idle,
    SwitchFlashOff,
    IndicateOn,
    IndicateOff,
  };

  const pin_t ledPin;

  uint8_t currentMode = Config::MODE_PIANO;
  setting_t currentBank = Config::BANK_PIANO;

  bool ledOn = false;
  bool holdArmed = false;

  Phase phase = Phase::Idle;
  unsigned long phaseStartedMs = 0;

  uint8_t indicateTargetBlinks = 0;
  uint8_t indicateCompletedBlinks = 0;
};
