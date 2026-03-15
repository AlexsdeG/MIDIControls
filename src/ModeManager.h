#pragma once

#include <Control_Surface.h>

class ModeManager {
 public:
  ModeManager(const OutputBank &bank, pin_t ledPin);

  void begin();
  void update();

 private:
  void setLed(bool on);
  void resetBlinkPattern(setting_t bankSelection, unsigned long now);

  const OutputBank &bank;
  const pin_t ledPin;

  bool hasLastBankSelection = false;
  setting_t lastBankSelection = 0;
  uint8_t completedBlinks = 0;
  bool ledOn = false;
  bool inPause = false;
  unsigned long lastTransitionMs = 0;
};
