#include <stdio.h>
#include <stdlib.h>
#include "dec_ref.h"

void test_decode_data() {
  FILE *fp;
  uint8_t raw[RAW_SIZE];
  uint8_t data[DATA_SIZE];
  int8_t error[2];

  fp = fopen("fec", "rb");
  fread(raw, sizeof(uint8_t), sizeof(raw), fp);
  fclose(fp);
 
  decode_data(raw, data, error);

  printf("Errors: RS[0] = %d, RS[1] = %d\n", error[0], error[1]);
  fp = fopen("dec_data", "wb");
  fwrite(data, sizeof(uint8_t), sizeof(data), fp);
  fclose(fp);
}

void test_decode_data_debug() {
  FILE *fp;
  uint8_t raw[RAW_SIZE];          // Raw data to be decoded
  uint8_t conv[CONV_SIZE];        // Deinterleaved raw block with SYNC removed
  uint8_t dec_data[RS_SIZE];      // Viterbi output: RS blocks interleaved and scrambled
  uint8_t rs[2][RS_BLOCK_SIZE];   // RS blocks without leading padding zeros
  uint8_t data[DATA_SIZE];        // Decoded data
  int8_t  error[2];               // RS block decoded and corrected errors (or -1 if unrecoverable error occured)

  fp = fopen("fec", "rb");
  fread(raw, sizeof(uint8_t), sizeof(raw), fp);
  fclose(fp);
 
  decode_data_debug(raw, data, error, conv, dec_data, rs);

  printf("Errors: RS[0] = %d, RS[1] = %d\n", error[0], error[1]);
  
  fp = fopen("out_data", "wb");
  fwrite(data, sizeof(uint8_t), sizeof(data), fp);
  fclose(fp);

  fp = fopen("out_conv", "wb");
  fwrite(conv, sizeof(uint8_t), sizeof(conv), fp);
  fclose(fp);

  fp = fopen("out_dec_data", "wb");
  fwrite(dec_data, sizeof(uint8_t), sizeof(dec_data), fp);
  fclose(fp);

  fp = fopen("out_rs0", "wb");
  fwrite(rs[0], sizeof(uint8_t), sizeof(rs[0]), fp);
  fclose(fp);

  fp = fopen("out_rs1", "wb");
  fwrite(rs[1], sizeof(uint8_t), sizeof(rs[1]), fp);
  fclose(fp);
}

int main() {
  // test_decode_data();
  test_decode_data_debug();

  return 0;
}