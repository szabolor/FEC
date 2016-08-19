#if (DEBUG > 0)
#include <stdio.h>
#endif
#include <string.h>
#include "uplink_dec.h"


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


// TODO: implement `On the decoding of the (24, 12, 8) Golay code`!

/* 
 * Golay (23,11) code decoder. 
 * Heavily based on http://www.aqdi.com/golay.htm with some optimizations
 * Doesn't use any LUT, instead try to catch errors in a "trap-style":
 * flip some bits and rotate all around unless the syndrome matches the 
 * syndrome of the received word
 * It's a little bit computation intense: ~300 steps on average to decode
 */
static int correct_errors(uint32_t *word) {
  int w;
  int i,j;
  uint32_t parity_bit;
  uint32_t s;
  uint32_t cw = *word;
  int errs = 0;

  parity_bit = cw & PARITY_MASK;
  (*word) &= LSB23BIT;

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
      errs = weight(s);
      if ( errs <= w ) {
        cw ^= s;
        cw = ((cw >> i | cw << (23 - i)) & LSB23BIT);

        if (j != -1)
          ++errs;

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
  errs += (cw & 1);

  return errs;
}

/*
 * Decode Golay(24,12) encoded 144 byte long frames.
 * If an unrecoverable error happens, processing will NOT be terminated 
 * but will be will be signaled!
 * The decision behind this is the fact that the probability of the error
 * happened exlusively among the parity bits is not negligible; hope CRC
 * find no errors, so that the message survives! :D
 */
void decode_data(uint8_t *encoded, uint8_t *data, int *error_count, int *fatal) {
  int word_idx, data_idx, bit_idx; // used for indexing
  unsigned int bit_counter = 0;    // used for interleaving
  uint32_t current_word = 0;
  int errs;
  
  *error_count = 0;
  *fatal = 0;

  memset(data, 0, MSG_LEN);

  for (word_idx = 0, data_idx = 0; word_idx < WORD_COUNT; ++word_idx) {
  	current_word = 0;

  	// Deinterleave
		for (bit_idx = 0; bit_idx < 24; ++bit_idx) {
			if (encoded[bit_counter >> 3] & ( 1 << (bit_counter & 7) ) ) {
				current_word |= ( 1 << bit_idx );
			} // it's already zero, so don't handle that!

			bit_counter += INTERLEAVER_STEP_SIZE;
			if (bit_counter >= INTERLEAVER_SIZE) {
				bit_counter -= (INTERLEAVER_SIZE - 1);
			}
		}


  	// Decoding
  	errs = correct_errors(&current_word);
#if (DEBUG > 1)
  	printf("[dec] %06x\n", current_word);
#endif
  	if (errs > 3)
      *fatal = 1;
    *error_count += errs;

    // Deinterleaving 12 bits to 8 bits (bytes)

  	if (word_idx & 1) { // Odd words
  		data[data_idx++] |= (current_word >> 4) & 0xf0;
  		data[data_idx++] = current_word & 0xff;
  	} else { // Even words
  		// 0xff is not needed, just in case
  		data[data_idx]   = (current_word >> 4) & 0xff;
  		data[++data_idx] = current_word & 0x0f;
  	}
  }
}

#ifdef TEST_DECODE
int main() {
	uint8_t data[MSG_LEN];
	uint8_t encoded[ENC_LEN] = {0x0d,0x00,0x80,0x00,0x00,0x1c,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x14,0x00,0x80,0x01,0x00,0x28,0x00,0x00,0x00,0x00,0xc0,0x01,0x00,0x3c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	int i;
	int error_count, fatal;

	decode_data(encoded, data, &error_count, &fatal);

	for (i = 0; i < MSG_LEN; ++i) {
		printf("%02x ('%c')  ", data[i], data[i]);
		if ((i & 0xf) == 0xf) {
			printf("\n");
		}
	}

	if ((i & 0xf) == 0xf) {
		printf("\n");
	}

	return 0;
}
#endif // TEST_DECODE
