#include <stdio.h>
#include <stdlib.h>
#include "enc_ref.h"

#define SAVE_TO_FILE 0

unsigned char data[256] = {0};
unsigned char encoded[650] = {0};
unsigned char message[] = "AO40 test message with enc_ref";
int message_len = 30;

int main() {
  int i;
  unsigned char tmp;
#if (SAVE_TO_FILE > 0)
  FILE *fp;
#endif

  for (i = 0; i < message_len; ++i)
    data[i] = message[i];

  encode_data(&data, &encoded);

#if (SAVE_TO_FILE > 0)
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

  return 0;
}
