#include "protocol.h"
#include <string.h>

static void crc16_update(uint16_t *crc, uint8_t data) {
    *crc ^= (uint16_t)data << 8;
    for (uint8_t i = 0; i < 8; i++) {
        if (*crc & 0x8000) *crc = (*crc << 1) ^ 0x1021;
        else *crc <<= 1;
    }
}
void proto_init(proto_ctx_t *ctx, proto_tx_fn tx_fn) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(proto_ctx_t));
    ctx->tx_func = tx_fn;
    ctx->state = STATE_WAIT_SOF;
}

void proto_process_byte(proto_ctx_t *ctx, uint8_t byte) {
    switch (ctx->state) {
        case STATE_WAIT_SOF:
            if (byte == PROTO_SOF) {
                ctx->state = STATE_WAIT_VER;
                ctx->running_crc = 0xFFFF;
            }
            break;

        case STATE_WAIT_VER:
            if (byte == PROTO_VERSION) {
                crc16_update(&ctx->running_crc, byte);
                ctx->state = STATE_WAIT_TYPE;
            } else {
                ctx->state = STATE_WAIT_SOF;
            }
            break;

        case STATE_WAIT_TYPE:
            ctx->msg_type = byte;
            crc16_update(&ctx->running_crc, byte);
            ctx->state = STATE_WAIT_LEN_H;
            break;

        case STATE_WAIT_LEN_H:
            ctx->msg_len = (uint16_t)byte << 8;
            crc16_update(&ctx->running_crc, byte);
            ctx->state = STATE_WAIT_LEN_L;
            break;

        case STATE_WAIT_LEN_L:
            ctx->msg_len |= byte;
            crc16_update(&ctx->running_crc, byte);
            ctx->rx_index = 0;
            if (ctx->msg_len > PROTO_MAX_PAYLOAD) ctx->state = STATE_WAIT_SOF;
            else ctx->state = (ctx->msg_len > 0) ? STATE_WAIT_PAYLOAD : STATE_WAIT_CRC_H;
            break;

        case STATE_WAIT_PAYLOAD:
            ctx->payload[ctx->rx_index++] = byte;
            crc16_update(&ctx->running_crc, byte);
            if (ctx->rx_index >= ctx->msg_len) ctx->state = STATE_WAIT_CRC_H;
            break;

        case STATE_WAIT_CRC_H:
            ctx->msg_crc = (uint16_t)byte << 8;
            ctx->state = STATE_WAIT_CRC_L;
            break;

        case STATE_WAIT_CRC_L:
            ctx->msg_crc |= byte;
            if (ctx->msg_crc == ctx->running_crc && ctx->on_msg_received) {
                if (ctx->msg_len >= 2) {
                    uint16_t cmd_id = (ctx->payload[0] << 8) | ctx->payload[1];
                    ctx->on_msg_received(ctx, (proto_msg_type_t)ctx->msg_type, cmd_id, &ctx->payload[2], ctx->msg_len - 2);
                }
            }
            ctx->state = STATE_WAIT_SOF;
            break;
    }
}

void proto_process_buffer(proto_ctx_t *ctx, const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i++) proto_process_byte(ctx, buf[i]);
}

static void send_raw_frame(proto_ctx_t *ctx, uint8_t type, const uint8_t *payload, uint16_t len) {
    if (!ctx || !ctx->tx_func) return;

        if (!ctx->tx_func) return;

    uint8_t header[5];
    header[0] = PROTO_SOF;
    header[1] = PROTO_VERSION;
    header[2] = (uint8_t)type;
    header[3] = (uint8_t)(len >> 8);
    header[4] = (uint8_t)(len & 0xFF);

    uint16_t crc = 0xFFFF;
    for(int i=1; i<5; i++) crc16_update(&crc, header[i]);
    for(int i=0; i<len; i++) crc16_update(&crc, payload[i]);

    uint8_t footer[2] = {(uint8_t)(crc >> 8), (uint8_t)(crc & 0xFF)};

    ctx->tx_func(ctx, header, 5); 
    if (len > 0) ctx->tx_func(ctx, payload, len);
    ctx->tx_func(ctx, footer, 2);
}

int proto_send_command(proto_ctx_t *ctx, uint16_t cmd, const uint8_t *data, uint16_t len) {
    uint8_t tmp_buf[PROTO_MAX_PAYLOAD];
    tmp_buf[0] = (uint8_t)(cmd >> 8);
    tmp_buf[1] = (uint8_t)(cmd & 0xFF);
    if (len > 0) memcpy(&tmp_buf[2], data, len);
    send_raw_frame(ctx, MSG_COMMAND, tmp_buf, len + 2);
    return 0;
}
