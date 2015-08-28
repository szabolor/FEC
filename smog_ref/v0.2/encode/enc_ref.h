#ifndef ENC_REF_H
#define ENC_REF_H

#include <stdint.h>

#define LOW_MEMORY        1 // low memory workaround to avoid LUT (and preserve 512byte)
#define ENABLE_BIT_OUTPUT 1 // enable 
#define MSBFISRT          1 // the first bit of bytes of encode_data_bit is the MSB

void encode_data(uint8_t (*data)[256], uint8_t (*encoded)[650]);

#if ENABLE_BIT_OUTPUT > 0
void encode_data_bit(uint8_t (*data)[256], uint8_t (*bit_encoded)[5200]);
#endif

#endif
