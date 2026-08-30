#include "dcp/dcp_sender.hpp"

#include "device.hpp"

#include "dcp/core/frame_encoder.hpp"
#include "dcp/internal/byte_writer.hpp"
#include "dcp/system/codec.hpp"
#include "dcp/uart/codec.hpp"

bool DcpSender::SendFrame(const dcp::FrameHeader &header, const uint8_t *payload,
                           uint16_t payload_length) {
  const auto result =
      dcp::FrameEncoder::Encode(header, payload, payload_length, tx_buf_, sizeof(tx_buf_));
  if (result.status != dcp::EncodeStatus::kOk) return false;

  return usb_.EnqueueTx(tx_buf_, result.size);
}

bool DcpSender::EncodeResponsePayload(const dcp::Response &response, uint8_t *dst,
                                       uint16_t capacity, uint16_t &out_len) {
  dcp::internal::ByteWriter writer(dst, capacity);
  if (!writer.WriteU16(static_cast<uint16_t>(response.status))) return false;

  const uint16_t remaining_capacity = static_cast<uint16_t>(capacity - writer.Size());
  dcp::EncodeResult domain_result{dcp::EncodeStatus::kOk, 0};

  switch (response.payload_type) {
    case dcp::ResponsePayloadType::kNone:
      break;
    case dcp::ResponsePayloadType::kCapabilities:
      domain_result = dcp::system::Codec::EncodeCapabilitiesResponse(
          response.payload.capabilities, dst + writer.Size(), remaining_capacity);
      break;
    case dcp::ResponsePayloadType::kUartConfig:
      domain_result = dcp::uart::Codec::EncodeConfig(response.payload.uart_config,
                                                       dst + writer.Size(), remaining_capacity);
      break;
    case dcp::ResponsePayloadType::kUartWrite:
      domain_result = dcp::uart::Codec::EncodeWriteResponse(
          response.payload.uart_write, dst + writer.Size(), remaining_capacity);
      break;
  }

  if (domain_result.status != dcp::EncodeStatus::kOk) return false;

  out_len = static_cast<uint16_t>(writer.Size() + domain_result.size);
  return true;
}

bool DcpSender::SendResponse(const dcp::Response &response) {
  uint8_t payload[dcp::kMaxPayloadSize];
  uint16_t payload_len = 0;
  if (!EncodeResponsePayload(response, payload, sizeof(payload), payload_len)) return false;

  dcp::FrameHeader header{};
  header.version = dcp::kProtocolVersion;
  header.message_type = dcp::MessageType::kResponse;
  header.peripheral = response.header.peripheral;
  header.resource_id = response.header.resource_id;
  header.command = response.header.command;
  header.request_id = response.header.request_id;

  return SendFrame(header, payload, payload_len);
}

bool DcpSender::SendUartRxEvent(uint8_t resource_id, const dcp::uart::RxEvent &event) {
  uint8_t payload[dcp::kMaxPayloadSize];
  const auto result = dcp::uart::Codec::EncodeRxEvent(event, payload, sizeof(payload));
  if (result.status != dcp::EncodeStatus::kOk) return false;

  dcp::FrameHeader header{};
  header.version = dcp::kProtocolVersion;
  header.message_type = dcp::MessageType::kEvent;
  header.peripheral = dcp::PeripheralType::kUart;
  header.resource_id = resource_id;
  header.command = dcp::MessageId::kUartRxEvent;

  return SendFrame(header, payload, result.size);
}

bool DcpSender::SendHeartbeatEvent(void) {
  dcp::FrameHeader header{};
  header.version = dcp::kProtocolVersion;
  header.message_type = dcp::MessageType::kEvent;
  header.peripheral = dcp::PeripheralType::kSystem;
  header.resource_id = 0;
  header.command = dcp::MessageId::kHeartbeatEvent;

  return SendFrame(header, nullptr, 0);
}

bool DcpSender::SendErrorEvent(dcp::Status status) {
  uint8_t payload[4];
  const auto result = dcp::system::Codec::EncodeErrorEvent(status, payload, sizeof(payload));
  if (result.status != dcp::EncodeStatus::kOk) return false;

  dcp::FrameHeader header{};
  header.version = dcp::kProtocolVersion;
  header.message_type = dcp::MessageType::kEvent;
  header.peripheral = dcp::PeripheralType::kSystem;
  header.resource_id = 0;
  header.command = dcp::MessageId::kErrorEvent;

  return SendFrame(header, payload, result.size);
}
