CC = cc
CFLAGS = -Wall -std=c11 -g
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
		sh ./test.sh

clean:
		rm -rf 9cc *.o *~ tmp* *.out
