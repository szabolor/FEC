#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Cyclic code: generator polynomial 0xC75 (or 0xAE3 bitwise reverse)
#define GEN_POLY       (0xAE3U)
#define PARITY_MASK (0x800000U)
#define DATA_MASK      (0xfffU)
#define LSB23BIT    (0x7fffffU)


/* This function calculates and returns the syndrome
   of a [23,12] Golay codeword. */
static uint32_t syndrome(uint32_t cw) {
  uint8_t i;

  cw &= LSB23BIT;
  for (i = 0; i < 12; ++i) { /* examine each data bit */
    if (cw & 1)           /* test data bit */
      cw ^= GEN_POLY;     /* XOR polynomial */
    cw >>= 1;             /* shift intermediate result */
  }

  return (cw << 12);      /* value pairs with upper bits of cw */
}

uint32_t golay_encode(uint32_t cw) {
  cw &= DATA_MASK; // maybe unnecessary, data side validation also made (* to be make)

  return ( syndrome(cw) | cw );    /* assemble codeword */
}

// TODO: try with simple `x ^= x >> 2**n` for size optimization
static inline uint8_t parity(uint32_t x) {
  x ^= x >> 1;
  x ^= x >> 2;
  x = (x & 0x11111111U) * 0x11111111U;
  return (x >> 28) & 1;
}

static inline uint8_t weight(uint32_t cw) {
  uint8_t c; // c accumulates the total bits set in v

  for (c = 0; cw; c++) {
    cw &= cw - 1; // clear the least significant bit set
  }

  return c;
}

static uint32_t correct(uint32_t cw, uint8_t *errs) {
  uint8_t    w;                /* current syndrome limit weight, 2 or 3 */
  uint32_t   mask;             /* mask for bit flipping */
  int8_t     i,j;              /* index */
  uint32_t   s;                /* calculated syndrome */
  uint32_t   cwsaver;          /* saves initial value of cw */

  cwsaver = cw;         /* save */
  *errs = 0;
  w = 3;                /* initial syndrome weight threshold */
  j = -1;               /* -1 = no trial bit flipping on first pass */
  mask = 1;
  while (j < 23) { /* flip each trial bit */
    if (j != -1) { /* toggle a trial bit */
      cw = cwsaver ^ mask; /* flip next trial bit */
      mask <<= 1;
      w = 2; /* lower the threshold while bit diddling */
    }

    s = syndrome(cw); /* look for errors */
    if (s) { /* errors exist */
      for (i = 0; i < 23; ++i) { /* check syndrome of each cyclic shift */
        if ( ( (*errs) = weight(s) ) <= w ) { /* syndrome matches error pattern */
          cw ^= s;              /* remove errors */
          cw = ((cw >> i | cw << (23 - i)) & LSB23BIT);  /* unrotate (right) data */

          if (j >= 0) /* count toggled bit (per Steve Duncan) */
            ++(*errs);

          return cw;
        } else {
          cw = ((cw << 1 | cw >> 22) & LSB23BIT);   /* rotate left to next pattern */
          s = syndrome(cw);         /* calc new syndrome */
        }
      }
      ++j; /* toggle next trial bit */
    } else {
      return cw; /* return corrected codeword */
    }
  }

  return cwsaver; /* return original if no corrections */
}

uint8_t golay_decode(uint32_t *cw, uint8_t *errs) {
  uint8_t parity_bit;

  parity_bit = (*cw) & PARITY_MASK ? 1 : 0;
  (*cw) &= ~PARITY_MASK;
  (*cw) = correct(*cw, errs);
  if (parity_bit)
    (*cw) |= PARITY_MASK;

  // 4 bit error checking: 0 - no error; 1 - 4 bit error
  return parity(*cw);
}

/*
int main () {

  uint32_t data = 0, codeword = 0;
  uint8_t errors = 0;

  data = 0x123;
  printf("data: %06x\n", data);

  codeword = golay_encode(data);
  if (parity(codeword))
    codeword |= PARITY_MASK;

  printf("code: %06x\n", codeword);

  codeword &= ~(1 << 0);
  codeword |= (1 << 1);
  //codeword &= ~(1 << 6);

  //codeword |= (1 << 20);
  codeword |= (1 << 21);
  //codeword |= (1 << 22);
  printf("code with error: %06x\n", codeword);

  if (golay_decode(&codeword, &errors))
    printf("There was a parity error\n");
  else
    printf("Decoded without parity error\n");

  printf("decoded: %06x with errors: %d\n", codeword, errors);

  return 0;
}
*/
