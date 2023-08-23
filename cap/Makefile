CC=gcc
NAME=cap
CFLAGS=-Wall -g -c
OBJS=cap.o barrier.o
all: $(NAME)

$(NAME): ${OBJS}
	$(CC) -o $@ ${OBJS}

cap.o: cap.c cap.h
	$(CC) ${CFLAGS} $< -o $@

barrier.o: barrier.c barrier.h
	$(CC) ${CFLAGS} $< -o $@

clean:
	rm -f $(NAME) *.o
