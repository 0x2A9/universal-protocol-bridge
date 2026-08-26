#include <stdio.h>
#include "board.h"
#include "device.hpp"
#include "usb_device.h"
#include "usbd_cdc_if.h"

static char buf[64];

volatile bool aht_tx_complete = false;
volatile bool aht_rx_complete = false;
volatile bool aht_i2c_error = false;



volatile AhtOperation aht_operation = AhtOperation::kNone;
volatile uint32_t aht_i2c_error_code = 0;
namespace {

constexpr uint16_t kAht10Address = 0x38U << 1;
constexpr uint32_t kAht10PowerUpDelayMs = 40;
constexpr uint32_t kAht10InitDelayMs = 10;
constexpr uint32_t kAht10MeasurementDelayMs = 80;

enum class Aht10State {
  kPowerUp,
  kMeasureTx,
  kMeasurementWait,
  kReceive,
  kDataReady,
  kError
};

Aht10State aht_state = Aht10State::kPowerUp;

uint8_t aht_init_cmd[3] = {
    0xE1,
    0x08,
    0x00
};

uint8_t aht_measure_cmd[3] = {
    0xAC,
    0x33,
    0x00
};

uint8_t aht_rx[6]{};

uint32_t aht_timestamp = 0;

/* Keep this visible for debugger */
volatile float aht_humidity = 0.0F;

}  // namespace


void TestAht10It()
{
  if (aht_i2c_error) {
    aht_state = Aht10State::kError;
  }

  switch (aht_state) {

    case Aht10State::kPowerUp:
      if (HAL_GetTick() >= kAht10PowerUpDelayMs) {

        /*
         * Prepare software state BEFORE starting
         * the asynchronous operation.
         */
        aht_tx_complete = false;
        aht_operation = AhtOperation::kMeasure;
        aht_state = Aht10State::kMeasureTx;

        HAL_StatusTypeDef st =
            HAL_I2C_Master_Transmit_IT(
                &hi2c2,
                kAht10Address,
                aht_measure_cmd,
                sizeof(aht_measure_cmd));

        if (st != HAL_OK) {
          aht_state = Aht10State::kError;
        }
      }
      break;

    case Aht10State::kMeasureTx:
      if (aht_tx_complete) {
        aht_tx_complete = false;

        /*
         * AC 33 00 has now actually been transmitted.
         * Start measurement timeout from here.
         */
        aht_timestamp = HAL_GetTick();

        aht_state = Aht10State::kMeasurementWait;
      }
      break;

    case Aht10State::kMeasurementWait:
      if ((HAL_GetTick() - aht_timestamp) >=
          kAht10MeasurementDelayMs) {

        aht_rx_complete = false;
        aht_operation = AhtOperation::kReceive;
        aht_state = Aht10State::kReceive;

        HAL_StatusTypeDef st =
            HAL_I2C_Master_Receive_IT(
                &hi2c2,
                kAht10Address,
                aht_rx,
                sizeof(aht_rx));

        if (st != HAL_OK) {
          aht_state = Aht10State::kError;
        }
      }
      break;

    case Aht10State::kReceive:
      if (aht_rx_complete) {
        aht_rx_complete = false;

        /*
         * Bit 7: BUSY
         */
        if ((aht_rx[0] & 0x80U) != 0U) {
          aht_state = Aht10State::kError;
          break;
        }

        uint32_t raw_humidity =
            ((uint32_t)aht_rx[1] << 12U) |
            ((uint32_t)aht_rx[2] << 4U) |
            ((uint32_t)aht_rx[3] >> 4U);

        aht_humidity =
            ((float)raw_humidity * 100.0F) /
            1048576.0F;

        aht_state = Aht10State::kDataReady;
      }
      break;

    case Aht10State::kDataReady:
      /*
       * Breakpoint:
       *
       * aht_rx
       * aht_humidity
       */
      break;

    case Aht10State::kError:
      break;
  }
}

void LedController::ToggleInfo(void) {
  HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);
}

void LedController::SetWarn(void) {
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
}

void LedController::ResetWarn(void) {
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}

Usb::Usb(void) {
  /* Avoid multiple instances */
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
}

/* May return nullptr */
Usb *Usb::TryInstance(void) {
  return instance_;
}

bool Usb::Init(void) {
  MX_USB_DEVICE_Init();
  return true;
}

bool Usb::IsReady(void) const {
  return USB_GetDeviceHandle()->dev_state == USBD_STATE_CONFIGURED;
}

bool Usb::EnqueueTx(const uint8_t *src, const uint16_t len) {
  if (tx_buf_.Free() < len) return false;
  tx_buf_.Push(src, len);
  return true;
}

