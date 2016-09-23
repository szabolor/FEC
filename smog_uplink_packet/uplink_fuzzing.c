#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "uplink_enc.h"
#include "uplink_dec.h"


int main(int argc, char const *argv[]) {
  uint8_t data[MSG_LEN];
  uint8_t encoded[ENC_LEN];
  uint8_t decoded[MSG_LEN];
  int error_count, fatal, i, bit_error = 0;

  FILE *fp;

  if (argc == 1) {
    printf("Usage: ./usage_fuzzing input_file\n");
    return EXIT_SUCCESS;
  }

  fp = fopen(argv[1], "rb");
  if (fp == NULL) {
    perror("Failed to open file");
    return EXIT_FAILURE;
  }

  if (fread(data, sizeof *data, MSG_LEN, fp) != MSG_LEN) {
    if (feof(fp))
      printf("Error reading file: unexpected end of file\n");
    else if (ferror(fp)) {
      perror("Error reading file");
    }
  }

  // last (LSB) nibble remains only
  data[MSG_LEN-1] &= 0x0f;

#ifdef PRINT
  printf("Input: \n");
  for (i = 0; i < MSG_LEN; ++i) {
    printf("%02x ", data[i]);
    if ((i & 0x7) == 0x7) {
      printf("\n");
    }
  }
  if ((i & 0x7) == 0x7) {
    printf("\n");
  }
  printf("\n");
#endif

  encode_data(data, encoded);
  decode_data(encoded, decoded, &error_count, &fatal);

#ifdef PRINT
  printf("Output: \n");
  for (i = 0; i < MSG_LEN; ++i) {
    printf("%02x ", decoded[i]);
    if ((i & 0x7) == 0x7) {
      printf("\n");
    }
  }
  if ((i & 0x7) == 0x7) {
    printf("\n");
  }
  printf("\n");
#endif

  assert(fatal == 0);
  assert(memcmp(data, decoded, MSG_LEN) == 0);

  return 0;
}
