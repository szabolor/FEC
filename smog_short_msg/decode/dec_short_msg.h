#ifndef DEC_REF_H
#define DEC_REF_H

#include <stdint.h>
#include "viterbi_short/spiral-vit_scalar_1280.h"
#include "rs/decode_rs.h"

#define DEBUG

#define INTERLEAVER_STEP_SIZE    51
#define INTERLEAVER_PILOT_BITS   80

#define RAW_SIZE      2652 // 51*52
#define CONV_SIZE     2572

#define RS_SIZE        160
#define DATA_SIZE      128
#define FRAME_BITS    1280
#define RS_BLOCK_SIZE  160

extern const uint8_t Scrambler[320];

void decode_data(uint8_t raw[RAW_SIZE], uint8_t data[DATA_SIZE], int8_t *error);

#ifdef DEBUG
void decode_data_debug(uint8_t raw[RAW_SIZE], uint8_t data[DATA_SIZE], int8_t *error, uint8_t conv[CONV_SIZE], uint8_t dec_data[RS_SIZE], uint8_t rs[RS_BLOCK_SIZE]);
#endif

#endif