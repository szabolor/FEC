CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c99
LDFLAGS = -L. -ldecref
RM = rm
AR = ar

.PHONY: all
all: libdecref dec_usage

dec_usage: libdecref dec_usage.c
	$(CC) dec_usage.c $(LDFLAGS) -o dec_usage

libdecref: dec_ref.o rs/decode_rs.o viterbi/spiral-vit_scalar.o
	$(AR) rcs libdecref.a dec_ref.o rs/decode_rs.o viterbi/spiral-vit_scalar.o

dec_ref.o: dec_ref.c
	$(CC) $(CFLAGS) -c -fPIC dec_ref.c -o dec_ref.o

rs/decode_rs.o: rs/decode_rs.c
	$(CC) $(CFLAGS) -c -fPIC rs/decode_rs.c -o rs/decode_rs.o

viterbi/spiral-vit_scalar.o: viterbi/spiral-vit_scalar.c
	$(CC) $(CFLAGS) -c -fPIC viterbi/spiral-vit_scalar.c -o viterbi/spiral-vit_scalar.o

clean: clean-debug
	-$(RM) libdecref.a dec_ref.o rs/decode_rs.o viterbi/spiral-vit_scalar.o
	-$(RM) dec_usage

clean-debug:
	-$(RM) debug_*