void Usb::ProcessTx(void) {
  if (!IsReady()) return;

  uint16_t avail = tx_buf_.Count();
  if (avail == 0) return;

  /* CDC FS packet is 64 bytes max */
  uint8_t tmp[64];
  uint16_t n = (avail > sizeof(tmp)) ? 
               (uint16_t)sizeof(tmp) : 
               avail;

  n = tx_buf_.Peek(tmp, n);
  if (n == 0) return;

  uint8_t st = CDC_Transmit_FS(tmp, n);
  if (st == USBD_OK) {
    tx_buf_.Drop(n); // Commit only on success
  }
  /* If BUSY: do nothing, try again next loop */
}

bool Usb::EnqueueRx(const uint8_t *src, const uint16_t len) {
  if (rx_buf_.Free() < len) return false;
  rx_buf_.Push(src, len);
  return true;
}

uint16_t Usb::DequeueRx(uint8_t *dst, const uint16_t len) {
  return rx_buf_.Pop(dst, len);
}

I2c::I2c(void) {
  /* Avoid multiple instances */
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
}

/* May return nullptr */
I2c *I2c::TryInstance(void) {
  return instance_;
}

Uart::Uart(void) {
  /* Avoid multiple instances */
  if (instance_ != nullptr) Error_Handler();

  instance_ = this;
}

bool I2c::Init(void) {
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {};

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
  PeriphClkInit.I2c2ClockSelection   = RCC_I2C2CLKSOURCE_HSI;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  MX_I2C2_Init();

  return true;
}

/* May return nullptr */
Uart *Uart::TryInstance(void) {
  return instance_;
}

bool Uart::Init(void) {
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {};

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  MX_USART2_UART_Init();
  StartRx();

  return true;
}

/* Rearms UART data reception */
void Uart::StartRx(void) {
  HAL_UART_Receive_IT(&huart2, rx_tmp_, sizeof(rx_tmp_));
}

uint8_t Uart::Transmit(uint8_t *src, const uint16_t len) {
  return HAL_UART_Transmit(&huart2, src, len, 10);
}

bool Uart::CopyRx(void) {
  if (!EnqueueRx(rx_tmp_, sizeof(rx_tmp_))) return false;
  is_new_rx_data_ = true;
  return true;
}

bool Uart::EnqueueRx(const uint8_t *src, const uint16_t len) {
  if (rx_buf_.Free() < len) return false;
  rx_buf_.Push(src, len);
  return true;
}

uint16_t Uart::DequeueRx(uint8_t *dst, const uint16_t len) {
  return rx_buf_.Pop(dst, len);
}

bool Uart::IsNewRxData(void) {
  return is_new_rx_data_;
}
void Uart::ClearNewRxDataFlag(void) {
  is_new_rx_data_ = false;
}

void Device::Init(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  usb_.Init();
  i2c_.Init();
  uart_.Init();
}

void Device::Run(void) {

  constexpr uint16_t kAht10Address = 0x38U << 1U;

  volatile HAL_StatusTypeDef status =
    HAL_I2C_IsDeviceReady(
        &hi2c2,
        kAht10Address,
        3,
        100);

        uint8_t sensor_status = 0;

// HAL_StatusTypeDef st =
//     HAL_I2C_Master_Receive(
//         &hi2c2,
//         kAht10Address,
//         &sensor_status,
//         1,
//         100);

//     if ((sensor_status & 0x08U) != 0U) {
//       aht_operation = AhtOperation::kInit;
//     }
  /* Simple USB-UART test */
  // if (!usb_.IsReady()) return;

  leds_.ResetWarn();
  // leds_.ToggleInfo();

  // int n = 0;

  // n = usb_.DequeueRx((uint8_t*)buf, sizeof(buf));

  // if (n > 0) {
  //   usb_.EnqueueTx((uint8_t*)buf, (uint16_t)n);
  // }

  // if (uart_.IsNewRxData()) {
  //   leds_.SetWarn();
  //   n = uart_.DequeueRx((uint8_t*)buf, sizeof(buf));
  //   usb_.EnqueueTx((uint8_t*)buf, (uint16_t)n);
  //   uart_.ClearNewRxDataFlag();
  // }

  // usb_.ProcessTx();

  // uint32_t t = HAL_GetTick();

  // n = snprintf(buf, sizeof(buf), "%lu\n", (unsigned long)t);

  // if (n > 0) {
  //   usb_.EnqueueTx((uint8_t*)buf, (uint16_t)n);
  // }

  // usb_.ProcessTx();

  TestAht10It();
}

void Device::DelayMs(const uint32_t ms) {
  HAL_Delay(ms);
}
