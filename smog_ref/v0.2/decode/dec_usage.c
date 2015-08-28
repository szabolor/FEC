#include <stdio.h>
#include <stdlib.h>
#include "dec_ref.h"

int main() {
  FILE *fp;
  uint8_t raw[RAW_SIZE];
  uint8_t data[DATA_SIZE];
  int8_t error[2];

  fp = fopen("fec", "rb");
  fread(raw, sizeof(uint8_t), sizeof(raw), fp);
  fclose(fp);
 
  decode_data(&raw, &data, &error);

  printf("Errors: RS[0] = %d, RS[1] = %d\n", error[0], error[1]);
  fp = fopen("dec_data", "wb");
  fwrite(data, sizeof(uint8_t), sizeof(data), fp);
  fclose(fp);

  return 0;
}