#ifndef SPIRAL_VIT_V4_SINGLE_H
#define SPIRAL_VIT_V4_SINGLE_H

#include <stdint.h>

/* ==== spiral-vit_v16.h ==== */
#define K 7
#define RATE 2
#define POLYS { 79, 109 }
#define NUMSTATES 64
#define FRAMEBITS 2560
#define DECISIONTYPE uint8_t
#define DECISIONTYPE_BITSIZE 8
#define COMPUTETYPE uint8_t

//decision_t is a BIT vector
typedef union {
  DECISIONTYPE t[NUMSTATES/DECISIONTYPE_BITSIZE];
  uint32_t w[NUMSTATES/32];
  uint16_t s[NUMSTATES/16];
  uint8_t c[NUMSTATES/8];
} decision_t __attribute__ ((aligned (16)));

typedef union {
  COMPUTETYPE t[NUMSTATES];
} metric_t __attribute__ ((aligned (16)));

/* State info for instance of Viterbi decoder */
struct v {
  __attribute__ ((aligned (16))) metric_t metrics1; /* path metric buffer 1 */
  __attribute__ ((aligned (16))) metric_t metrics2; /* path metric buffer 2 */
  metric_t *old_metrics,*new_metrics; /* Pointers to path metrics, swapped on every bit */
  decision_t *decisions;   /* decisions */
};

int init_viterbi(void *p, int starting_state);
void *create_viterbi(int len);
int chainback_viterbi(void *p, uint8_t *data, uint16_t nbits, uint16_t endstate);
void delete_viterbi(void *p);
int update_viterbi_blk(void *p, COMPUTETYPE *syms,int nbits);

#endif