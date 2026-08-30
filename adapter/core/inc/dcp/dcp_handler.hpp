#ifndef ADAPTER_CORE_INC_DCP_DCP_HANDLER_HPP
#define ADAPTER_CORE_INC_DCP_DCP_HANDLER_HPP

#include <stdint.h>

#include "dcp/core/frame_parser.hpp"
#include "dcp/uart/messages.hpp"

#ifdef __cplusplus
extern "C" {
#endif

class Usb;
class PeripheralsController;
class DcpSender;

/* Inbound DCP path: owns the RX half of Usb plus the FrameParser, decodes
 * requests and dispatches them, replying through DcpSender. */
class DcpHandler {
 public:
  struct ProcessResult {
    bool uart_rx_processed = false;
  };

  DcpHandler(Usb &usb, PeripheralsController &controller, DcpSender &sender);

  bool Init(void);
  bool IsReady(void) const;
  ProcessResult Process(void);
  void Tick(uint32_t now_ms);

 private:
  void Handle(const dcp::Frame &frame);
  void HandleGetCapabilities(const dcp::FrameHeader &header);
  void HandleConfigure(const dcp::Frame &frame);
  void HandleGetConfig(const dcp::Frame &frame);
  void HandleUartConfigure(const dcp::FrameHeader &header, const dcp::uart::Config &config);
  void HandleUartGetConfig(const dcp::FrameHeader &header);
  void HandleUartWrite(const dcp::FrameHeader &header, const dcp::uart::WriteRequest &request);
  void HandleReset(const dcp::FrameHeader &header);

  void ReadUsbRx(void);
  void ProcessFrames(void);
  bool ProcessUartRx(void);

  /* Status-only response (no domain payload) -- used by every router/leaf
   * that just needs to ack/reject a request. A response carrying an actual
   * domain payload (e.g. HandleUartConfigure) builds a dcp::Response
   * directly and calls sender_.SendResponse() instead of going through
   * this helper. */
  void SendResponse(const dcp::FrameHeader &request_header, dcp::Status status);

  Usb &usb_;
  PeripheralsController &controller_;
  DcpSender &sender_;
  dcp::FrameParser parser_{dcp::MessageType::kRequest};
  uint32_t last_heartbeat_ = 0;
};

#ifdef __cplusplus
}
#endif

#endif // ADAPTER_CORE_INC_DCP_DCP_HANDLER_HPP
