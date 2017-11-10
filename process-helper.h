/*
 * Copyright Davis Cook 2017
 */

#ifndef PROCESS_HELPER_H
#define PROCESS_HELPER_H

#include <sys/types.h>

#include "pshell-structs.h"

#define PID_CANNOT_EXEC_PIPELINE -1
#define PID_CANNOT_EXEC_ASYNC_SEQUENCE -1

#define STATUS_PIPE_CREATED 0

#define EXECV_EXTRA_SIZE 2

/*
 * define functions for executing
 * pipelines and async sequences
 */
pid_t execute_pipeline(Pipeline pipeline);
pid_t execute_async_sequence(Async_sequence async_sequence);

#endif
