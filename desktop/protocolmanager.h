#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QtEndian>
#include "protocol.h"
#include "tlv.h"

class ProtocolManager : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolManager(QSerialPort *port, QObject *parent = nullptr);

    void sendCommand(uint16_t cmdId, const QByteArray &data = QByteArray());
    void sendConfig(uint16_t cmdId, const QMap<uint8_t, QByteArray> &tlvs);

signals:
    void messageReceived(uint16_t cmdId, QByteArray payload);
    void ackReceived(uint16_t cmdId, uint16_t status, QByteArray data);
    void errorReceived(uint16_t cmdId, uint16_t errorCode);

public slots:
    void handleReadyRead();

private:
    proto_ctx_t m_ctx;
    QSerialPort *m_serialPort;

    static void txCallback(proto_ctx_t *ctx, const uint8_t *data, size_t len);
    static void rxCallback(proto_ctx_t *ctx, uint8_t type, uint16_t cmd_id, const uint8_t *data, uint16_t len);
};

#endif // PROTOCOLMANAGER_H
