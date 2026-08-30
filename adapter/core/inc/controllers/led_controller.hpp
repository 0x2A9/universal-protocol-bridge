#ifndef ADAPTER_CORE_INC_CONTROLLERS_LED_CONTROLLER_HPP
#define ADAPTER_CORE_INC_CONTROLLERS_LED_CONTROLLER_HPP

#ifdef __cplusplus
extern "C" {
#endif

class LedController {
 public:
  void ToggleInfo(void);
  void SetWarn(void);
  void ResetWarn(void);
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_CONTROLLERS_LED_CONTROLLER_HPP
