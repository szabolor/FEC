#include <stdio.h>
#include <stdlib.h>
#include "uplink_dec.h"


/*
 * To provide enough bit-flipping XOR data with a pseudo-random stream.
 */
static void scrambling(uint8_t (*data)[ENC_LEN]) {
  int i;
  extern const uint8_t scrambler[ENC_LEN];

  for (i = 0; i < ENC_LEN; ++i) {
    (*data)[i] ^= scrambler[i];
  }
}


/*
 * Interleave data back to the array of Golay codewords.
 * Compose input array as a 48 bit * 48 bit grid, and read the valued bit by bit
 * in the columns.
 */
static void interleave(uint8_t (*before)[ENC_LEN], uint32_t (*after)[WORD_COUNT]) {
  int i;
  int k;
  int base;
  uint8_t get_mask;
  uint32_t  tmp;

  for (i = 0; i < WORD_COUNT; i += 2) {

    base = i >> 4;
    get_mask = 1 << (7 - ((i>>1) & 0x7));

    //printf("base=%d, get_mask=%02x\n", base, get_mask);
    tmp = 0;
    for (k = 0; k < 24; k++) {
      tmp <<= 1;
      if ((*before)[base + 6 * k] & get_mask) {
        tmp |= 1;
      }
    }
    (*after)[i] = tmp;

    base += 144;
    tmp = 0;
    for (k = 0; k < 24; k++) {
      tmp <<= 1;
      if ((*before)[base + 6 * k] & get_mask) {
        tmp |= 1;
      }
    }
    (*after)[i+1] = tmp;
  }
}


/*
 * Compute the Golay (23,11) syndrome to a codeword.
 * Only the LSB 23 bit will be checked.
 */
static inline uint32_t syndrome(uint32_t word) {
  int i;
  word &= LSB23BIT;
  for (i = 0; i < 12; ++i) {
    if (word & 1)
      word ^= GEN_POLY;
    word >>= 1;
  }
  return (word << 12);
}


/*
 * Return with the number of 1-bits in a 32bit word
 */
static inline int weight(uint32_t cw) {
  int c;
  for (c = 0; cw; ++c)
    cw &= cw - 1;
  return c;
}


/* 
 * Golay (23,11) code decoder. 
 * Heavily based on http://www.aqdi.com/golay.htm with some optimizations
 * Doesn't use any LUT, instead try to catch errors in a "trap-style":
 * flip some bits and rotate all around unless the syndrome matches the 
 * syndrome of the received word
 * It's a little bit computation intense: ~300 steps on average to decode
 */
static void correct_errors(uint32_t *word, int *errs) {
  int w;
  int i,j;
  uint32_t parity_bit;
  uint32_t s;
  uint32_t cw = *word;

  parity_bit = (*word) & PARITY_MASK;
  (*word) &= LSB23BIT;

  *errs = 0;
  w = 3;
  j = -1;
  while (j < 23) {
    if (j != -1) {
      cw = (*word) ^ (1 << j);
      w = 2;
    }

    s = syndrome(cw);
    if (!s) {
      *word = cw;
      goto final_check;
    }

    for (i = 0; i < 23; ++i) {
      (*errs) = weight(s);
      if ( (*errs) <= w ) {
        cw ^= s;
        cw = ((cw >> i | cw << (23 - i)) & LSB23BIT);

        if (j != -1)
          ++(*errs);

        *word = cw;
        goto final_check;
      }
      cw = ((cw << 1 | cw >> 22) & LSB23BIT);
      s = syndrome(cw);
    }
    ++j;
  }

// checking for parity errors
final_check:

  // put back the parity bit
  *word |= parity_bit;

  // compute new parity: 0 - no error, 1 - parity error
  cw = (*word);
  cw ^= cw >> 16;
  cw ^= cw >> 8;
  cw ^= cw >> 4;
  cw ^= cw >> 2;
  cw ^= cw >> 1;

  // if there's a parity error and there are 3 errors detected
  // there must be at least 4 errors => FATAL ERROR
  //  > so errs++
  // But if errs < 3 there's no point to correct the faulty parity bit,
  // just leave it alone
  (*errs) += (cw & 1);
}

/*
 * Decode Golay(24,12) encoded 144 byte long frames.
 * If an unrecoverable error happens, processing will NOT be terminated 
 * but will be will be signaled!
 * The decision behind this is the fact that the probability of the error
 * happened exlusively among the parity bits is not negligible; hope CRC
 * find no errors, so that the message survives! :D
 */
