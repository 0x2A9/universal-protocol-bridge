#include <stdio.h>
#include "board.h"
#include "device.hpp"

extern "C" {

volatile HAL_I2C_StateTypeDef aht_i2c_state = HAL_I2C_STATE_RESET;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance != USART2) return;

  if (Uart *u = Uart::TryInstance()) {
    u->CopyRx();
    u->StartRx();
  }
}

void BoardUsb_OnRx(const uint8_t *data, uint16_t len) {
  if (Usb *u = Usb::TryInstance()) {
    u->EnqueueRx(data, len);
  }
}

void HAL_I2C_MasterTxCpltCallback(
    I2C_HandleTypeDef *hi2c) {

  if (hi2c->Instance == I2C2) {
     aht_tx_complete = true;
  }
}

void HAL_I2C_MasterRxCpltCallback(
    I2C_HandleTypeDef *hi2c) {

  if (hi2c->Instance == I2C2) {
    aht_rx_complete = true;
  }
}

void HAL_I2C_ErrorCallback(
    I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance == I2C2) {
    aht_i2c_error_code = hi2c->ErrorCode;
    aht_i2c_error = true;

    aht_operation;

    // Put breakpoint here.
    // Inspect aht_operation.
  }
}

} // extern "C"
