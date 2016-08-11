#include <stdio.h>
#include <stdlib.h>
#include "dec_short_msg.h"

void test_decode_data(char const *filename) {
  FILE *fp;
  uint8_t raw[RAW_SIZE];
  uint8_t data[DATA_SIZE];
  int8_t  error;

  fp = fopen(filename, "rb");
  fread(raw, 1, sizeof(raw), fp);
  fclose(fp);
 
  decode_data(raw, data, &error);

  printf("Errors: RS = %d\n", error);
  fp = fopen("dec_data", "wb");
  fwrite(data, 1, sizeof(data), fp);
  fclose(fp);
}

void test_decode_data_debug(char const *filename) {
  FILE *fp;
  uint8_t raw[RAW_SIZE] = {0};          // Raw data to be decoded
  uint8_t conv[CONV_SIZE] = {0};        // Deinterleaved raw block with SYNC removed
  uint8_t dec_data[RS_SIZE] = {0};      // Viterbi output: RS blocks interleaved and scrambled
  uint8_t rs[RS_BLOCK_SIZE] = {0};      // RS blocks without leading padding zeros
  uint8_t data[DATA_SIZE] = {0};        // Decoded data
  int8_t  error = 0;                    // RS block decoded and corrected errors (or -1 if unrecoverable error occured)

  int i;

  fp = fopen(filename, "rb");
  fread(raw, 1, RAW_SIZE, fp);
  fclose(fp);

  decode_data_debug(raw, data, &error, conv, dec_data, rs);

  printf("Errors: RS = %d\n", error);
  
  fp = fopen("out/data", "wb");
  fwrite(data, 1, DATA_SIZE, fp);
  fclose(fp);

  fp = fopen("out/conv", "wb");
  fwrite(conv, 1, CONV_SIZE, fp);
  fclose(fp);

  fp = fopen("out/dec_data", "wb");
  fwrite(dec_data, 1, RS_SIZE, fp);
  fclose(fp);

  fp = fopen("out/rs", "wb");
  fwrite(rs, 1, RS_BLOCK_SIZE, fp);
  fclose(fp);
}

int main(int argc, char const *argv[]) {
  // test_decode_data(argv[1]);
  test_decode_data_debug(argv[1]);

  return 0;
}