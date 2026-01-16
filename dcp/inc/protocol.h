#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTO_SOF         0xAD
#define PROTO_VERSION     0x01

#define PROTO_MAX_PAYLOAD 1024
typedef uint16_t proto_cmd_t;

// WINDOWS
// WINDOWS

// LINUX
// LINUX

typedef enum {
    MSG_COMMAND = 0x01,
    MSG_ACK     = 0x02,
    MSG_MESSAGE = 0x03,
    MSG_ERROR   = 0x04,
} proto_msg_type_t;

typedef enum {
    STATUS_OK       = 0x0000,
    ERR_UNKNOWN_CMD = 0x0002,
    ERR_INVALID_VAL = 0x0003,
    ERR_TIMEOUT     = 0x0004,
} proto_status_t;

typedef enum {
    STATE_WAIT_SOF,
    STATE_WAIT_VER,
    STATE_WAIT_TYPE,
    STATE_WAIT_LEN_H,
    STATE_WAIT_LEN_L,
    STATE_WAIT_PAYLOAD,
    STATE_WAIT_CRC_H,
    STATE_WAIT_CRC_L
} proto_state_t;

typedef void (*proto_tx_fn)(struct proto_ctx_s *ctx, const uint8_t *data, size_t len);

typedef void (*proto_rx_callback_fn)(struct proto_ctx_s *ctx, uint8_t type, uint16_t cmd_id, const uint8_t *data, uint16_t len);

typedef struct proto_ctx_s {
    proto_state_t state;
    uint8_t  msg_type;
    uint16_t msg_len;
    uint16_t msg_crc;
    uint16_t rx_index;
    uint16_t running_crc;
    uint8_t  payload[PROTO_MAX_PAYLOAD];

    void* user_data; 

    proto_tx_fn tx_func;

    void (*on_msg_received)(struct proto_ctx_s *ctx, uint8_t type, uint16_t cmd_id, const uint8_t *data, uint16_t len);
} proto_ctx_t;

void proto_init(proto_ctx_t *ctx, proto_tx_fn tx_fn);
void proto_process_byte(proto_ctx_t *ctx, uint8_t byte);
void proto_process_buffer(proto_ctx_t *ctx, const uint8_t *buf, size_t len);

int proto_send_command(proto_ctx_t *ctx, uint16_t cmd, const uint8_t *payload, uint16_t len);
int proto_send_ack(proto_ctx_t *ctx, uint16_t cmd, proto_status_t status, const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif
#endif
