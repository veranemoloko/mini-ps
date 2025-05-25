CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2
SRCS = main.c proc_parser.c
OBJS = $(SRCS:.c=.o)

all: minips

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

minips: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f *.o minips

.PHONY: all clean