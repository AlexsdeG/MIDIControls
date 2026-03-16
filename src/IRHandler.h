#pragma once

#include <stdint.h>

class IRHandler {
 public:
  void begin();
  void update();

 private:
  uint8_t commandToCC(uint16_t command) const;
  void sendDAWCC(uint8_t cc) const;

  uint16_t lastSentCommand = 0xFFFF;
  unsigned long lastSentAt = 0;
};
