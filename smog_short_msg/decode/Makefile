CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c99
LDFLAGS = -L. -ldecshortmsg
RM = rm
AR = ar

.PHONY: all
all: libdecshortmsg usage

usage: libdecshortmsg dec_short_usage.c
	$(CC) dec_short_usage.c $(LDFLAGS) -o dec_usage

libdecshortmsg: dec_short_msg.o rs/decode_rs.o viterbi_short/spiral-vit_scalar_1280.o
	$(AR) rcs libdecshortmsg.a dec_short_msg.o rs/decode_rs.o viterbi_short/spiral-vit_scalar_1280.o

dec_short_msg.o: dec_short_msg.c
	$(CC) $(CFLAGS) -c -fPIC dec_short_msg.c -o dec_short_msg.o

rs/decode_rs.o: rs/decode_rs.c
	$(CC) $(CFLAGS) -c -fPIC rs/decode_rs.c -o rs/decode_rs.o

viterbi/spiral-vit_scalar.o: viterbi_short/spiral-vit_scalar_1280.c
	$(CC) $(CFLAGS) -c -fPIC viterbi_short/spiral-vit_scalar_1280.c -o viterbi_short/spiral-vit_scalar_1280.o

clean: clean-debug
	-$(RM) libdecshortmsg.a dec_short_msg.o rs/decode_rs.o viterbi_short/spiral-vit_scalar_1280.o
	-$(RM) usage

clean-debug:
	-$(RM) debug_*