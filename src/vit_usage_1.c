#include <stdio.h>
#include <stdlib.h>
#include "../fec-3.0.1/fec.h"

#define MAXBYTES (10000)
#define SEED (1)
/*
  Compile with: `gcc -o vit_usage_1 vit_usage_1.c ../fec-3.0.1/libfec.a`
 */

int main() {
  int i, j;
  void *vp;   /* For Viterbi decoder TODO: why `void*`? */
  int bit;    /* Just to store a single bit TODO: why `int`?!*/
  int sr = 0; /* Shift register emulation TODO: why `int`? */
  int framebits = 2048; /* Amount of data to be encoded in bits */
  int errcnt = 0; /* For error counting */
  
  unsigned char bits[MAXBYTES]; /* Data to be encoded in byte-fromat padded with extra 6 bit zeros */
  unsigned char data[MAXBYTES]; /* Decoded data in byte-format */
  unsigned char xordata[MAXBYTES]; /* Differences between data encoded and decoded */

  /* Encoded data in bit format with reliability
   * `+6` for zero padding, `*2` for R=1/2 
   * 
   * Phil Karn's implementation uses a soft-decision Viterbi BMU (Branch Metric Unit)
   * with sensitivity of (0, 255). It means the "strongest 1" has a value of 255
   * and "strongest 0" is 0. */
  unsigned char symbols[8*2*(MAXBYTES+6)];

  srand(SEED);

/* ====== ENCODING ====== */

  printf("Convolutional Encoding of %d bit length data\n\n", framebits);
  /* Convolutional Encode Data */
  /* `+6` is for flushing the encoder to all-zero state */
  for (i = 0; i < framebits + 6; ++i) {
    /* Means, that fill random data to the framebits,
       but the 6 flushing bit must be all zero 
       Be aware that the last 6 bit would shift to the least significant side
       of the byte! (e.g. 0b00xxxxxx, but it's 0 anyway...) */
    bit = (i < framebits) ? (random() & 1) : 0;
    
    /* Shift register: load the bit */
    sr = (sr << 1) | bit;

    /* Store data in byte-format TODO: why rewrite bit[i/8] eigth time?! */
    bits[i/8] = sr & 0xff;
    
    //symbols[2*i+0] = addnoise(parity(sr & V27POLYA),gain,Gain,127.5,255);
    //symbols[2*i+1] = addnoise(parity(sr & V27POLYB),gain,Gain,127.5,255);
    symbols[2*i+0] = parity(sr & V27POLYA) ? 255 : 0;
    symbols[2*i+1] = parity(sr & V27POLYB) ? 255 : 0;
  }

  /* Be aware that the last flushing 6 bit are cropped! */
  printf("data:\n");
  for (i = 0; i < framebits / 8; ++i) {
    printf("%02x ", bits[i]);
  }
  printf("\n\n");

/* ====== CORRUPTING ====== */

  symbols[0] = 10;
  symbols[1] = 10;
  symbols[10] = 50;
  symbols[70] = 20;
  symbols[71] = 20;
  symbols[72] = 20;
  symbols[73] = 20;
  symbols[74] = 20;
  symbols[75] = 20;
  symbols[76] = 20;
  for (i = 0; i < 8; ++i) {
    symbols[i+30] = 50;
  }

/* ====== DECODING ====== */

  /* Create the Viterbi Decoder */
  if ((vp = create_viterbi27(framebits)) == NULL) {
    printf("create_viterbi27 failed\n");
    exit(1);
  }

  /* Initialize Viterbi Decoder (vp structure, starting state)*/
  i = init_viterbi27(vp, 0);
  //printf("init_viterbi27       return: %d\n", i);

  /* Decode block */
  i = update_viterbi27_blk(vp, symbols, framebits + 6);
  //printf("update_viterbi27_blk return: %d\n", i);
  
  /* Do Viterbi chainback (vp structure, output data, bit length of data, terminal state)*/
  i = chainback_viterbi27(vp, data, framebits, 0);
  //printf("chainback_viterbi27  return: %d\n\n", i);

  printf("decoded data:\n");
  for (i = 0; i < framebits / 8; ++i) {
    printf("%02x ", data[i]);
  }
  printf("\n\n");

  /* Search for decode errors */
  for (i = 0; i < framebits / 8; ++i) {
    errcnt += Bitcnt[xordata[i] = data[i] ^ bits[i]];
  }

  /* Bit diff of decoded and original */
  printf("%d bit erasured:\n", errcnt);
  for (i = 0; i < framebits / 8; ++i) {
    printf("%02x ", xordata[i]);
  }
  printf("\n");

  return 0;
}