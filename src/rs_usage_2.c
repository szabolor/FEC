#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../fec-3.0.1/fec.h"
#include "../fec-3.0.1/fixed.h"
#include "../fec-3.0.1/ccsds.h"

/*
  Compile with: `gcc -o rs_usage_2 rs_usage_2.c ../fec-3.0.1/libfec.a`
 */

int main() {
  /* data_t: the base type of the data (here I use 8-bit width unsigned chars) TODO: uint8_t would be much more specific */
  /* NN = 255, the length of a single encoded packet */
  data_t block[NN];      /* Encoded data symbols (= data[223] + parity[32]) */
  data_t tblock[NN];     /* Transmitted data symbols, may contains errors */

  int derrors = 0;      /* decode error code / error count */
  int derrlocs[NROOTS]; /* Detected error location */ 
  int erasures = 0;     /* Changed data symbol count */

  int i;

  memset(block, 0, sizeof(block));
  strcpy(block, "Lorem ipsum dolor sit amet, ex quas referrentur mel");
  printf("Original:\t'%s'\n", block);

  encode_rs_8(block, block + NN - NROOTS, 0);
  
  memcpy(tblock, block, sizeof(tblock));
  
  /* Make some errors */
  tblock[3]  = 'X';
  tblock[0]  = '-';
  tblock[50] = '*';
  tblock[40] = '_';
  tblock[30] = '+';
  tblock[20] = '=';
  tblock[10] = '/';
  tblock[5]  = '~';
  tblock[15] = '%';
  printf("Transmitted:\t'%s'\n", tblock);

  /* Initialize decoder */
  memset(derrlocs, 0, sizeof(derrlocs));
  derrors = decode_rs_8(tblock, derrlocs, erasures, 0);

  printf("Recovered:\t'%s'\n", tblock);

  printf("Error locations:\t\n");
  for (i = 0; i < NROOTS; ++i) {
    printf("%d ", derrlocs[i]);
  }
  putchar('\n');

  return 0;
}