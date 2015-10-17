#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "uplink_enc.h"
#include "uplink_dec.h"

#define TEST_CASES (10000)

int main() {
  uint8_t in[MSG_LEN] = {0};
  uint8_t out[ENC_LEN] = {0};
  uint8_t encoded[ENC_LEN] = {0};
  uint8_t decoded[MSG_LEN] = {0};
  char test_data[] = "This is a test message for SMOG uplink with Golay(24,12) coding, interleaving and scrambling - 2015.01.17.";
  int i, j;
  int err_cnt, fatal;
  FILE *f;
  struct timespec tstart={0,0}, tend={0,0};

  for (i = 0; i < 106; ++i) {
    in[i] = test_data[i];
  }

  if ((f = fopen("debug_msg", "w"))) {
    fwrite(in, 1, MSG_LEN, f);
    fclose(f);
  }

  // Encode data into packet
  encode_data(&in, &out);

  if ((f = fopen("debug_enc", "w"))) {
    fwrite(out, 1, ENC_LEN, f);
    fclose(f);
  }

  // Make some error
  out[120] ^= 0xff;
  out[126] ^= 0xff;
  out[132] ^= 0xff;
  out[138] ^= 0xff;
  out[144] ^= 0xff;
  
  clock_gettime(CLOCK_MONOTONIC, &tstart);  
  for (i = 0; i < TEST_CASES; ++i) {
    memcpy(encoded, out, ENC_LEN);
    memset(decoded, 0, MSG_LEN);
    err_cnt = 0;
    fatal = 0;
    // Decode data to message
    decode_data(&encoded, &decoded, &err_cnt, &fatal);
  }
  clock_gettime(CLOCK_MONOTONIC, &tend);
  printf("%d iterations took %.5f seconds\n", TEST_CASES, ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));

  for (i = 0; i < MSG_LEN; ++i) {
    printf("%c", decoded[i]);
  }
  printf("\nerr: %d, fatal: %d\n", err_cnt, fatal);

  if ((f = fopen("debug_dec", "w"))) {
    fwrite(in, 1, MSG_LEN, f);
    fclose(f);
  }
  
  return 0;
}