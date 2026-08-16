#include "dcp.h"

static uint8_t dcpCrc8Update(uint8_t crc, uint8_t byte) {
    crc ^= byte;
    for (int i = 0; i < 8; i++) {
        crc = (crc & 0x80) ? uint8_t((crc << 1) ^ 0x07) : uint8_t(crc << 1);
    }
    
    return crc;
}

uint8_t dcpCrc8(const uint8_t *data, int len) {
    uint8_t crc = 0x00;
    for (int i = 0; i < len; i++) crc = dcpCrc8Update(crc, data[i]);

    return crc;
}

QString dcpCmdToString(DcpCmd cmd) {
    switch (cmd) {
    case DcpCmd::Run:       return QStringLiteral("RUN");
    case DcpCmd::Data:      return QStringLiteral("DATA");
    case DcpCmd::Ack:       return QStringLiteral("ACK");
    case DcpCmd::Err:       return QStringLiteral("ERR");
    case DcpCmd::GetCfg:    return QStringLiteral("GET_CFG");
    case DcpCmd::SetCfg:    return QStringLiteral("SET_CFG");
    case DcpCmd::Cfg:       return QStringLiteral("CFG");
    case DcpCmd::Heartbeat: return QStringLiteral("HEARTBEAT");
    case DcpCmd::Reset:     return QStringLiteral("RESET");
    }

    return QStringLiteral("UNKNOWN_CMD");
}

QString dcpInterfaceToString(DcpInterface iface) {
    switch (iface) {
    case DcpInterface::Uart:   return QStringLiteral("UART");
    case DcpInterface::I2c:    return QStringLiteral("I2C");
    case DcpInterface::Spi:    return QStringLiteral("SPI");
    case DcpInterface::System: return QStringLiteral("SYSTEM");
    }

    return QStringLiteral("UNKNOWN_INTERFACE");
}

QString dcpErrorToString(DcpError err) {
    switch (err) {
    case DcpError::CrcFail:              return QStringLiteral("CRC_FAIL");
    case DcpError::VersionMismatch:      return QStringLiteral("VERSION_MISMATCH");
    case DcpError::UnsupportedInterface: return QStringLiteral("UNSUPPORTED_INTERFACE");
    case DcpError::PayloadTooLarge:      return QStringLiteral("PAYLOAD_TOO_LARGE");
    case DcpError::MalformedFrame:       return QStringLiteral("MALFORMED_FRAME");
    case DcpError::I2cNack:              return QStringLiteral("I2C_NACK");
    case DcpError::I2cTimeout:           return QStringLiteral("I2C_TIMEOUT");
    }

    return QStringLiteral("UNKNOWN_ERROR");
}

QByteArray dcpEncode(DcpCmd cmd, DcpInterface iface, uint8_t txnId,
                      const QByteArray &payload) {
    QByteArray body;
    QByteArray truncated = payload;
    if (truncated.size() > kDcpMaxPayload) truncated.truncate(kDcpMaxPayload);

    body.reserve(6 + truncated.size());
    body.append(char(kDcpVersion));
    body.append(char(static_cast<uint8_t>(cmd)));
    body.append(char(static_cast<uint8_t>(iface)));
    body.append(char(txnId));
    body.append(char(uint8_t(truncated.size() >> 8)));
    body.append(char(uint8_t(truncated.size() & 0xFF)));
    body.append(truncated);

    QByteArray frame;
    frame.reserve(2 + body.size());
    frame.append(char(kDcpSof));
    frame.append(body);
    frame.append(char(dcpCrc8(reinterpret_cast<const uint8_t *>(body.constData()),
                               body.size())));

    return frame;
}

void DcpParser::feed(const QByteArray &data) {
    rx_.append(data);
}

void DcpParser::reset() {
    state_ = State::WaitSof;
}

DcpPopResult DcpParser::step(uint8_t b, DcpFrame &out, DcpError &errOut) {
    switch (state_) {
    case State::WaitSof:
        if (b == kDcpSof) {
            crc_ = 0;
            state_ = State::Ver;
        }

        return DcpPopResult::None;

    case State::Ver:
        ver_ = b;
        crc_ = dcpCrc8Update(crc_, b);

        if (ver_ != kDcpVersion) {
            out = DcpFrame{};
            out.ver = ver_;
            out.cmd = DcpCmd::Err;
            out.interface = DcpInterface::System;
            out.txnId = 0;
            errOut = DcpError::VersionMismatch;
            reset();
            return DcpPopResult::ProtocolError;
        }

        state_ = State::Cmd;

        return DcpPopResult::None;

    case State::Cmd:
        cmd_ = b;
        crc_ = dcpCrc8Update(crc_, b);
        state_ = State::Interface;

        return DcpPopResult::None;

    case State::Interface:
        iface_ = b;
        crc_ = dcpCrc8Update(crc_, b);
        state_ = State::TxnId;

        return DcpPopResult::None;

    case State::TxnId:
        txnId_ = b;
        crc_ = dcpCrc8Update(crc_, b);
        state_ = State::LenHi;

        return DcpPopResult::None;

    case State::LenHi:
        len_ = uint16_t(uint16_t(b) << 8);
        crc_ = dcpCrc8Update(crc_, b);
        state_ = State::LenLo;

        return DcpPopResult::None;

    case State::LenLo:
        len_ = uint16_t(len_ | b);
        crc_ = dcpCrc8Update(crc_, b);

        if (len_ > kDcpMaxPayload) {
            out = DcpFrame{};
            out.ver = ver_;
            out.cmd = DcpCmd::Err;
            out.interface = static_cast<DcpInterface>(iface_);
            out.txnId = txnId_;
            errOut = DcpError::PayloadTooLarge;
            reset();

            return DcpPopResult::ProtocolError;
        }

        payloadBuf_.clear();
        payloadBuf_.reserve(len_);
        state_ = (len_ == 0) ? State::Crc : State::Payload;

        return DcpPopResult::None;

    case State::Payload:
        payloadBuf_.append(char(b));
        crc_ = dcpCrc8Update(crc_, b);
        if (payloadBuf_.size() >= int(len_)) state_ = State::Crc;

        return DcpPopResult::None;

    case State::Crc:
        if (b != crc_) {
            out = DcpFrame{};
            out.ver = ver_;
            out.cmd = DcpCmd::Err;
            out.interface = static_cast<DcpInterface>(iface_);
            out.txnId = txnId_;
            errOut = DcpError::CrcFail;
            reset();

            return DcpPopResult::ProtocolError;
        }

        out.ver = ver_;
        out.cmd = static_cast<DcpCmd>(cmd_);
        out.interface = static_cast<DcpInterface>(iface_);
        out.txnId = txnId_;
        out.payload = payloadBuf_;
        reset();

        return DcpPopResult::Frame;
    }

    return DcpPopResult::None;
}

DcpPopResult DcpParser::popFrame(DcpFrame &out, DcpError &errOut) {
    DcpPopResult result = DcpPopResult::None;

    while (rxPos_ < rx_.size()) {
        uint8_t b = static_cast<uint8_t>(rx_.at(rxPos_++));
        result = step(b, out, errOut);

        if (result != DcpPopResult::None) break;
    }

    if (rxPos_ > 0) {
        rx_.remove(0, rxPos_);
        rxPos_ = 0;
    }

    return result;
}
