
CFLAGS=-g -Wall -std=c99 -pedantic
EXEC=prueba_hash
CC=gcc
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)
LDFLAGS=

ifneq (,$(shell grep -lm 1 \'^\s*\#.*include.*\<math\.h\>\' *.h *.c ))
	LDFLAGS+=-lm
endif

all: clean $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(EXEC)

clean:
	rm -f *.o $(EXEC)

