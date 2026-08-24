#ifndef ADAPTER_DEVICES_ST_STM32F303VCT6_CORE_INC_BOARD_H
#define ADAPTER_DEVICES_ST_STM32F303VCT6_CORE_INC_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx_hal.h"

extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart2;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);
void MX_I2C2_Init(void);

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_DEVICES_ST_STM32F303VCT6_CORE_INC_BOARD_H
