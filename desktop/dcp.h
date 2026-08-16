#ifndef DCP_H
#define DCP_H

#include <QByteArray>
#include <QString>
#include <cstdint>

enum class DcpCmd : uint8_t {
    Run       = 0x01,
    Data      = 0x02,
    Ack       = 0x03,
    Err       = 0x04,
    GetCfg    = 0x05,
    SetCfg    = 0x06,
    Cfg       = 0x07,
    Heartbeat = 0x08,
    Reset     = 0x09,
};

enum class DcpInterface : uint8_t {
    Uart   = 0x00,
    I2c    = 0x01,
    Spi    = 0x02,
    System = 0xFF,
};

enum class DcpError : uint8_t {
    CrcFail              = 0x01,
    VersionMismatch      = 0x02,
    UnsupportedInterface = 0x03,
    PayloadTooLarge      = 0x04,
    MalformedFrame       = 0x05,
    I2cNack              = 0x06,
    I2cTimeout           = 0x07,
};

QString dcpErrorToString(DcpError err);
QString dcpCmdToString(DcpCmd cmd);
QString dcpInterfaceToString(DcpInterface iface);

constexpr uint8_t kDcpSof = 0x7E;
constexpr uint8_t kDcpVersion = 0x01;
constexpr int kDcpMaxPayload = 256;

struct DcpFrame {
    uint8_t ver = kDcpVersion;
    DcpCmd cmd = DcpCmd::Err;
    DcpInterface interface = DcpInterface::System;
    uint8_t txnId = 0;
    QByteArray payload;
};

enum class DcpPopResult {
    None,          /* no complete frame available yet */
    Frame,         /* a valid frame was decoded into `out` */
    ProtocolError, /* a malformed frame was rejected, see `errOut` */
};

uint8_t dcpCrc8(const uint8_t *data, int len);

QByteArray dcpEncode(DcpCmd cmd, DcpInterface iface, uint8_t txnId,
                      const QByteArray &payload = QByteArray());

class DcpParser {
public:
    void feed(const QByteArray &data);

    DcpPopResult popFrame(DcpFrame &out, DcpError &errOut);

private:
    enum class State {
        WaitSof,
        Ver,
        Cmd,
        Interface,
        TxnId,
        LenHi,
        LenLo,
        Payload,
        Crc,
    };

    DcpPopResult step(uint8_t b, DcpFrame &out, DcpError &errOut);
    void reset();

    QByteArray rx_;
    int rxPos_ = 0;

    State state_ = State::WaitSof;
    uint8_t crc_ = 0;
    uint8_t ver_ = 0;
    uint8_t cmd_ = 0;
    uint8_t iface_ = 0;
    uint8_t txnId_ = 0;
    uint16_t len_ = 0;
    QByteArray payloadBuf_;
};

#endif // DCP_H
