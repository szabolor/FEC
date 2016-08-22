#ifndef __UPLINK_CODEC_COMMON_H__
#define __UPLINK_CODEC_COMMON_H__

#define GEN_POLY   (0xAE3U)

// Data size to be encoded (interpreted as payload length) 
// only the first 31 byte and the LSB 4 bits of data[31] will
// be coded!
#define MSG_LEN    (32)

// Encoder output size in bytes [must be multiples of 3]
#define ENC_LEN    (63)

// The amount of Golay code-words: ceil(MSG_LEN/1.5)
#define WORD_COUNT (21)

// Interleaver settings: 24x21 matrix
#define INTERLEAVER_STEP_SIZE (21)
#define INTERLEAVER_SIZE     (504)

#endif
