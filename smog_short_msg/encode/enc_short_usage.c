#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "enc_short_msg.h"

uint8_t data[128] = {0};
uint8_t encoded[INTERLEAVER_SIZE_BYTES] = {0};

#ifdef ENABLE_BIT_OUTPUT
uint8_t bit_encoded[INTERLEAVER_SIZE_BITS] = {0};
#endif

int main(int argc, char const *argv[]) {
  int i;
  FILE *fp;

  fp = fopen(argv[1], "rb");
  fread(data, 1, sizeof(data), fp);
  fclose(fp);

  encode_short_data(data, encoded);

  for (i = 0; i < INTERLEAVER_SIZE_BYTES; ++i) {
    if (i % 6 == 0) {
      printf(" %3d:  ", i);
    }
  	printf("%02x ", encoded[i]);
  	if (i % 6 == 5) {
  		printf("\n");
  	}
  }
  if (i % 6 != 5) {
    printf("\n");
  }

#ifdef ENABLE_BIT_OUTPUT
  encode_short_data_bit(data, bit_encoded);

  for (i = 0; i < INTERLEAVER_SIZE_BITS; ++i) {
    bit_encoded[i] = bit_encoded[i] ? 0xD0 : 0x20;
  }

  fp = fopen("encoded_bits", "wb");
  fwrite(bit_encoded, 1, INTERLEAVER_SIZE_BITS, fp);
  fclose(fp);
#endif

  return 0;
}
