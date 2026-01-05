#include <stdio.h>
#include "board.h"
#include "device.hpp"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance != USART2) return;

  if (Uart *u = Uart::TryInstance()) {
    u->CopyRx();
    u->StartRx();
  }
}

extern "C" void BoardUsb_OnRx(const uint8_t* data, uint16_t len) {
  if (BoardUsb *u = BoardUsb::TryInstance()) {
    u->PushRx(data, len);
  }
}
