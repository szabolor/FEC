#ifndef DECODE_RS_H
#define DECODE_RS_H

#include <stdint.h> // because of uint8_t

#define NN     255  // GF(2^8-1)
#define NROOTS  32
#define FCR    112
#define PRIM    11
#define IPRIM  116
#define PAD     95

#include <stdio.h>

extern const uint8_t ALPHA_TO[];
extern const uint8_t INDEX_OF[];

#define MODNN(x) mod255(x)

static inline int mod255(int x){
  while (x >= 255) {
    x -= 255;
    x = (x >> 8) + (x & 255);
  }
  return x;
}

int8_t decode_rs_8(uint8_t *data, int *eras_pots, int no_eras);

#endif