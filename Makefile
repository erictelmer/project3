# Some variables
CC 		= gcc
CFLAGS		= -g -Wall -DDEBUG
LDFLAGS		= -lm

OBJS		= starter.o throughput_connections.o command_line.o orderedList.o

BINS            = starter

# Implicit .o target
.c.o:
	$(CC) -c $(CFLAGS) $<

# Explit build and testing targets

all: ${BINS}



starter: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)



clean:
	rm -f *.o $(BINS)

