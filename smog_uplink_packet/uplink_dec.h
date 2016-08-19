#ifndef UPLINK_DEC_H
#define UPLINK_DEC_H

#include <stdint.h>

#define GEN_POLY   (0xAE3U)

// Data size to be encoded (interpreted as payload length) 
// [MUST BE DIVISIBLE BY 3]
#define MSG_LEN    (31)

// Encoder output size in bytes
#define ENC_LEN    (63)

// The amount of Golay code-words: ceil(MSG_LEN/1.5)
#define WORD_COUNT (21)

// Interleaver settings: 24x21 matrix
#define INTERLEAVER_STEP_SIZE (21)
#define INTERLEAVER_SIZE     (504)

#define LSB23BIT    (0x7fffff)
#define PARITY_MASK (0x800000)

// in: ENC_LEN; out: MSG_LEN
void decode_data(uint8_t *encoded, uint8_t *data, int *error_count, int *fatal);

#endif
