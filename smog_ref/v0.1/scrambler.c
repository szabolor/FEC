#include <stdio.h>
#include <stdlib.h>

#define SCRAMBLER_POLY 0x95

static inline int parity(int x){
  x ^= x >> 4;
  x ^= x >> 2;
  x ^= x >> 1;
  return x & 1;
}
  
int main() {
  int i, j;
  unsigned char Scrambler = 0xff;
  
  for (i = 0; i < 320; ++i) {
    printf("%02x ", Scrambler);
    if ((i & 0x0f) == 0x0f)
      printf("\n");
    for(j=0;j<8;++j)
     Scrambler = (Scrambler << 1) | parity(Scrambler & SCRAMBLER_POLY); 
  }
  
  return 0;
}