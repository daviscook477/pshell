/*
 * Copyright Davis Cook 2017
 */

#ifndef PSHELL_H
#define PSHELL_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * the program has the following exit states
 * EXIT_SUCCESS comes predefined in <stdlib.h>
 *
 * EXIT_SUCCESS = 0 -> successful termination of the program
 * EXIT_COULD_NOT_CREATE_PIPE = 1 -> system call to pipe() failed
 * EXIT_COULD_NOT_ALLOC_MEMORY = 2 -> call to malloc() failed
 * EXIT_COULD_NOT_FORK = 3 -> system call to fork() failed
 * EXIT_COULD_NOT_EXEC = 4 -> system call to exec() failed
 *
 * NOTE: EXIT_COULD_NOT_EXEC will only be returned by child
 * processes
 */

#define EXIT_COULD_NOT_CREATE_PIPE 1
#define EXIT_COULD_NOT_ALLOC_MEMORY 2
#define EXIT_COULD_NOT_FORK 3
#define EXIT_COULD_NOT_EXEC 4

/*
 * macro for checking if a memory allocation
 * failed and killing the program if it does
 */

#define MEM_CHECK(ptr) if ((ptr) == NULL) {\
fprintf(stderr, "fatal error - could not allocate memory\n");\
fprintf(stderr, "malloc() failed with %d\n", errno);\
exit(EXIT_COULD_NOT_ALLOC_MEMORY);\
}

#endif
