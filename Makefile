# Some variables
CC 		= gcc
CFLAGS		= -g -Wall -DDEBUG
LDFLAGS		= -lm

MODS            = proxy.o throughput_connections.o command_line.o orderedList.o log.o

VPATH           = src

OBJS		=  $(MODS)

BINS            = proxy

# Implicit .o target
.c.o:
	$(CC) -c $(CFLAGS) $<

# Explit build and testing targets

all: ${BINS}



proxy: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)



clean:
	rm -f *.o $(BINS)

