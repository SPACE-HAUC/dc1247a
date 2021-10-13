CC = gcc
EDCFLAGS = -O2 $(CFLAGS)
EDLDFLAGS = -lpthread -lm

COBJS = i2cbus/i2cbus.o \
		main.o

all: $(COBJS)
	$(CC) $(COBJS) -o test_bias.out $(EDLDFLAGS)

%.o: %.c
	$(CC) $(EDCFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	rm -vf $(COBJS)
	rm -vf *.out