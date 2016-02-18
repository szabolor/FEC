#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "enc_ref.h"

#define SAVE_TO_FILE

uint8_t data[256] = {0};
uint8_t encoded[650] = {0};
uint8_t message[] = "AO40 test message with enc_ref";
uint8_t bit_encoded[5200] = {0};
int message_len = 30;

int main() {
  int i;
  uint8_t tmp;
#ifdef SAVE_TO_FILE
  FILE *fp;
#endif

  for (i = 0; i < message_len; ++i)
    data[i] = message[i];

  encode_data(data, encoded);

#ifdef SAVE_TO_FILE
  fp = fopen("test_enc", "wb");
  fwrite(encoded, 1, sizeof(encoded), fp);
  fclose(fp);

  fp = fopen("test_fec", "wb");
  for (i = 0; i < 650 * 8; ++i) {
    tmp = (encoded[i >> 3] & (1 << (7 - (i & 7)))) ? 200 : 50;
    fwrite(&tmp, 1, sizeof(tmp), fp);
  }
  fclose(fp);
#endif

#ifdef ENABLE_BIT_OUTPUT
  encode_data_bit(&data, &bit_encoded);

  fp = fopen("test_bit", "wb");
  fwrite(bit_encoded, 1, sizeof(bit_encoded), fp);
  fclose(fp);
#endif

  return 0;
}
