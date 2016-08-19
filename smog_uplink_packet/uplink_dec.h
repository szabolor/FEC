#ifndef UPLINK_DEC_H
#define UPLINK_DEC_H

#include <stdint.h>
#include "uplink_common.h"

#define LSB23BIT    (0x7fffff)
#define PARITY_MASK (0x800000)

void decode_data(uint8_t *encoded, uint8_t *data, int *error_count, int *fatal);

#endif
