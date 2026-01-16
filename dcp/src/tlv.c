#include "tlv.h"
#include <string.h>

int tlv_encode(uint8_t tag, const uint8_t *value, uint16_t value_len, 
               uint8_t *buf, uint16_t buf_max, uint16_t *encoded_len) {
    
    uint16_t total_needed = 3 + value_len;

    if (!buf || !encoded_len || total_needed > buf_max) {
        return -1; 
    }

    buf[0] = tag;
    buf[1] = (uint8_t)(value_len >> 8);
    buf[2] = (uint8_t)(value_len & 0xFF);

    if (value_len > 0 && value) {
        memcpy(&buf[3], value, value_len);
    }

    *encoded_len = total_needed;
    return 0;
}

int tlv_decode(const uint8_t *buf, uint16_t buf_len, 
               uint8_t *tag, const uint8_t **value, uint16_t *value_len) {
    
    if (!buf || buf_len < 3 || !tag || !value || !value_len) {
        return -1;
    }

    *tag = buf[0];
    *value_len = ((uint16_t)buf[1] << 8) | buf[2];

    if (buf_len < (3 + *value_len)) {
        return -1;
    }

    *value = &buf[3]
    return 0;
}

const uint8_t* tlv_find(uint8_t target_tag, const uint8_t *payload, uint16_t payload_len, uint16_t *out_len) {
    uint16_t offset = 0;
    
    while (offset + 3 <= payload_len) {
        uint8_t  current_tag = payload[offset];
        uint16_t current_len = ((uint16_t)payload[offset + 1] << 8) | payload[offset + 2];
        
        if (current_tag == target_tag) {
            if (offset + 3 + current_len <= payload_len) {
                *out_len = current_len;
                return &payload[offset + 3];
            }
            return NULL;
        }
        
        offset += (3 + current_len);
    }
    
    return NULL;
}
