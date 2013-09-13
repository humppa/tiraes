
NAME=tiraes
CFLAGS=-g -O0 -ansi -pedantic -std=c99 -Wall -Werror

test: $(NAME)
	echo a123b456c789d000 | valgrind ./$(NAME)
	echo a123b456c789d000 | ./$(NAME) > out.log

clean:
	rm $(NAME)
