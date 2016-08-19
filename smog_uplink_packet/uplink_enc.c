#if (DEBUG > 0)
#include <stdio.h>
#endif
#include <string.h>
#include "uplink_enc.h"


/*
 * Given a 12bit width data "word".
 * Compute its Golay parity bits and extend the word with them.
 */
static inline uint32_t encode_word(uint32_t word) {
  int i;
  uint32_t x;

  // just for precaution leave only the 12 data bits
  word &= 0x000fff;
  x = word;

  // Calculate syndrome
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
 * Given an `uint8_t in[31]` array as input data.
 * Compute the interleaved and FEC coded output data.
 * Put the output to the `uint8_t out[63]` array (allocated outside 
 * of this file)
 */
void encode_data(uint8_t data[MSG_LEN], uint8_t encoded[ENC_LEN]) {
  int word_idx, data_idx, bit_idx; // used for indexing
  unsigned int bit_counter = 0;    // used for interleaving
  uint32_t current_word = 0;

  memset(encoded, 0, ENC_LEN);

  // Encode every 12bit (1.5byte) into a 24bit (3byte) word
  //              0          1          2
  // data in: [aaaabbbb] [ccccdddd] [eeeeffff]
  // encoded: [aaaabbbbdddd]    [cccceeeeffff]
  //                 0                 1

  for (word_idx = 0, data_idx = 0; word_idx < WORD_COUNT; ++word_idx) {
  	current_word = 0;
  	if (word_idx & 1) { // Odd words
		// Current word := (In[data_idx] & 0xf0) << 4 | In[data_idx+1]
		current_word = (((uint32_t) data[data_idx] & 0xf0) << 4) | ((uint32_t) data[data_idx+1]);
		data_idx += 2;
  	} else { // Even words
  		// Current word := In[data_idx] << 4 | In[data_idx+1] & 0x0f
  		current_word = ((uint32_t) data[data_idx] << 4) | ((uint32_t) data[data_idx+1] & 0x0f);
  		++data_idx;
  	}

  	// Encode assembled 12 bit width data into Golay(24, 12) codeword!
		current_word = encode_word(current_word);
#if (DEBUG > 0)
		printf("[enc] %06x\n", current_word);
#endif
		// Inlined interleaving:
		// The interleaver forms a 24 row by 21 column matrix.
		// It fills in the 21 encoded data word in the columns 
		// and reads out by the rows
		// TODO: can be unrolled, but the whole encoding has a very low 
		// performance impact (0.01ms or 1ms is the same for this problem)
		for (bit_idx = 0; bit_idx < 24; ++bit_idx) {
			// interleave strating with LSB bit
			if ( current_word & ( 1 << bit_idx ) ) {
				// interleaved bit position is starting from LSB
				encoded[bit_counter >> 3] |= ( 1 << (bit_counter & 7) );
			} 
			// don't handle when the bit is zero, because 
			// the whole array is zero initialized

			bit_counter += INTERLEAVER_STEP_SIZE;
			if (bit_counter >= INTERLEAVER_SIZE) {
				bit_counter -= (INTERLEAVER_SIZE - 1);
			}
		}
  }
}

#ifdef TEST_ENCODE
int main() {
	uint8_t data[MSG_LEN] = {'H', 'e', 'l', 'l', 'o', '!', 0};
	uint8_t encoded[ENC_LEN] = {0};
	int i;

	encode_data(data, encoded);

	for (i = 0; i < ENC_LEN; ++i) {
		printf("%02x  ", encoded[i]);
		if ((i & 0xf) == 0xf) {
			printf("\n");
		}
	}

	if ((i & 0xf) == 0xf) {
		printf("\n");
	}

	return 0;
}
#endif // TEST_ENCODE
