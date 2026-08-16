#include "dcp.hpp"

static uint8_t Crc8Update(uint8_t crc, uint8_t byte) {
  crc ^= byte;

  for (int i = 0; i < 8; i++) {
    crc = (crc & 0x80) ? uint8_t((crc << 1) ^ 0x07) : uint8_t(crc << 1);
  }

  return crc;
}

uint8_t DcpCrc8(const uint8_t *data, uint16_t len) {
  uint8_t crc = 0x00;
  
  for (uint16_t i = 0; i < len; i++) crc = Crc8Update(crc, data[i]);
  
  return crc;
}

uint16_t DcpEncode(uint8_t *dst, DcpCmd cmd, DcpInterface iface,
                    uint8_t txn_id, const uint8_t *payload,
                    uint16_t len) {
  if (len > kDcpMaxPayload) len = kDcpMaxPayload;

  uint16_t i = 0;
  dst[i++] = kDcpSof;

  const uint16_t crc_start = i;
  dst[i++] = kDcpVersion;
  dst[i++] = static_cast<uint8_t>(cmd);
  dst[i++] = static_cast<uint8_t>(iface);
  dst[i++] = txn_id;
  dst[i++] = uint8_t(len >> 8);
  dst[i++] = uint8_t(len & 0xFF);

  for (uint16_t j = 0; j < len; j++) dst[i++] = payload[j];

  const uint8_t crc = DcpCrc8(&dst[crc_start], uint16_t(i - crc_start));
  dst[i++] = crc;

  return i;
}

void DcpParser::Feed(const uint8_t *src, uint16_t len) {
  rx_.Push(src, len);
}

void DcpParser::Reset(void) {
  state_ = State::kWaitSof;
}

DcpPopResult DcpParser::Step(uint8_t b, DcpFrame &out, DcpError &err_out) {
  switch (state_) {
    case State::kWaitSof:
      if (b == kDcpSof) {
        crc_ = 0;
        state_ = State::kVer;
      }

      return DcpPopResult::kNone;

    case State::kVer:
      ver_ = b;
      crc_ = Crc8Update(crc_, b);
      
      if (ver_ != kDcpVersion) {
        out = DcpFrame{};
        out.ver = ver_;
        out.cmd = DcpCmd::kErr;
        out.interface = DcpInterface::kSystem;
        out.txn_id = 0;
        err_out = DcpError::kVersionMismatch;
        Reset();
        
        return DcpPopResult::kProtocolError;
      }
      
      state_ = State::kCmd;
      
      return DcpPopResult::kNone;

    case State::kCmd:
      cmd_ = b;
      crc_ = Crc8Update(crc_, b);
      state_ = State::kInterface;
      
      return DcpPopResult::kNone;

    case State::kInterface:
      iface_ = b;
      crc_ = Crc8Update(crc_, b);
      state_ = State::kTxnId;
      
      return DcpPopResult::kNone;

    case State::kTxnId:
      txn_id_ = b;
      crc_ = Crc8Update(crc_, b);
      state_ = State::kLenHi;
      
      return DcpPopResult::kNone;

    case State::kLenHi:
      len_ = uint16_t(uint16_t(b) << 8);
      crc_ = Crc8Update(crc_, b);
      state_ = State::kLenLo;
      
      return DcpPopResult::kNone;

    case State::kLenLo:
      len_ = uint16_t(len_ | b);
      crc_ = Crc8Update(crc_, b);
      
      if (len_ > kDcpMaxPayload) {
        out = DcpFrame{};
        out.ver = ver_;
        out.cmd = DcpCmd::kErr;
        out.interface = static_cast<DcpInterface>(iface_);
        out.txn_id = txn_id_;
        err_out = DcpError::kPayloadTooLarge;
        Reset();
        
        return DcpPopResult::kProtocolError;
      }

      payload_idx_ = 0;
      state_ = (len_ == 0) ? State::kCrc : State::kPayload;
      
      return DcpPopResult::kNone;

    case State::kPayload:
      payload_buf_[payload_idx_++] = b;
      crc_ = Crc8Update(crc_, b);
      
      if (payload_idx_ >= len_) state_ = State::kCrc;
      
      return DcpPopResult::kNone;

    case State::kCrc:
      if (b != crc_) {
        out = DcpFrame{};
        out.ver = ver_;
        out.cmd = DcpCmd::kErr;
        out.interface = static_cast<DcpInterface>(iface_);
        out.txn_id = txn_id_;
        err_out = DcpError::kCrcFail;
        
        Reset();
        
        return DcpPopResult::kProtocolError;
      }

      out.ver = ver_;
      out.cmd = static_cast<DcpCmd>(cmd_);
      out.interface = static_cast<DcpInterface>(iface_);
      out.txn_id = txn_id_;
      out.payload = payload_buf_;
      out.len = len_;
      
      Reset();
      
      return DcpPopResult::kFrame;
  }

  return DcpPopResult::kNone;
}

DcpPopResult DcpParser::PopFrame(DcpFrame &out, DcpError &err_out) {
  uint8_t b;
  
  while (rx_.Pop(&b, 1) == 1) {
    DcpPopResult r = Step(b, out, err_out);
    
    if (r != DcpPopResult::kNone) return r;
  }

  return DcpPopResult::kNone;
}
