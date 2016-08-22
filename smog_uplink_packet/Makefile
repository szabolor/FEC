CC = gcc
ARM = arm-none-eabi-

CFLAGS = -Wall -Wextra -O2
CFLAGS_DEBUG = -Wall -Wextra -g3 -O0 -ggdb
CFLAGS_ARM_EXTRA = -mcpu=cortex-m4 -mthumb --specs=nosys.specs

LDFLAGS = -L. -luplink_dec -luplink_enc

RM = rm
AR = ar

all: libuplink_enc libuplink_dec usage

arm: libuplink_dec_arm

libuplink_dec_arm: libuplink_dec_arm.o
	$(ARM)$(AR) rcs libuplink_dec_arm.a libuplink_dec_arm.o
	arm-none-eabi-size libuplink_dec_arm.o

libuplink_dec_arm.o: uplink_dec.c
	$(ARM)$(CC) $(CFLAGS) $(CFLAGS_ARM_EXTRA) -c -fPIC uplink_dec.c -o libuplink_dec_arm.o

usage: libuplink_enc libuplink_dec uplink_usage.c
	$(CC) uplink_usage.c $(LDFLAGS) -o uplink_usage

uplink_enc.o: uplink_enc.c
	$(CC) $(CFLAGS) -c -fPIC uplink_enc.c -o uplink_enc.o

libuplink_enc: uplink_enc.o
	$(AR) rcs libuplink_enc.a uplink_enc.o

uplink_dec.o: uplink_dec.c
	$(CC) $(CFLAGS) -c -fPIC uplink_dec.c -o uplink_dec.o

libuplink_dec: uplink_dec.o
	$(AR) rcs libuplink_dec.a uplink_dec.o

clean: clean-debug
	-$(RM) *.a *.o usage

clean-debug:
	-$(RM) debug_*
