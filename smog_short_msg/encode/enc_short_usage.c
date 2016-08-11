#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "enc_short_msg.h"

uint8_t data[128] = {0};
uint8_t encoded[INTERLEAVER_SIZE_BYTES] = {0};

#ifdef ENABLE_BIT_OUTPUT
uint8_t tmp;
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
  fp = fopen("encoded_bits", "wb");
  for (i = 0; i < INTERLEAVER_SIZE_BITS; ++i) {
    tmp = (encoded[i >> 3] & (1 << (7 - (i & 7)))) ? 0xD0 : 0x20;
    fwrite(&tmp, 1, sizeof(tmp), fp);
  }
  fclose(fp);
#endif

  return 0;
}
