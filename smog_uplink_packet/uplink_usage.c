#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "uplink_enc.h"
#include "uplink_dec.h"

// #define HEXASCII
// #define PRINT_DATA

static const uint8_t BitsSetTable256[256] = 
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

void test(int error_num, int *sum_bit_error, int *fatal_count) {
  uint8_t data[MSG_LEN] = {'H', 'e', 'l', 'l', 'o', 0};
  uint8_t encoded[ENC_LEN];
  uint8_t decoded[MSG_LEN];
  int error_count, fatal, i, bit_error = 0;

#ifdef PRINT_DATA
  printf("input data:\n");
  for (i = 0; i < MSG_LEN; ++i) {
    data[i] = rand() & 0xff;
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
#ifdef PRINT_DATA
  printf("encoded data:\n");
  for (i = 0; i < ENC_LEN; ++i) {
#ifdef HEXASCII   
    printf("%02x[%c] ", encoded[i], isprint(encoded[i]) ? encoded[i] : '.');
#else
    printf("%02x ", encoded[i]);
#endif
    if ((i & 0x7) == 0x7) {
      printf("\n");
    }
  }
  if ((i & 0x7) == 0x7) {
    printf("\n");
  }
  printf("\n");
#endif

  // Flip bits in the fashion as burst errors happens
  for (i = error_num; i > 0; --i) {
    encoded[(rand() % ENC_LEN)] ^= rand();
  }

  decode_data(encoded, decoded, &error_count, &fatal);
  *fatal_count += fatal;
#ifdef PRINT_DATA
  printf("decoded data:\n");
  for (i = 0; i < MSG_LEN; ++i) {
#ifdef HEXASCII 
    printf("%02x[%c] ", decoded[i], isprint(decoded[i]) ? decoded[i] : '.');
#else
    printf("%02x ", decoded[i]);
#endif
    if ((i & 0x7) == 0x7) {
      printf("\n");
    }
  }
  if ((i & 0x7) == 0x7) {
    printf("\n");
  }
  printf("error_count: %d, fatal: %d\n", error_count, fatal);
#endif


  for (i = 0; i < MSG_LEN; ++i) {
    bit_error += BitsSetTable256[data[i] ^ decoded[i]];
  }
  // compensate the last half byte (because of 31.5 encoded data)
  bit_error -= BitsSetTable256[(data[MSG_LEN-1] ^ decoded[MSG_LEN-1]) & 0x0f];
#ifdef PRINT_DATA
  printf("Bit error: %d\n", bit_error);
#endif

  *sum_bit_error += bit_error;
}

int main(int argc, char const *argv[]) {
  int i, itercount, error_num, init_seed = 0;
  int sum_bit_error = 0, total_fatal = 0;

  if (argc > 1) {
      itercount = atoi(argv[1]);
      printf("itercount=%d\n", itercount);
      if (argc > 2) {
        error_num = atoi(argv[2]);
        printf("error_num=%d\n", error_num);
        if (argc == 4) {
          init_seed = atoi(argv[3]);
          srand(init_seed);
          printf("init_seed=%d\n", init_seed);
        } else {
          goto usage;
        }
      }
  } else {
usage:
    printf("Usage: ./usage [itercount] [max_error_num] [init_seed]\n");
    exit(0);
  }

  for (i = 0; i < itercount; ++i) {
#ifdef PRINT_DATA
    printf("\n === Testing [%d/%d] === \n", i+1, itercount);
#endif
    test(error_num, &sum_bit_error, &total_fatal);
  }

  printf("Summed bit error: %d/%d (%f)\n", sum_bit_error, MSG_LEN * itercount * 8, ((double) sum_bit_error) / ((double) MSG_LEN * itercount * 8));
  printf("Total packet errors: %d/%d (%f)\n", total_fatal, itercount, total_fatal / (double) itercount);

  return 0;
}
