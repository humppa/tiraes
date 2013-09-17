
NAME=tiraes
KEY=2b7e1516:28aed2a6.abf71588:09cf4f3c
CFLAGS=-g -O0 -ansi -pedantic -std=c99 -Wall -Werror

$(NAME): debug.o

test: $(NAME)
	# echo a123b456c789d000 | valgrind ./$(NAME) -e -f key -v 2>&1 | tee debug.log
	echo a123b456c789d000 | valgrind ./$(NAME) -e -h $(KEY) -v 2>&1 | tee debug.log

clean:
	rm $(NAME) *.o debug.log
