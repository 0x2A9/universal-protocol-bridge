#include "controllers/led_controller.hpp"

#include "board.h"

void LedController::ToggleInfo(void) {
  HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);
}

void LedController::SetWarn(void) {
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
}

void LedController::ResetWarn(void) {
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}
