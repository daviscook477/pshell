/*
 * Copyright Davis Cook 2017
 */

/*
 * The following functions are used
 * to manipulate the creation of new
 * processes that are piped together
 */

/* allow us to use 'execvpe' */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "tokenizer.h"
#include "pshell.h"
#include "pshell-structs.h"
#include "process-helper.h"

/*
 * pull in the current environment
 *
 * this variable is defined in unistd.h
 */
extern char **environ;

/*
 * takes in a pipeline and executes all of the commands in
 * the pipeline while properly setting up pipes between
 * the stdin and stdout of each of the processes
 *
 * returns the PID of the last command in the pipeline
 */
pid_t execute_pipeline(Pipeline pipeline) {
  int (*fds)[2];
  int i, j;
  pid_t new_process_id;
  char **execv_arguments;

  /* init array to hold the file descriptor
   * arrays returned by pipe() */
  fds = malloc(sizeof(int [2]) *
    (pipeline.num_commands - 1));
  MEM_CHECK(fds);

  /* make pipeline.num_commands - 1 pipes
   * and store the file descriptors for each pipe
   * in fds
   *
   * ex for "a | b | c", it has 3 commands but only 2 pipes
   */
  /*printf("begin building %d pipes\n", pipeline.num_commands - 1);*/
  for (i = 0; i < pipeline.num_commands - 1; i++) {
    if (pipe(fds[i]) != STATUS_PIPE_CREATED) {
      fprintf(stderr, "fatal error - could not create pipe\n");
      fprintf(stderr, "pipe() failed with %d\n", errno);
      exit(EXIT_COULD_NOT_CREATE_PIPE);
    }
    /*printf("pipe read end %d write end %d\n", fds[i][0], fds[i][1]);
    fflush(stdout);*/
  }
  /*printf("end building pipes\n");*/
 
  /* fork pipeline.num_commands processes,
   * one child process for each command that
   * needs to be run */
  for (i = 0; i < pipeline.num_commands; i++) {
    /*printf("creating a new process #%d\n", i);*/
    /* create a new process to run the command */
    new_process_id = fork();
 
    if (new_process_id == 0) {
      /* loop through all the pipes and close all inputs
       * that are not the incoming pipe to this process */
      for (j = 0; j < pipeline.num_commands - 1; j++) {
        if (j != i - 1) {
          /*printf("am child process #%d and am closing read end of pipe #%d\n", i, j);*/
          close(fds[j][0]);
        }
      }

      /* loop through all the pipes and close all outputs
       * that are not the output pipe for this process */
      for (j = 0; j < pipeline.num_commands - 1; j++) {
        if (j != i) {
          /*printf("am child process #%d and am closing write end of pipe #%d\n", i, j);*/
          close(fds[j][1]);
        }
      }

      /*printf("am child process #%d with PID %d\n", i, getpid());*/

      /* copy the appropriate pipes over to stdin
       * and stdout for this child process */
      if (i != 0) {
        /*printf("am child process #%d and am copying read end of pipe #%d to my stdin\n", i, i-1);
        printf("dup2 %d %d\n", fds[i - 1][0], STDIN_FILENO);*/
        dup2(fds[i - 1][0], STDIN_FILENO);
        close(fds[i - 1][0]);
      }
      if (i != pipeline.num_commands - 1) {
        /*printf("am child process #%d and am copying write end of pipe #%d to my stdout\n", i, i);
        printf("dup2 %d %d\n", fds[i][1], STDOUT_FILENO);*/
        dup2(fds[i][1], STDOUT_FILENO);
        close(fds[i][1]);
      }

      /* set up the arguments for the command in a way
       * that execv will understand 
       *
       * this requires 2 extra strings because the
       * start of the array must be the command itself
       * and the end of the array must be a NULL pointer */
        execv_arguments = malloc(sizeof(char *) *
          (pipeline.commands[i]->num_args + EXECV_EXTRA_SIZE));
      MEM_CHECK(execv_arguments);      
      execv_arguments[0] = malloc(sizeof(char) *
        (strlen(pipeline.commands[i]->program) + NUL_TERM_SIZE));
      strcpy(execv_arguments[0], pipeline.commands[i]->program);
      /*printf("am child process #%d and pos 0 of the execv_argument list is now\
 %s\n", i, execv_arguments[0]);*/
      execv_arguments[pipeline.commands[i]->num_args+1] = NULL;
      for (j = 0; j < pipeline.commands[i]->num_args; j++) {
        execv_arguments[j+1] = malloc(sizeof(char) *
          (strlen(pipeline.commands[i]->arguments[j]) + NUL_TERM_SIZE));
        MEM_CHECK(execv_arguments[j+1]);
        strcpy(execv_arguments[j+1], pipeline.commands[i]->arguments[j]);
        /*printf("am child process #%d and pos %d of the execv_argument list is now\
 %s\n", i, j + 1, execv_arguments[j+1]);*/
      }

      /*printf("am child process #%d and am about to run\
 program %s\n", i, pipeline.commands[i]->program);*/
      /* replace the currently running program with the current
       * command (this preserves the file descriptors so the pipes
       * will properly connect everything) */
      execvpe(pipeline.commands[i]->program, execv_arguments, environ);

      /* if we get here exec failed
       *
       * we use stderr here because it will still
       * be the same as the parent (the shell)
       *
       * this lets the error appear to the user easily */
      fprintf(stderr, "non fatal error - could not run command\n");
      fprintf(stderr, "\"\" failed with error %d\n", errno);
      fprintf(stderr, "error code meanings can be found with \"man -P\
 'less -p ^ERRORS' execve\"\n");
      fprintf(stderr, "strerror() says the problem is \"%s\"\n", strerror(errno));

      /* even though execv didn't work we still need the child
       * process to die 
       *
       * NOTE: this exit() is not killing the shell, just the
       * child process that the shell spawned to do its bidding */
      exit(EXIT_COULD_NOT_EXEC);

    } else if (new_process_id > 0) {
      if (i - 1 >= 0) {
        /* close the parent's file descriptors for each of the
         * pipes because they aren't going to be used directly by
         * the shell */
        /*printf("am parent process and am closing read and write end of pipe #%d\n", i);*/
        close(fds[i - 1][0]); /* close read end of pipe */
        close(fds[i - 1][1]); /* close write end of pipe */    
      }
    } else{
      fprintf(stderr, "fatal error - could not create child process\n");
      fprintf(stderr, "fork() failed with %d\n", errno);
      exit(EXIT_COULD_NOT_FORK);
    }
  }

  /* make sure to cleanup the memory used by
   * the file descriptor array in the parent
   *
   * the children don't care because they get
   * execvpe()'ed into different programs
   */
  free(fds);

  return new_process_id;
}

/*
 * takes in an async sequence and executes all of the piplines
 * in the sequence asynchronously
 *
 * returns the PID of the last command in the last pipeline
 */
pid_t execute_async_sequence(Async_sequence async_sequence) {
  Pipeline **curr_pipeline;
  int i;
  pid_t last_command_pid = PID_CANNOT_EXEC_ASYNC_SEQUENCE;

  curr_pipeline = async_sequence.pipelines;
  for (i = 0; i < async_sequence.num_pipelines; i++) {
    /*printf("begin exec pipeline #%d\n", i);*/
    last_command_pid = execute_pipeline(**curr_pipeline);
    curr_pipeline++;
    /*printf("end exec pipeline #%d, it had PID of %d\n", i, last_command_pid);*/
  }

  return last_command_pid;
}
