#ifndef ADAPTER_CORE_INC_DCP_DCP_SENDER_HPP
#define ADAPTER_CORE_INC_DCP_DCP_SENDER_HPP

#include <stdint.h>

#include "dcp/core/constants.hpp"
#include "dcp/core/frame.hpp"
#include "dcp/response.hpp"
#include "dcp/uart/messages.hpp"

#ifdef __cplusplus
extern "C" {
#endif

class Usb;

/* Outbound DCP path: owns the TX half of Usb, turns a dcp::Response or a
 * typed event payload into bytes on the wire. No dependency on DcpHandler --
 * it doesn't know a request ever existed. */
class DcpSender {
 public:
  explicit DcpSender(Usb &usb) : usb_(usb) {}

  bool SendResponse(const dcp::Response &response);
  bool SendUartRxEvent(uint8_t resource_id, const dcp::uart::RxEvent &event);
  bool SendHeartbeatEvent(void);
  bool SendErrorEvent(dcp::Status status);

 private:
  bool SendFrame(const dcp::FrameHeader &header, const uint8_t *payload, uint16_t payload_length);
  bool EncodeResponsePayload(const dcp::Response &response, uint8_t *dst, uint16_t capacity,
                              uint16_t &out_len);

  Usb &usb_;
  uint8_t tx_buf_[dcp::kMaxFrameSize] {};
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_DCP_DCP_SENDER_HPP
