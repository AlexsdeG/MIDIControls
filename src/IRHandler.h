#pragma once

#include <stdint.h>

class IRHandler {
 public:
  void begin();
  void update();

 private:
  void sendMCUTransport(uint8_t noteNumber) const;

  uint8_t lastCommand = 0;
  unsigned long lastDispatchMs = 0;
};
