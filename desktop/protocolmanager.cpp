#include "protocolmanager.h"
#include <QDebug>

ProtocolManager::ProtocolManager(QSerialPort *port, QObject *parent)
    : QObject(parent), m_serialPort(port)
{
    proto_init(&m_ctx, (proto_tx_fn)ProtocolManager::txCallback);

    m_ctx.user_data = static_cast<void*>(this);

    m_ctx.on_msg_received = ProtocolManager::rxCallback;

    if (m_serialPort) {
        connect(m_serialPort, &QSerialPort::readyRead, this, &ProtocolManager::handleReadyRead);
    }
}

void ProtocolManager::handleReadyRead()
{
    QByteArray data = m_serialPort->readAll();
    if (!data.isEmpty()) {
        proto_process_buffer(&m_ctx, reinterpret_cast<const uint8_t*>(data.data()), data.size());
    }
}

void ProtocolManager::sendCommand(uint16_t cmdId, const QByteArray &data)
{
    qDebug() << "HALLO";

    proto_send_command(&m_ctx, (proto_cmd_t)cmdId,
                       reinterpret_cast<const uint8_t*>(data.data()),
                       static_cast<uint16_t>(data.size()));
}

void ProtocolManager::sendConfig(uint16_t cmdId, const QMap<uint8_t, QByteArray> &tlvs)
{
    uint8_t buffer[PROTO_MAX_PAYLOAD];
    uint16_t offset = 0;

    for (auto it = tlvs.begin(); it != tlvs.end(); ++it) {
        uint16_t encoded_len = 0;
        tlv_encode(it.key(), reinterpret_cast<const uint8_t*>(it.value().data()),
                   it.value().size(), &buffer[offset], PROTO_MAX_PAYLOAD - offset, &encoded_len);
        offset += encoded_len;
    }

    proto_send_command(&m_ctx, (proto_cmd_t)cmdId, buffer, offset);
}


void ProtocolManager::txCallback(proto_ctx_t *ctx, const uint8_t *data, size_t len)
{
    ProtocolManager *self = static_cast<ProtocolManager*>(ctx->user_data);
    if (self && self->m_serialPort && self->m_serialPort->isOpen()) {
        self->m_serialPort->write(reinterpret_cast<const char*>(data), len);
    }
}

void ProtocolManager::rxCallback(proto_ctx_t *ctx, uint8_t type, uint16_t cmd_id, const uint8_t *data, uint16_t len)
{
    ProtocolManager *self = static_cast<ProtocolManager*>(ctx->user_data);
    if (!self) return;

    QByteArray payload(reinterpret_cast<const char*>(data), len);

    proto_msg_type_t msgType = static_cast<proto_msg_type_t>(type);
    switch (type) {
    case MSG_COMMAND:
        emit self->messageReceived(cmd_id, payload);
        break;
    case MSG_ACK: {
        if (len >= 2) { 
            uint16_t status = qFromBigEndian<uint16_t>(reinterpret_cast<const uint8_t*>(payload.data()));
            emit self->ackReceived(cmd_id, status, payload.mid(2));
        }
        break;
    }
    case MSG_ERROR:
        emit self->errorReceived(cmd_id, 0); 
        break;
    default:
        break;
    }
}
