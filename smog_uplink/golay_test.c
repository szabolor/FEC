#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include "ref_encdec.h"

#define DATA_MASK      (0xfffU)
#define LSB23BIT    (0x7fffffU)
#define BIT23       (0x400000U)

uint32_t get_next_combination(uint8_t (*state)[4], uint8_t bit_index, uint8_t bit_max_index, uint8_t step) {
  uint32_t tmp;

  if (bit_index > bit_max_index)
    return 0;

  if (!step) {
    return (get_next_combination(state, bit_index + 1, bit_max_index, 0) | (1 << (*state)[bit_index]));
  }

  if (bit_index == bit_max_index) {
    if (((*state)[bit_index] == 22)) {
      return 0;
    } else {
      ++((*state)[bit_index]);
      return (1 << (*state)[bit_index]);
    }
  } else {
    if ((*state)[bit_index+1] == ((*state)[bit_index] + 1)) {
      (*state)[bit_index] = bit_index;
      tmp = get_next_combination(state, bit_index + 1, bit_max_index, 1);
      if (tmp)
        return (tmp | (1 << (*state)[bit_index]));
      else
        return 0;
    } else {
      ++((*state)[bit_index]);
      return (get_next_combination(state, bit_index + 1, bit_max_index, 0) | 1 << (*state)[bit_index]);
    }
  }
}

// Test with all possible ONE error
void error_test(uint32_t start, uint32_t stop, uint8_t error_num) {
  uint32_t data = 0;
  uint32_t codeword = 0;
  uint32_t cw_error = 0;
  uint32_t error_mask = 0;
  uint8_t  errors = 0;
  uint8_t  parity_error = 0;
  uint8_t  error_loc[4];

  printf("Test parameters:\nFrom: 0x%03x (%d) - To: 0x%03x (%d)\nError number: %d\n", start, start, stop, stop, error_num);

  for (data = start; data <= stop; ++data) {
    codeword = golay_encode(data);

    error_loc[0] = 0;
    error_loc[1] = 1;
    error_loc[2] = 2;
    error_loc[3] = 3;
    error_mask = (1 << error_num) - 1;
    while (error_mask) {
      // Make some errors
      cw_error = codeword ^ error_mask;

      parity_error = golay_decode(&cw_error, &errors);
      assert( (cw_error & DATA_MASK) == data );
      assert( errors == error_num );
      assert( parity_error == 0 );

      // Generate the combinatorically next combination of error_num bits of 23 places
      error_mask = get_next_combination(&error_loc, 0, error_num - 1, 1);
    }
  }

  printf("PASSED!\n");
}

void print_usage() {
  printf("Golay testing\n\n");
  printf(" -f [filename]    Use file output\n");
  printf(" -e [error_num]   There's exactly [error_num] error in the codeword with all combinations\n");
  printf(" -a [start]       Start data point (default: 0)\n");
  printf(" -b [stop]        End data point (default: 0xfff)\n");
}

int main(int argc, char *argv[]) {
  uint32_t start_cw = 0x000;
  uint32_t stop_cw = 0xfff;
  char *filename = NULL;
  char test = '\0';
  int option = 0;
  uint8_t error_num = 3;


  while ((option = getopt(argc, argv, "f:ca:b:e:")) != -1) {
    switch (option) {
      case 'f':
        strcpy(filename, optarg);
        break;
      case 'e':
        test = 'e';
        error_num = atoi(optarg);
        break;
      case 'a':
        start_cw = atoi(optarg);
        break;
      case 'b':
        stop_cw = atoi(optarg);
        break;
      default: print_usage();
        exit(1);
    }
  }

  switch (test) {
    case 'e':
      error_test(start_cw, stop_cw, error_num);
      break;
    default:
      printf("No test mode have selected!\n");
      exit(1);
  }

  return 0;
}
