
NAME=tiraes
KEY=2b7e1516:28aed2a6.abf71588:09cf4f3c
CFLAGS=-g -O0 -ansi -pedantic -std=c99 -Wall -Werror

PLAIN=test.plain
CRYPT=test.crypt
READY=test.jpg

$(NAME): debug.o

all: $(NAME)

enc: $(NAME)
	valgrind ./$(NAME) -e -k $(KEY) < $(PLAIN) > $(CRYPT) | tee debug.log
	@sha1sum -c sums

dec: $(NAME)
	valgrind ./$(NAME) -d -k $(KEY) < $(CRYPT) > $(READY) | tee debug.log
	@sha1sum -c sums

clean:
	rm $(NAME) *.o debug.log
