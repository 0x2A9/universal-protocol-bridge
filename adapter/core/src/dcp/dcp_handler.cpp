#include "dcp/dcp_handler.hpp"

#include "board.h"
#include "device.hpp"
#include "controllers/peripherals_controller.hpp"

#include "dcp/dcp_sender.hpp"
#include "dcp/response.hpp"
#include "dcp/uart/codec.hpp"

static constexpr uint8_t kFwVersionMajor = 1;
static constexpr uint8_t kFwVersionMinor = 0;
static constexpr uint8_t kFwVersionPatch = 0;
static constexpr uint8_t kSupportedInterfaceMask = 0x01; /* bit0 = UART */

DcpHandler::DcpHandler(Usb &usb, PeripheralsController &controller, DcpSender &sender)
    : usb_(usb), controller_(controller), sender_(sender) {}

bool DcpHandler::Init(void) {
  return usb_.Init();
}

bool DcpHandler::IsReady(void) const {
  return usb_.IsReady();
}

void DcpHandler::ReadUsbRx(void) {
  uint8_t chunk[64];
  uint16_t n = usb_.DequeueRx(chunk, sizeof(chunk));
  if (n > 0) parser_.Parse(chunk, n);
}

void DcpHandler::ProcessFrames(void) {
  while (true) {
    dcp::ParseResult r = parser_.Parse(nullptr, 0);

    if (r == dcp::ParseResult::kOk) {
      dcp::Frame frame;
      if (parser_.PopFrame(frame)) Handle(frame);
      continue;
    }

    if (r == dcp::ParseResult::kNoFrame) break;

    /* Any other ParseResult is a protocol-level failure. */
    sender_.SendErrorEvent(dcp::Status::kProtocolError);
  }
}

bool DcpHandler::ProcessUartRx(void) {
  uint8_t buf[64];
  uint16_t n = controller_.ReadUart(buf, sizeof(buf));
  if (n == 0) return false;

  dcp::uart::RxEvent event{};
  event.timestamp_us = HAL_GetTick() * 1000; /* relay time, not per-byte receive time */
  event.data_length = n;
  for (uint16_t i = 0; i < n; i++) event.data[i] = buf[i];

  sender_.SendUartRxEvent(0, event);
  return true;
}

DcpHandler::ProcessResult DcpHandler::Process(void) {
  ProcessResult result{};

  ReadUsbRx();
  ProcessFrames();

  if (ProcessUartRx()) result.uart_rx_processed = true;

  usb_.ProcessTx();

  return result;
}

void DcpHandler::Tick(uint32_t now_ms) {
  if (now_ms - last_heartbeat_ >= 1000) {
    last_heartbeat_ = now_ms;
    sender_.SendHeartbeatEvent();
  }

  usb_.ProcessTx();
}

void DcpHandler::SendResponse(const dcp::FrameHeader &request_header, dcp::Status status) {
  dcp::Response response{};
  response.header = {request_header.message_type, request_header.peripheral,
                      request_header.resource_id, request_header.command,
                      request_header.request_id};
  response.status = status;
  response.payload_type = dcp::ResponsePayloadType::kNone;

  sender_.SendResponse(response);
}

void DcpHandler::Handle(const dcp::Frame &frame) {
  switch (frame.header.command) {
    case dcp::MessageId::kGetCapabilities:
      HandleGetCapabilities(frame.header);
      break;

    case dcp::MessageId::kConfigure:
      HandleConfigure(frame);
      break;

    case dcp::MessageId::kGetConfig:
      HandleGetConfig(frame);
      break;

    case dcp::MessageId::kWriteUart: {
      dcp::uart::WriteRequest request;
      if (dcp::uart::Codec::DecodeWriteRequest(frame.payload, frame.header.payload_length, request) !=
          dcp::DecodeResult::kOk) {
        SendResponse(frame.header, dcp::Status::kInvalidParameter);
        break;
      }
      HandleUartWrite(frame.header, request);
      break;
    }

    case dcp::MessageId::kReset:
      HandleReset(frame.header);
      break;

    default:
      SendResponse(frame.header, dcp::Status::kInvalidCommand);
      break;
  }
}

void DcpHandler::HandleGetCapabilities(const dcp::FrameHeader &header) {
  dcp::Response response{};
  response.header = {header.message_type, header.peripheral, header.resource_id, header.command,
                      header.request_id};
  response.status = dcp::Status::kOk;
  response.payload_type = dcp::ResponsePayloadType::kCapabilities;
  response.payload.capabilities =
      dcp::system::Capabilities{kFwVersionMajor, kFwVersionMinor, kFwVersionPatch,
                                 kSupportedInterfaceMask};

  sender_.SendResponse(response);
}

void DcpHandler::HandleConfigure(const dcp::Frame &frame) {
  switch (frame.header.peripheral) {
    case dcp::PeripheralType::kUart: {
      dcp::uart::Config config;
      if (dcp::uart::Codec::DecodeConfigRequest(frame.payload, frame.header.payload_length, config) !=
          dcp::DecodeResult::kOk) {
        SendResponse(frame.header, dcp::Status::kInvalidParameter);
        return;
      }
      HandleUartConfigure(frame.header, config);
      return;
    }
    default:
      SendResponse(frame.header, dcp::Status::kNotSupported);
  }
}

void DcpHandler::HandleGetConfig(const dcp::Frame &frame) {
  switch (frame.header.peripheral) {
    case dcp::PeripheralType::kUart:
      HandleUartGetConfig(frame.header);
      return;
    default:
      SendResponse(frame.header, dcp::Status::kNotSupported);
  }
}

void DcpHandler::HandleUartConfigure(const dcp::FrameHeader &header,
                                      const dcp::uart::Config &config) {
  controller_.ConfigureUart(config);

  dcp::Response response{};
  response.header = {header.message_type, header.peripheral, header.resource_id, header.command,
                      header.request_id};
  response.status = dcp::Status::kOk;
  response.payload_type = dcp::ResponsePayloadType::kUartConfig;
  response.payload.uart_config = controller_.GetUartConfig();

  sender_.SendResponse(response);
}

void DcpHandler::HandleUartGetConfig(const dcp::FrameHeader &header) {
  dcp::Response response{};
  response.header = {header.message_type, header.peripheral, header.resource_id, header.command,
                      header.request_id};
  response.status = dcp::Status::kOk;
  response.payload_type = dcp::ResponsePayloadType::kUartConfig;
  response.payload.uart_config = controller_.GetUartConfig();

  sender_.SendResponse(response);
}

void DcpHandler::HandleUartWrite(const dcp::FrameHeader &header,
                                  const dcp::uart::WriteRequest &request) {
  const uint16_t accepted = controller_.WriteUart(request.data, request.data_length);

  dcp::Response response{};
  response.header = {header.message_type, header.peripheral, header.resource_id, header.command,
                      header.request_id};
  response.status = dcp::Status::kOk;
  response.payload_type = dcp::ResponsePayloadType::kUartWrite;
  response.payload.uart_write = dcp::uart::WriteResponse{accepted};

  sender_.SendResponse(response);
}

void DcpHandler::HandleReset(const dcp::FrameHeader &header) {
  switch (header.peripheral) {
    case dcp::PeripheralType::kUart:
      controller_.ResetUart();
      SendResponse(header, dcp::Status::kOk);
      return;

    case dcp::PeripheralType::kSystem:
      SendResponse(header, dcp::Status::kOk);
      usb_.ProcessTx();
      controller_.ResetSystem();
      return;

    default:
      SendResponse(header, dcp::Status::kNotSupported);
  }
}
