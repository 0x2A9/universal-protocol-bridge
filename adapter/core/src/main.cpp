#include "device.hpp"

int main(void) { 
  BoardLedController leds;
  BoardUsb usb;
  Device dev(leds, usb);

  dev.Init();

  while (true) {
    dev.Run();
    dev.DelayMs(500);
  }

  return 0;
}
