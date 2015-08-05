/* 
 * Reference encoder for proposed coded AO-40 telemetry format - v1.0  7 Jan 2002
 * Copyright 2002, Phil Karn, KA9Q
 * This software may be used under the terms of the GNU Public License (GPL)
 */

#include <stdint.h>

#define LOW_MEMORY 0 // set memory workarounds to keep the footprint low...

#if (LOW_MEMORY == 0)
#define INDEX_OF(x) ( Index_of[ (x) ] )
#define ALPHA_TO(x) ( Alpha_to[ (x) ] )
#else
#define INDEX_OF(x) ( index_of_func( (x) ) )
#define ALPHA_TO(x) ( alpha_to_func( (x) ) )
#endif

#define SYNC_POLY      0x48
#define SCRAMBLER_POLY 0x95
#define CPOLYA         0x4f
#define CPOLYB         0x6d
#define GF_POLY        0x187
#define A0             255

#if (LOW_MEMORY == 0)
static uint8_t Index_of[256];
static uint8_t Alpha_to[256];
#endif

static uint8_t RS_poly[] = {249,59,66,4,43,126,251,97,30,3,213,50,66,170,5,24};
uint8_t RS_block[2][32];
static uint16_t Nbytes;
static uint8_t Bmask;
static uint16_t Bindex;
static uint8_t Scrambler;
static uint8_t Conv_sr;
uint8_t *Interleaver;

uint8_t index_of_func(uint8_t x) {
  uint16_t sr = 1;
  uint8_t i = 0;

  if (x == 0)
    return A0;

  while (sr != x) {
    sr <<= 1;
    if (sr & 0x100)
      sr ^= GF_POLY;
    sr &= 0xff;
    ++i;
  }

  return i;
}

uint8_t alpha_to_func(uint8_t x) {
  uint16_t sr = 1;
  uint8_t i;

  if (x == A0)
    return 0;

  for (i = 0; i < x; ++i) {
    sr <<= 1;
    if (sr & 0x100)
      sr ^= GF_POLY;
    sr &= 0xff;
  }

  return sr;
}

static inline uint8_t mod255(uint16_t x){
  while (x >= 255)
    x -= 255;
  return x;
}

static inline uint8_t parity(uint8_t x){
  x ^= x >> 4;
  x ^= x >> 2;
  x ^= x >> 1;
  return x & 1;
}

static void interleave_symbol(uint8_t c){
  if(c)
    Interleaver[Bindex] |= Bmask;
  else
    Interleaver[Bindex] &= ~Bmask;

  Bindex += 10;
  if(Bindex >= 650){
    Bindex -= 650;
    Bmask >>= 1;
    if(Bmask == 0){
      Bmask = 0x80;
      ++Bindex;
    }
  }
}

static void encode_and_interleave(uint8_t c, uint8_t cnt){
  while(cnt-- != 0){
    Conv_sr = (Conv_sr << 1) | (c >> 7);
    c <<= 1;
    interleave_symbol(parity(Conv_sr & CPOLYA));
    interleave_symbol(!parity(Conv_sr & CPOLYB)); /* Second encoder symbol is inverted */
  }    
}

static void scramble_and_encode(uint8_t c){
  uint8_t i;

  c ^= Scrambler;
  for (i = 0; i < 8; ++i)
    Scrambler = (Scrambler << 1) | parity(Scrambler & SCRAMBLER_POLY);
  encode_and_interleave(c, 8);
}

void reset_encoder(void){
  uint8_t i;
  uint8_t j;

  Nbytes = 0;
  Conv_sr = 0;
  Scrambler = 0xff;
  Bmask = 0x40;
  Bindex = 0;
  for(j=0;j<2;j++)
    for(i=0;i<32;i++)
      RS_block[j][i] = 0;
}

void init_encoder(void){
  uint8_t i;
  uint16_t sr;

#if (LOW_MEMORY == 0)
  Index_of[0] = A0;
  Alpha_to[A0] = 0;
  sr = 1;
  for (i = 0; i < 255; ++i) {
    Index_of[sr] = i;
    Alpha_to[i] = sr;
    sr <<= 1;
    if(sr & 256)
      sr ^= GF_POLY;
    sr &= 255;
  }
#endif

  for (sr = 0; sr < 650; ++sr)   // sr is used because its 16bit width
    Interleaver[sr] = 0;         // to preserve memory
  
  sr = 0x7f;
  for (i = 0; i < 65; ++i) {
    if(sr & 64) {
      Interleaver[10*i] |= 0x80;
    }
    sr = (sr << 1) | parity(sr & SYNC_POLY);
  }
  reset_encoder();
}

void encode_byte(uint8_t c){
  uint8_t *rp;
  uint8_t i;
  uint8_t j;
  uint8_t feedback;
  uint8_t t;

  rp = RS_block[Nbytes & 1];
  feedback = INDEX_OF(c ^ rp[0]);
  
  if (feedback != A0){
    for (j = 0; j < 15; ++j) {
      t = ALPHA_TO(mod255(feedback + RS_poly[j]));
      rp[j+1] ^= t;
      rp[31-j] ^= t;
    }
    rp[16] ^= ALPHA_TO(mod255(feedback + RS_poly[15]));
  }
  
  for (i = 0; i < 31; ++i)
    rp[i] = rp[i+1];

  if (feedback != A0){
    rp[31] = ALPHA_TO(feedback);
  } else {
    rp[31] = 0;
  }
  scramble_and_encode(c);
  ++Nbytes;
}  

void encode_parity(void){
  uint8_t c;

  c =  RS_block[Nbytes & 1][(Nbytes - 256) >> 1];
  scramble_and_encode(c);
  if (++Nbytes == 320) {
    encode_and_interleave(0, 6);
  }
}

/**** ENCODER CODE ENDED ****/

/*
 * Encoding data with the prevously described method
 *  - data:    exactly 256 byte sized uint8_t array
 *             It stores the data to be encoded
 *  - encoded: exactly 650 byte sized uint8_t array
 *             It holds the encoded data in byte format
 */ 

void encode_data(uint8_t *data, uint8_t *encoded) {
  uint16_t i;

  // Use already allocated array to store encoded data
  Interleaver = encoded;

  init_encoder();
  reset_encoder();

  for (i = 0; i < 256; ++i) {
    encode_byte(data[i]);
  }

  for (i = 0; i < 64; ++i) {
    encode_parity();
  }
}