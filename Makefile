CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2
SRCS = main.c proc_parser.c
OBJS = $(SRCS:.c=.o)

all: minips

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

minips: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

minips_sanitize: $(OBJS)
	$(CC) $(CFLAGS) -fsanitize=address,undefined -o $@ $(OBJS)

clean:
	rm -f $(OBJS) minips minips_sanitize

.PHONY: all clean
