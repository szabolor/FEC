CC = gcc
ARM = arm-none-eabi-

CFLAGS = -Wall -Wextra
CFLAGS_ARM_EXTRA = -Os -mcpu=cortex-m3 -mthumb --specs=nosys.specs

LDFLAGS = -L. -lencref

RM = rm
AR = ar


all: libencref enc_usage

arm: libencref_arm

libencref_arm: enc_ref_arm.o
	$(ARM)$(AR) rcs libencref_arm.a enc_ref_arm.o

enc_ref_arm.o: enc_ref.c
	$(ARM)$(CC) $(CFLAGS) $(CFLAGS_ARM_EXTRA) -c -fPIC enc_ref.c -o enc_ref_arm.o

enc_usage: libencref enc_usage.c
	$(CC) enc_usage.c $(LDFLAGS) -o enc_usage

libencref: enc_ref.o
	$(AR) rcs libencref.a enc_ref.o

enc_ref.o: enc_ref.c
	$(CC) $(CFLAGS) -c -fPIC enc_ref.c -o enc_ref.o

clean: clean-test clean-arm
	-$(RM) libencref.a enc_ref.o enc_usage

clean-test:
	-$(RM) test_*

clean-arm:
	-$(RM) libencref_arm.a enc_ref_arm.o
