#include "device.hpp"

LedController leds;
Usb usb;
I2c i2c;
Uart uart;
Device dev(leds, usb, i2c, uart);

int main(void) { 
  dev.Init();

  while (true) {
    dev.Run();
    dev.DelayMs(100);
  }

  return 0;
}
