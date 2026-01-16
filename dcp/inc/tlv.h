#ifndef TLV_H
#define TLV_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)
typedef struct {
    uint8_t  tag;
    uint16_t length;
    uint8_t  value[];
} tlv_t;
#pragma pack(pop)

int tlv_encode(uint8_t tag, const uint8_t *value, uint16_t value_len, 
               uint8_t *buf, uint16_t buf_max, uint16_t *encoded_len);

int tlv_decode(const uint8_t *buf, uint16_t buf_len, 
               uint8_t *tag, const uint8_t **value, uint16_t *value_len);

const uint8_t* tlv_find(uint8_t target_tag, const uint8_t *payload, uint16_t payload_len, uint16_t *out_len);

#ifdef __cplusplus
}
#endif

#endif
