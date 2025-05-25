CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2

OBJS = main.o proc_parser.o

minips: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f *.o minips
