#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "enc_short_msg.h"

uint8_t data[128] = {0};
uint8_t encoded[324] = {0};

int main(int argc, char const *argv[]) {
  int i;

  FILE *fp = fopen(argv[1], "rb");
  fread(data, 1, sizeof(data), fp);
  fclose(fp);

  encode_short_data(data, encoded);

  for (i = 0; i < 324; ++i) {
  	printf("%02x ", encoded[i]);
  	if (i % 6 == 5) {
  		printf("\n");
  	}
  }
  printf("\n");

  return 0;
}