void decode_data(uint8_t (*in)[ENC_LEN], uint8_t (*out)[MSG_LEN], int *error_count, int *fatal) {
  uint32_t word_array[WORD_COUNT] = {0};
  int i;
  int j;
  int err;
  *error_count = 0;
  *fatal = 0;

  scrambling(in);
  interleave(in, &word_array);

  i = 0;
  while (i < WORD_COUNT) {
    //printf("before: %06x\n", word_array[i]);
    correct_errors(&word_array[i], &err);
    //printf("after:  %06x (err: %d)\n", word_array[i], err);
    if (err > 3)
      *fatal = 1;
    *error_count += err;
    ++i;
  }

  for (i = 0, j = 0; i < MSG_LEN; i += 3, j += 2) {
    (*out)[i] = (word_array[j] >> 4) & 0xff;
    (*out)[i+1] = ((word_array[j] & 0xf) << 4) | ((word_array[j+1] & 0xf00) >> 8);
    (*out)[i+2] = word_array[j+1] & 0xff;
  }

}

/*
// Only for quick-testing purpose
int main() {
  uint8_t in[ENC_LEN] = {0xb8, 0x48, 0x0e, 0xc0, 0x9a, 0x0d, 0x90, 0xbc, 0x8e, 0x2c, 0x93, 0xad, 0x6f, 0xb7, 0x46, 0xce, 0x5a, 0x97, 0x45, 0xcc, 0x32, 0xa2, 0xbf, 0x3e, 0x4a, 0x10, 0xf1, 0x88, 0x94, 0xcd, 0x3a, 0xb1, 0xfe, 0x90, 0x1d, 0x81, 0xac, 0x1a, 0xe1, 0x79, 0x1c, 0x59, 0x67, 0x5b, 0x4f, 0x6e, 0x8d, 0x9c, 0x5d, 0x2e, 0xfb, 0x98, 0x65, 0x45, 0xfe, 0x7c, 0x14, 0x21, 0xe3, 0x11, 0x59, 0x9b, 0xd5, 0x63, 0xfd, 0x20, 0xa3, 0x02, 0x68, 0x35, 0xc2, 0xf2, 0x38, 0xb2, 0x4e, 0xb6, 0x9e, 0xdd, 0xeb, 0x39, 0x6a, 0x5d, 0xf7, 0x30, 0xb2, 0x8a, 0xfc, 0xf8, 0x28, 0x43, 0x36, 0x22, 0x53, 0x37, 0xaa, 0xc7, 0xba, 0x40, 0x76, 0x04, 0xd0, 0x6b, 0x15, 0xe4, 0x71, 0x64, 0x9d, 0x6d, 0x4d, 0xba, 0x36, 0x72, 0xd4, 0xbb, 0xc6, 0x61, 0x95, 0x15, 0xf9, 0xf0, 0x50, 0x87, 0x8c, 0x44, 0xa6, 0x6f, 0xa5, 0x8f, 0xf4, 0x80, 0xec, 0x09, 0x50, 0xd7, 0x0b, 0xc8, 0xe2, 0xc9, 0x5a, 0xda, 0x7b, 0x74, 0x6c, 0xe5, 0xe9, 0x77, 0xdc, 0xc3, 0x2a, 0x2b, 0x83, 0xe0, 0xa1, 0x0f, 0x18, 0x89, 0x0c, 0xde, 0xab, 0x1f, 0xe9, 0x01, 0x68, 0x13, 0x41, 0xae, 0x17, 0x91, 0x65, 0x92, 0x75, 0xb4, 0xf6, 0xe8, 0x39, 0xcb, 0x52, 0xef, 0xb9, 0x86, 0x54, 0x57, 0xe7, 0xc1, 0x42, 0x1e, 0x41, 0x12, 0x99, 0xbd, 0x56, 0x3f, 0xf2, 0x03, 0xb0, 0x26, 0x83, 0x5c, 0x9f, 0x23, 0x8b, 0x24, 0xeb, 0x69, 0x2d, 0xd1, 0xb3, 0x96, 0xa5, 0xdf, 0x43, 0x0c, 0xa8, 0xaf, 0xcf, 0x82, 0xa4, 0x3c, 0x62, 0x25, 0x33, 0x7a, 0x7c, 0x7f, 0xa4, 0x07, 0x60, 0x4d, 0x26, 0xb8, 0x5e, 0x47, 0x16, 0x49, 0x46, 0xd3, 0xdb, 0xa3, 0x67, 0x2d, 0x4b, 0xbe, 0xe6, 0x19, 0x51, 0x5f, 0x2f, 0x05, 0x08, 0x78, 0xc4, 0x4a, 0x96, 0xf5, 0x58, 0xff, 0x48, 0x0e, 0x40, 0x9a, 0x0d, 0x70, 0xbc, 0x8e, 0x0c, 0x93, 0xad, 0xa7, 0xb7, 0x46, 0xfe, 0x5a, 0x97, 0x7d, 0xcc, 0x32, 0x12, 0xbf, 0x3e, 0x0a, 0x10, 0xf1, 0x38, 0x94, 0xcd, 0xea, 0xb1, 0xfe};
  uint8_t out[MSG_LEN] = {0};
  int i;
  int err_cnt, fatal;

  decode_data(&in, &out, &err_cnt, &fatal);

  for (i = 0; i < 144; ++i) {
    printf("%c", out[i]);
  }
  printf("\nCorrectable errors: %2d, Fatal errors: %2d\n", err_cnt, fatal);

}
*/