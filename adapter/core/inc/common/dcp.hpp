#ifndef ADAPTER_CORE_INC_COMMON_DCP_HPP
#define ADAPTER_CORE_INC_COMMON_DCP_HPP

#include <stdint.h>
#include "queue.hpp"

enum class DcpCmd : uint8_t {
  kRun       = 0x01,
  kData      = 0x02,
  kAck       = 0x03,
  kErr       = 0x04,
  kGetCfg    = 0x05,
  kSetCfg    = 0x06,
  kCfg       = 0x07,
  kHeartbeat = 0x08,
  kReset     = 0x09,
};

enum class DcpInterface : uint8_t {
  kUart   = 0x00,
  kI2c    = 0x01,
  kSpi    = 0x02,
  kSystem = 0xFF,
};

enum class DcpError : uint8_t {
  kCrcFail              = 0x01,
  kVersionMismatch      = 0x02,
  kUnsupportedInterface = 0x03,
  kPayloadTooLarge      = 0x04,
  kMalformedFrame       = 0x05,
  kI2cNack              = 0x06,
  kI2cTimeout           = 0x07,
};

static constexpr uint8_t kDcpSof = 0x7E;
static constexpr uint8_t kDcpVersion = 0x01;
static constexpr uint16_t kDcpMaxPayload = 256;
static constexpr uint16_t kDcpHeaderSize = 8;
static constexpr uint16_t kDcpMaxFrameSize = kDcpHeaderSize + kDcpMaxPayload;

struct DcpFrame {
  uint8_t ver = kDcpVersion;
  DcpCmd cmd = DcpCmd::kErr;
  DcpInterface interface = DcpInterface::kSystem;
  uint8_t txn_id = 0;
  const uint8_t *payload = nullptr;
  uint16_t len = 0;
};

enum class DcpPopResult {
  kNone,          /* no complete frame available yet */
  kFrame,         /* a valid frame was decoded into `out` */
  kProtocolError, /* a malformed frame was rejected, see `err_out` */
};

uint8_t DcpCrc8(const uint8_t *data, uint16_t len);

uint16_t DcpEncode(uint8_t *dst, DcpCmd cmd, DcpInterface iface,
                    uint8_t txn_id, const uint8_t *payload,
                    uint16_t len);

class DcpParser {
 public:
  void Feed(const uint8_t *src, uint16_t len);

  DcpPopResult PopFrame(DcpFrame &out, DcpError &err_out);

 private:
  enum class State {
    kWaitSof,
    kVer,
    kCmd,
    kInterface,
    kTxnId,
    kLenHi,
    kLenLo,
    kPayload,
    kCrc,
  };

  DcpPopResult Step(uint8_t b, DcpFrame &out, DcpError &err_out);
  void Reset(void);

  Queue rx_;

  State state_ = State::kWaitSof;
  uint8_t crc_ = 0;
  uint8_t ver_ = 0;
  uint8_t cmd_ = 0;
  uint8_t iface_ = 0;
  uint8_t txn_id_ = 0;
  uint16_t len_ = 0;
  uint16_t payload_idx_ = 0;
  uint8_t payload_buf_[kDcpMaxPayload] {};
};

#endif // ADAPTER_CORE_INC_COMMON_DCP_HPP
