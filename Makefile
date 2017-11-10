# set our compiler and compiler flags to start
CC = gcc
CFLAGS = -ansi -pedantic-errors -Wall -Werror -Wshadow

all: pshell.x tokenizer_test01.x process-helper_test01.x

clean:
	rm -f *.x
	rm -f *.o

tokenizer.o: tokenizer.c tokenizer.h
	${CC} ${CFLAGS} -c tokenizer.c

splitter.o: splitter.c splitter.h tokenizer.h pshell.h
	${CC} ${CFLAGS} -c splitter.c

process-helper.o: process-helper.c process-helper.h pshell.h pshell-structs.h tokenizer.h splitter.h
	${CC} ${CFLAGS} -c process-helper.c

pshell.o: pshell.c pshell.h pshell-structs.h tokenizer.h splitter.h process-helper.h
	${CC} ${CFLAGS} -c pshell.c

pshell.x: pshell.o tokenizer.o splitter.o process-helper.o
	${CC} pshell.o tokenizer.o splitter.o process-helper.o -o pshell.x

tokenizer_test01.x: tokenizer.c tokenizer.h tokenizer_test01.c
	${CC} tokenizer.c tokenizer_test01.c -o tokenizer_test01.x

process-helper_test01.x: process-helper.h process-helper.c pshell-structs.h process-helper_test01.c
	${CC} process-helper.c process-helper_test01.c -o process-helper_test01.x
