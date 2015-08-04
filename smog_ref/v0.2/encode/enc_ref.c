/* 
 * Reference encoder for proposed coded AO-40 telemetry format - v1.0  7 Jan 2002
 * Copyright 2002, Phil Karn, KA9Q
 * This software may be used under the terms of the GNU Public License (GPL)
 */

#define SYNC_POLY      0x48
#define SCRAMBLER_POLY 0x95
#define CPOLYA         0x4f
#define CPOLYB         0x6d
#define GF_POLY        0x187
#define A0             255

static unsigned char Index_of[256];
static unsigned char Alpha_to[256];
static unsigned char RS_poly[] = {249, 59, 66,  4, 43,126,251, 97, 30,  3,213, 50, 66,170,  5, 24};
unsigned char RS_block[2][32];
static int Nbytes;
static unsigned char Bmask;
static int Bindex;
static unsigned char Scrambler;
static unsigned char Conv_sr;
unsigned char *Interleaver;

static inline int mod255(int x){
  while (x >= 255)
    x -= 255;
  return x;
}

static inline int parity(int x){
  x ^= x >> 4;
  x ^= x >> 2;
  x ^= x >> 1;
  return x & 1;
}

static void interleave_symbol(int c){
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
      Bindex++;
    }
  }
}

static void encode_and_interleave(unsigned char c,int cnt){
  while(cnt-- != 0){
    Conv_sr = (Conv_sr << 1) | (c >> 7);
    c <<= 1;
    interleave_symbol(parity(Conv_sr & CPOLYA));
    interleave_symbol(!parity(Conv_sr & CPOLYB)); /* Second encoder symbol is inverted */
  }    
}

static void scramble_and_encode(unsigned char c){
  int i;

  c ^= Scrambler;
  for(i=0;i<8;i++)
    Scrambler = (Scrambler << 1) | parity(Scrambler & SCRAMBLER_POLY);
  encode_and_interleave(c,8);
}

void reset_encoder(void){
  int i,j;

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
  int i,sr;
  Index_of[0] = A0;
  Alpha_to[A0] = 0;
  sr = 1;
  for(i=0;i<255;i++){
    Index_of[sr] = i;
    Alpha_to[i] = sr;
    sr <<= 1;
    if(sr & 256)
      sr ^= GF_POLY;
    sr &= 255;
  }
  for(i=0;i<650;i++)
    Interleaver[i] = 0;
  sr = 0x7f;
  for(i=0;i<65;i++){
    if(sr & 64) {
      Interleaver[10*i] |= 0x80; /* Every 80th bit is a sync bit */
    }
    sr = (sr << 1) | parity(sr & SYNC_POLY);
  }
  reset_encoder();
}

void encode_byte(unsigned char c){
  unsigned char *rp;
  int i;
  int j;
  unsigned char feedback;
  unsigned char t;

  rp = RS_block[Nbytes & 1];
  feedback = Index_of[c ^ rp[0]];
  if(feedback != A0){
    for(j=0;j<15;j++){
      t = Alpha_to[mod255(feedback + RS_poly[j])];
      rp[j+1] ^= t;
      rp[31-j] ^= t;
    }
    rp[16] ^= Alpha_to[mod255(feedback + RS_poly[15])];
  }
  for(i=0;i<31;i++)
    rp[i] = rp[i+1];
  if(feedback != A0){
    rp[31] = Alpha_to[feedback];
  } else {
    rp[31] = 0;
  }
  scramble_and_encode(c);
  Nbytes++;
}  

void encode_parity(void){
  unsigned char c;

  c =  RS_block[Nbytes & 1][(Nbytes-256)>>1];
  scramble_and_encode(c);
  if(++Nbytes == 320){
    encode_and_interleave(0,6);
  }
}

/**** ENCODER CODE ENDED ****/

/*
 * Encoding data with the prevously described method
 *  - data:    exactly 256 byte sized unsigned char array
 *             It stores the data to be encoded
 *  - encoded: exactly 650 byte sized unsigned char array
 *             It holds the encoded data in byte format
 */ 

void encode_data(unsigned char *data, unsigned char *encoded) {
  int i;

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