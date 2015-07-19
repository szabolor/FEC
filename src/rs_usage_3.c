#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../fec-3.0.1/fec.h"
#include "../fec-3.0.1/fixed.h"
#include "../fec-3.0.1/ccsds.h"

/*
  CCSDS standard Reed-Solomon coding with padding 

  Padding: 95 symbol zeros at the beginning of the data
           must not transmit, mutually agreed values
 
  Compile with: `gcc -o rs_usage_3 rs_usage_3.c ../fec-3.0.1/libfec.a`
 */

int main() {
  /* data_t: the base type of the data (here I use 8-bit width unsigned chars) TODO: uint8_t would be much more specific */
  /* NN = 255, the length of a single encoded packet */
  data_t block[NN];       /* Encoded data symbols (= data[223] + parity[32]) */
  data_t tblock[NN - 95]; /* Transmitted data symbols, may contains errors */
  data_t padded_received[NN];

  int derrors = 0;      /* decode error code / error count */
  int derrlocs[NROOTS]; /* Detected error location */ 
  int erasures = 0;     /* Changed data symbol count */

  int i;

  memset(block, 0, sizeof(block));
  strcpy(block + 95, "Lorem ipsum dolor sit amet, ex quas referrentur mel");
  printf("Original:\t'%s'\n", block + 95);
  printf(" - Size:\t%d\n", sizeof(block));

  encode_rs_8(block, block + NN - NROOTS, 0);
  
  memcpy(tblock, block + 95, sizeof(tblock));
  
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
  printf(" - Size:\t%d\n", sizeof(tblock));

  /* Initialize decoder */
  memset(padded_received, 0, sizeof(padded_received));
  memcpy(padded_received + 95, tblock, sizeof(tblock));
  memset(derrlocs, 0, sizeof(derrlocs));
  derrors = decode_rs_8(padded_received, derrlocs, erasures, 0);

  printf("Recovered:\t'%s'\n", padded_received + 95);

  printf("Error locs.:\t");
  for (i = 0; i < NROOTS; ++i) {
    printf("%d ", derrlocs[i]);
  }
  putchar('\n');

  return 0;
}