#ifndef ENC_SHORT_REF_H
#define ENC_SHORT_REF_H

#include <stdint.h>

/*
Choose interleaver size / interleaving step size:
 - interleaver size (I) must be greater than the D bits of useful data
 - it is desired to include a pilot signal of P bits
   => I > D + P
 - interleaver ideally should be a square matrix in order to provide uniformly
   maximal distance between consecutive bits
 - interleaver step size (S) is rounded value of the side of this square 
   matrix
   => S = ceil( sqrt( D + P ) )
 - message is rounded to bytes, so depending on the interleaver size it is
   possible to have upto 8 useless bit (U) in the end of the message

In this particular case D = 2572 ( (160*8+6)*2 ), so the possible P values
while maintaing a square matrix:

S:   51 |   52
I: 2601 | 2704
P:   29 |  132
U:    7 |    0 

Or "sub-optimally": S1 * S2 = I

S1:   50 |   51 |   48 (6byte) |
S2:   52 |   52 |   54         |
I : 2600 | 2652 | 2592         |
     325 |  332 |  324         |
P :   28 |   80 |   20         |
U :    0 |    4 |    0         |

*/

#define INTERLEAVER_PILOT_BITS   80
#define INTERLEAVER_STEP_SIZE    51
#define INTERLEAVER_SIZE_BITS  2652 // 51*52 with 80 pilot bits
#define INTERLEAVER_SIZE_BYTES  ((INTERLEAVER_SIZE_BITS + 7) / 8)

#define SYNC_POLY      0x48
#define SCRAMBLER_POLY 0x95
#define CPOLYA         0x4f // 79
#define CPOLYB         0x6d // 109
#define GF_POLY        0x187
#define A0             255

//#define LOW_MEMORY          // low memory workaround to avoid LUT (and preserve 512byte)
//#define ENABLE_BIT_OUTPUT  // enable debug bit output mode
#define MSBFISRT           // the first bit of bytes of encode_data_bit is the MSB

void encode_short_data(uint8_t *data, uint8_t *encoded);

#ifdef ENABLE_BIT_OUTPUT
void encode_short_data_bit(uint8_t *data, uint8_t *bit_encoded);
#endif

#endif
