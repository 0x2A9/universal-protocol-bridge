#include "device.hpp"

LedController leds;
Usb usb;
Uart uart;
Device dev(leds, usb, uart);

int main(void) { 
  dev.Init();

  while (true) {
    dev.Run();
    dev.DelayMs(500);
  }

  return 0;
}
