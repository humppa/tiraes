
NAME=tiraes
KEY=2b7e1516:28aed2a6.abf71588:09cf4f3c
CFLAGS=-g -O0 -ansi -pedantic -std=c99 -Wall -Werror

HASHER=md5sum
VALGRIND=valgrind
PLAIN=test.plain
CRYPT=test.crypt
READY=test.ready

all: $(NAME)

test: $(NAME)
	$(VALGRIND) ./$(NAME) -e -k $(KEY) < $(PLAIN) > $(CRYPT)
	$(VALGRIND) ./$(NAME) -d -k $(KEY) < $(CRYPT) > $(READY)
	@echo
	$(HASHER) $(PLAIN) $(READY)

.PHONY: clean

clean:
	rm -f $(NAME) *.o
