CC=gcc
NAME=eap
CFLAGS=-Wall -g -c
OBJS=eap.o vlock.o barrier.o
all: $(NAME)

$(NAME): ${OBJS}
	$(CC) -o $@ ${OBJS}

eap.o: eap.c eap.h
	$(CC) ${CFLAGS} $< -o $@
	
vlock.o: vlock.c vlock.h
	$(CC) ${CFLAGS} $< -o $@

barrier.o: barrier.c barrier.h
	$(CC) ${CFLAGS} $< -o $@
	
clean:
	rm -f $(NAME) *.o
