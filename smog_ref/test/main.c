#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#define DEBUG 0

extern void decode_data(uint8_t (*raw)[5200], uint8_t (*data)[256], int8_t (*error)[2]);
extern void encode_data(uint8_t (*data)[256], uint8_t (*encoded)[650]);

static volatile int run = 1;

struct {
  uint32_t cases;
  uint32_t block_error; // when error[0 or 1] is -1 or dec_data != data
  uint32_t error_count; // the sum of errors of successfully decoded messages
} stat;

struct timeval start, stop, diff;

double ebn0 = 2;

void print_usage() {
  printf("SMOG reference coding test suite\n\n");
  printf(" -c         Continuous mode testing\n");
  printf(" -e [value] Set the desired Eb/N0 value in dB\n");
  printf("\n");
  printf("Hit Ctrl+C to end the test and get the final statistics\n");
  printf("Hit Ctrl+Z to see the current results\n");
}

double normal_rand(double mean, double std_dev)
{
  double fac,rsq,v1,v2;
  static double gset;
  static int iset = 0;

  if (iset) {
    iset = 0;
    return mean + std_dev * gset;
  }

  do {
    v1 = 2.0 * (double) random() / RAND_MAX - 1;
    v2 = 2.0 * (double) random() / RAND_MAX - 1;
    rsq = v1 * v1 + v2 * v2;
  } while (rsq >= 1.0 || rsq == 0.0);

  fac = sqrt(-2.0 * log(rsq) / rsq);
  gset = v1 * fac;
  iset++;
  
  return mean + std_dev * v2 * fac;
}

uint8_t addnoise(uint8_t sym, double amp, double gain){
  double sample;
    
  sample = 127.5 + gain * normal_rand(sym ? amp : -amp, 1.0);

  if (sample < 0.)
    sample = 0.;
  else if (sample > 255.0)
    sample = 255.0;
  return (uint8_t) sample;
}

void generate_random_data(uint8_t (*data)[256]) {
  uint16_t i;

  for (i=0; i<256; ++i) {
    (*data)[i] = random() & 0xff;
  }
}

uint8_t is_equal(uint8_t (*data)[256], uint8_t (*dec_data)[256]) {
  uint16_t i;

  // TODO: make much more efficient via int64_t concation and comparation or other method
  for (i=0; i<256; ++i)
    if ((*data)[i] != (*dec_data)[i])
      return 0;
  return 1;
}

void stat_init() {
  stat.cases = 0;
  stat.block_error = 0;
  stat.error_count = 0;
}

void stat_refresh(int8_t (*error)[2], uint8_t equal) {
  ++stat.cases;
  if (equal && ((*error)[0] != -1) && ((*error)[1] != -1)) {
    stat.error_count += (*error)[0];
    stat.error_count += (*error)[1];
  } else {
    ++stat.block_error;
  }
}

void stat_print() {
  timersub(&stop, &start, &diff);

  printf("\n=== Statistics ===\n");
  printf("Number of processed blocks:                   %8d\n", stat.cases);
  printf("Block failed to decode or decoded incorretly: %8d\n", stat.block_error);
  printf("Reed-Solomon code error correction sum:       %8d\n", stat.error_count);
  printf("Eb/N0 = %.4f\n", ebn0);
  printf("BlER = %.10e\n", (double) stat.block_error / (double) stat.cases);
  printf("Time elapsed: %ld.%06ld sec\n", (long int)diff.tv_sec, (long int)diff.tv_usec);
  
}

void stat_quick_print(int signo) {
  printf("Case: %6d; Block Error: %6d; BlER: %.10e\n", stat.cases, stat.block_error, (double) stat.block_error / (double) stat.cases);
}

void stoprun(int signo) {
  run = 0;
  gettimeofday(&stop, NULL);
  printf("Stopping...\n");
  sleep(0.5);
  // because of no using locks it could happen, that there's is a half-written stat... :\
  // try to avoid this by sleeping
  stat_print();
  exit(0);
}

void continuous_test(double ebn0) {
  uint8_t data[256] = {0};
  uint8_t encoded[650] = {0};
  uint8_t bits[5200] = {0};
  uint8_t dec_data[256] = {0};
  int8_t error[2] = {0};
  FILE *fp;
  uint16_t i;

  // Es/No in dB - Energy per symbol (symbol, like channel bit)
  double esn0 = ebn0 + 10*log10(1. / 2.);

  // Compute noise voltage. The 0.5 factor accounts for BPSK seeing
  // only half the noise power, and the sqrt() converts power to
  // voltage.
  double gain = 1./sqrt(0.5/pow(10.,esn0/10.));

  gettimeofday(&start, NULL);
  while (run) {
    generate_random_data(&data);
#if (DEBUG >= 1)
    fp = fopen("data", "wb");
    fwrite(data, 256, 1, fp);
    fclose(fp);
#endif

    encode_data(&data, &encoded);

    // AWGN channel simulation
    for (i=0; i<5200; ++i) {
      bits[i] = addnoise(encoded[i>>3] & (1 << (7 - (i & 7))), gain, 32.0);
    }

    decode_data(&bits, &dec_data, &error);
#if (DEBUG >= 1)
    fp = fopen("dec_data", "wb");
    fwrite(dec_data, 256, 1, fp);
    fclose(fp);
#endif

    stat_refresh(&error, is_equal(&data, &dec_data));
  }
}

int main(int argc, char *argv[]) {
  char mode = '\0';
  int option = 0;

  srand(time(NULL));
  stat_init();

  // Ctrl+C: stop program:
  signal(SIGINT, stoprun);
  // Ctrl+Z: quick review of state
  signal(SIGTSTP, stat_quick_print);

  while ((option = getopt(argc, argv, "ce:")) != -1) {
    switch (option) {
      case 'c':
        mode = 'c';
        break;
      case 'e':
        ebn0 = atof(optarg);
        break;
      default:
        print_usage();
        exit(1);
    }
  }

  switch (mode) {
    case 'c':
      continuous_test(ebn0);
      break;
    default:
      printf("No mode have selected!\n");
      print_usage();
      exit(0);
  }

  return 0;
}