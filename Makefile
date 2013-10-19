
NAME=tiraes
CFLAGS=-O2 -ansi -pedantic -std=c99 -Wall -Werror

all: $(NAME)

.PHONY: clean

clean:
	rm -f $(NAME) *.o
