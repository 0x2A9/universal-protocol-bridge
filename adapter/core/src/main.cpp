#include "device.hpp"
#include "controllers/led_controller.hpp"
#include "controllers/peripherals_controller.hpp"
#include "dcp/dcp_sender.hpp"
#include "dcp/dcp_handler.hpp"

LedController leds;
Usb usb;
Uart uart;
PeripheralsController peripherals(uart);
DcpSender dcp_sender(usb);
DcpHandler dcp_handler(usb, peripherals, dcp_sender);
Device dev(leds, dcp_handler, peripherals);

int main(void) {
  dev.Init();

  while (true) {
    dev.Run();
    dev.DelayMs(100);
  }

  return 0;
}
