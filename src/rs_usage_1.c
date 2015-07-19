#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../fec-3.0.1/fec.h"
#include "../fec-3.0.1/fixed.h"
#include "../fec-3.0.1/ccsds.h"

/*
  Compile with: `gcc -o rs_usage_1 rs_usage_1.c ../fec-3.0.1/libfec.a`
 */

int ccsds_1(int trials) {
  #define VERY_VERBOSE 0

  data_t block[NN], tblock[NN];
  int i;
  int errors;
  int errlocs[NN];
  int derrlocs[NROOTS];
  int derrors;
  int errval, errloc;
  int erasures;
  int decoder_errors = 0;

  while(trials-- > 0) {
    /* Test up to the error correction capacity of the code */
    for(errors = 0; errors <= NROOTS; ++errors) {

      /* Load block with random data and encode */
      for(i=0;i<NN-NROOTS;i++)
        block[i] = random() & NN;

      #if VERY_VERBOSE
      printf("Data:\n");
      for(i=0; i<NN; i++)
        printf("%02x ", block[i]);
      printf("\n");
      #endif

      encode_rs_8(&block[0], &block[NN-NROOTS], 0);

      /* Make temp copy, seed with errors */
      memcpy(tblock, block, sizeof(tblock));
      memset(errlocs, 0, sizeof(errlocs));
      memset(derrlocs, 0, sizeof(derrlocs));
      erasures=0;
      
      for(i = 0; i < errors; ++i) {
        do {
          errval = random() & NN;
        } while(errval == 0); /* Error value must be nonzero */

        do {
          errloc = random() % NN;
        } while(errlocs[errloc] != 0); /* Must not choose the same location twice */

        errlocs[errloc] = 1;

        if(random() & 1) /* 50-50 chance */
          derrlocs[erasures++] = errloc;

        tblock[errloc] ^= errval;
      }

      /* Decode the errored block */
      derrors = decode_rs_8(tblock, derrlocs, erasures, 0);

      #if VERY_VERBOSE
      printf("Decoded data:\n");
      for(i=0; i<NN; i++)
        printf("%02x ", tblock[i]);
      printf("\n");
      #endif

      if(derrors != errors) {
        printf("CCSDS (255, 223): decoder says %d errors, true number is %d\n", derrors, errors);
        ++decoder_errors;
      }

      for(i = 0; i < derrors; i++) {
        if(errlocs[derrlocs[i]] == 0){
          printf("CCSDS (255, 223): decoder indicates error in location %d without error\n", derrlocs[i]);
          ++decoder_errors;
        }
      }

      if(memcmp(tblock, block, sizeof(tblock)) != 0) {
        printf("CCSDS (255, 223): uncorrected errors! output ^ input:\n");
        ++decoder_errors;
        #if VERY_VERBOSE
        for(i=0; i<NN; i++)
          printf("%02x í", tblock[i] ^ block[i]);
        printf("\n");
        #endif
        break;
      }
    }
  }

  return decoder_errors;
}

int main() {
  int errs;

  printf("Testing CCSDS standard (255, 223) RS codec...");
  fflush(stdout);

  errs = ccsds_1(10);
  if(errs == 0){
    printf("\tOK\n");
  }
}