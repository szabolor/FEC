#ifndef ENC_SHORT_REF_H
#define ENC_SHORT_REF_H

#include <stdint.h>

//#define LOW_MEMORY          // low memory workaround to avoid LUT (and preserve 512byte)
//#define ENABLE_BIT_OUTPUT  // enable debug bit output mode
//#define MSBFISRT           // the first bit of bytes of encode_data_bit is the MSB

void encode_short_data(uint8_t *data, uint8_t *encoded);

#ifdef ENABLE_BIT_OUTPUT
void encode_short_data_bit(uint8_t *data, uint8_t *bit_encoded);
#endif

#endif
