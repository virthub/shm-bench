CC=gcc
NAME=dap
CFLAGS=-Wall -g -c
OBJS=dap.o barrier.o
all: $(NAME)

$(NAME): ${OBJS}
	$(CC) -g -o $@ ${OBJS}

dap.o: dap.c dap.h
	$(CC) ${CFLAGS} $< -o $@

barrier.o: barrier.c barrier.h
	$(CC) ${CFLAGS} $< -o $@

clean:
	rm -f $(NAME) *.o
