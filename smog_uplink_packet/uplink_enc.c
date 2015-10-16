#include <stdio.h>
#include <string.h>
#include "uplink_enc.h"

static void scrambling(uint8_t (*out)[ENC_LEN]) {
  int i;
  extern const uint8_t scrambler[ENC_LEN];

  for (i = 0; i < ENC_LEN; ++i) {
    (*out)[i] ^= scrambler[i];
  }
}


/*
 * Given the Golay (24,12) encoded data in before.
 * Arrange input data as a 48 bit (6 byte) * 48 (bit) square array.
 * Only the LSB 48 bit (6 byte) contains the golay codeword.
 * Transpose it and write to the `after` array line-by-line.
 */
static void interleave(uint32_t (*before)[WORD_COUNT], uint8_t (*after)[ENC_LEN]) {
  int i;
  int k;
  int base;
  uint32_t get_mask;
  uint8_t  set_mask;

  memset((*after), 0, ENC_LEN);

  // Select Golay codeword
  for (i = 0; i < WORD_COUNT; i += 2) {

    base = i >> 4;
    get_mask = 0x800000;
    set_mask = 1 << (7 - ((i>>1) & 0x7));
    // printf("base=%d, get_mask=%06x, set_mask=%02x\n", base, get_mask, set_mask);

    // Select a bit out of the total 24 bits MSB first and set the apropriate bit in `after`
    for (k = 0; k < 24; ++k, get_mask >>= 1) {
      if ((*before)[i] & get_mask) {
        (*after)[base + 6 * k] |=  set_mask; // & 0xff may be unnecessary
      }
    }
    
    // now the second 24 bit Golay codeword (shifted ENC_LEN/2)
    base += 144;
    get_mask = 0x800000;
    for (k = 0; k < 24; ++k, get_mask >>= 1) {
      if ((*before)[i] & get_mask) {
        (*after)[base + 6 * k] |=  set_mask;
      }
    }
    
  }
}


/*
 * Given a 12bit width data "word".
 * Compute its Golay parity bits and extend the word with them.
 */
static inline uint32_t encode_word(uint32_t word) {
  int i;
  uint32_t x = word;

  for (i = 0; i < 12; ++i) {
    if (x & 1)
      x ^= GEN_POLY;
    // without branching: cw ^= (GEN_POLY & (0x1000 - (cw & 1))) & 0xfff; 
    x >>= 1;
  }
  // save the Golay (23,11) code
  word |= x << 12;
  x = word;

  // compute the 24th parity bit, the "overall parity"
  x ^= x >> 16;
  x ^= x >> 8;
  x ^= x >> 4;
  x ^= x >> 2;
  x ^= x >> 1;

  return ((x & 1) << 23 | word);
}


/*
 * Given an `uint8_t in[144]` array as input data.
 * Compute the interleaved and FEC coded output data.
 * Put the output to the `uint8_t out[288]` array (allocated outside 
 * of this file)
 */
void encode_data(uint8_t (*in)[MSG_LEN], uint8_t (*out)[ENC_LEN]) {
  int i;
  int j = 0;
  uint32_t tmp_array[WORD_COUNT] = {0};
  uint32_t tmp_data;

  // Process data in two-pass per loop 
  // (because of golay makes 12 bit wide data, so 3 byte => 2 golay word)
  for (i = 0; i < MSG_LEN; i += 3) {
    // 2k-th golay data = { k*3-th message byte (8bit), k*3+1-th message byte upper half (4 bit) }
    tmp_data = ( (*in)[i] << 4 ) | ( ( (*in)[i+1] & 0xf0 ) >> 4 );
    tmp_array[j++] = encode_word(tmp_data);
    // 2k+1-th golay data = { k*3+1-th message byte lower half (4bit), k*3+2-th message byte (8 bit) }
    tmp_data = ( ( (*in)[i+1] & 0x0f ) << 8 ) | ( (*in)[i+2] );
    tmp_array[j++] = encode_word(tmp_data);
  }
/*
  for (i = 0; i < 10; ++i) {
    printf("i: %2d => 0x%08x\n", i, tmp_array[i]);
  }
*/
  interleave(&tmp_array, out);
  scrambling(out);
}


int main() {
  uint8_t in[MSG_LEN] = {0};
  uint8_t out[ENC_LEN] = {0};
  char test_data[] = "Teszt szoveg!";
  int i, j;

  for (i = 0; i < 13; ++i) {
    in[i] = test_data[i];
  }

  encode_data(&in, &out);

  for (i = 0; i < 48; ++i) {
    for (j = 0; j < 6; ++j) {
      printf("%02x ", out[i*6+j]);
    }
    printf("\n");
  }

}