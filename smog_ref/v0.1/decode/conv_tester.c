#include <stdio.h>
#include <stdlib.h>
#include "fec.h"

#define MAXBYTES (320)
#define SEED (1)
/*
  Compile with: `gcc -o conv_tester conv_tester.c libfec.a`
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

  FILE* fp;
  unsigned char symb_byte[650];

  fp = fopen("enc_conv", "rb");
  fread(symb_byte, 1, sizeof(symb_byte), fp);
  fclose(fp);

  for (i = 0; i < 5200; ++i) {
    symbols[i] = (symb_byte[i / 8] & ( 1 << ( i & 0x07 ))) ? 200 : 50;
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

  fp = fopen("conv_tester_out", "wb");
  fwrite(data, 1, sizeof(data), fp);
  fclose(fp);

  return 0;
}