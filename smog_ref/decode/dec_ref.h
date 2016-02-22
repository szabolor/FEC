#ifndef DEC_REF_H
#define DEC_REF_H

#include <stdint.h>
#include "viterbi/spiral-vit_scalar.h"
#include "rs/decode_rs.h"

#define DEBUG

#define RAW_SIZE      5200
#define CONV_SIZE     5132

#define RS_SIZE        320
#define DATA_SIZE      256
#define FRAME_BITS    2560
#define RS_BLOCK_SIZE  160

extern const uint8_t Scrambler[320];

void decode_data(uint8_t raw[RAW_SIZE], uint8_t data[DATA_SIZE], int8_t error[2]);

#ifdef DEBUG
void decode_data_debug(uint8_t raw[RAW_SIZE], uint8_t data[DATA_SIZE], int8_t  error[2], uint8_t conv[CONV_SIZE], uint8_t dec_data[RS_SIZE], uint8_t rs[2][RS_BLOCK_SIZE]);
#endif

#endif