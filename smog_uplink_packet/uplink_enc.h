#ifndef UPLINK_ENC_H
#define UPLINK_ENC_H

#include <stdint.h>
#include "uplink_common.h"

void encode_data(uint8_t data[MSG_LEN], uint8_t encoded[ENC_LEN]);

#endif
