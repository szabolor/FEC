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

int test(int error_num) {
	uint8_t data[MSG_LEN] = {'H', 'e', 'l', 'l', 'o', 0};
	uint8_t encoded[ENC_LEN];
	uint8_t decoded[MSG_LEN];
	int error_count, fatal, i, bit_error = 0;

#ifdef PRINT_DATA
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
#endif

	// Flip bits in the fashion as burst errors happens
	for (i = error_num + 1; i > 0; --i) {
		encoded[(rand() % ENC_LEN)] ^= rand();
	}

	decode_data(encoded, decoded, &error_count, &fatal);
#ifdef PRINT_DATA
	printf("decoded data:\nerror_count: %d, fatal: %d\n", error_count, fatal);
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
#endif


	for (i = 0; i < MSG_LEN; ++i) {
		bit_error += BitsSetTable256[data[i] ^ decoded[i]];
	}
#ifdef PRINT_DATA
	printf("Bit error: %d\n", bit_error);
#endif

	return bit_error;
}

int main(int argc, char const *argv[]) {
	int i, itercount, error_num, init_seed = 0;
	int sum_bit_error = 0;

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
		printf("\nTesting... [%d/%d]\n", i+1, itercount);
#endif
		sum_bit_error += test(error_num);
	}

	printf("Summed bit error: %d (%f)\n", sum_bit_error,
	 ((double) sum_bit_error) / ((double) MSG_LEN * itercount * 8));

	return 0;
}

/*
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
  encode_data(in, out);

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
    decode_data(encoded, decoded, &err_cnt, &fatal);
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
}*/