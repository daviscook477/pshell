/*
 * Copyright Davis Cook 2017
 */

/*
 * The following program is an interactive shell
 * hereby called pshell (pronounced P-shell)
 * that implements a basic shell that can
 * execute programs and pipe them together
 * in many interesting ways
 */

/* allow us to use 'execvpe' */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#include "pshell.h"
#include "pshell-structs.h"
#include "tokenizer.h"
#include "splitter.h"
#include "process-helper.h"

#define MAX_LINE_SIZE 300

#define SYNC_DELIMITER ";"
#define ASYNC_DELIMITER "&"
#define PIPE_DELIMITER "|"

/* 
 * pull in the current environment
 *
 * this variable is defined in unistd.h
 */
extern char **environ;

/*
 * define prototypes
 */
static Async_sequence **parse_synchronous_command_sequence(Token_list token_list);
static void cleanup_sync_sequence(Async_sequence **sync_sequence);

/*
 * takes in a token list and then parses the list into a synchronous sequence
 * of async sequences of pipelines of commands that the shell will have to
 * execute
 *
 * the function return type "Async_sequence *" means an array of
 * Async_sequence terminated with a NULL pointer which is the same
 * as a sequence of synchronous commands
 *
 * the grammar parsing for this shell DEFINITELY should be done in a more
 * robust way -> but that wasn't really the goal of this project so this
 * fairly rigid and in-extensible system is all we've got
 */
static Async_sequence **parse_synchronous_command_sequence(Token_list token_list) {
  Async_sequence **sync_sequence;
  Token_list_list split_by_sync_delim, split_by_async_delim, split_by_pipe_delim;
  int count_sync_sections, count_async_sections, count_pipe_sections, count_args;
  int i, j, k, l;
  Async_sequence *async_sequence;
  Pipeline *pipeline;
  Command *command;
  Token_list *token_list_sync, *token_list_async, *token_list_pipeline;
  Token curr_token;

  /* TODO This function needs to be able to handle cases where people
   * do this wrong in their input without blowing up */

  /*printf("begin splitting on %s\n", SYNC_DELIMITER);
  fflush(stdout);*/
  split_by_sync_delim = split(token_list, SYNC_DELIMITER);
  /*printf("end splittong on %s\n", SYNC_DELIMITER);
  fflush(stdout);*/
  count_sync_sections = count_token_list_list_size(&split_by_sync_delim);
  /*printf("num sync sections %d\n", count_sync_sections);
  fflush(stdout);*/
  sync_sequence = malloc(sizeof(Async_sequence *) * 
    (count_sync_sections + 1));
  MEM_CHECK(sync_sequence);

  /* pad the end of the sync sections array
   * with a NULL pointer to show where it ends */
  sync_sequence[count_sync_sections] = NULL;
  for (i = 0; i < count_sync_sections; i++) {
    token_list_sync = next_token_list(&split_by_sync_delim);
    async_sequence = malloc(sizeof(Async_sequence));
    MEM_CHECK(async_sequence);
    sync_sequence[i] = async_sequence;

    /*printf("begin splitting on %s, loop #%d\n", ASYNC_DELIMITER, i);
    fflush(stdout);*/
    split_by_async_delim = split(*token_list_sync, ASYNC_DELIMITER);
    /*printf("end splitting on %s\n", ASYNC_DELIMITER);
    fflush(stdout);*/
    count_async_sections = count_token_list_list_size(&split_by_async_delim);
    /*printf("num async sections %d\n", count_async_sections);
    fflush(stdout);*/
    async_sequence->num_pipelines = count_async_sections;
    async_sequence->pipelines = malloc(sizeof(Pipeline *) * count_async_sections);
    MEM_CHECK(async_sequence->pipelines);
    for (j = 0; j < count_async_sections; j++) {
      token_list_async = next_token_list(&split_by_async_delim);
      pipeline = malloc(sizeof(Pipeline));
      MEM_CHECK(pipeline);
      async_sequence->pipelines[j] = pipeline;

      /*printf("begin splitting on %s, loop #%d\n", PIPE_DELIMITER, j);
      fflush(stdout);*/
      split_by_pipe_delim = split(*token_list_async, PIPE_DELIMITER);
      /*printf("end splitting on %s\n", PIPE_DELIMITER);
      fflush(stdout);*/
      count_pipe_sections = count_token_list_list_size(&split_by_pipe_delim);
      /*printf("num pipe sections %d\n", count_pipe_sections);      
      fflush(stdout);*/
      pipeline->num_commands = count_pipe_sections;
      pipeline->commands = malloc(sizeof(Command *) * count_pipe_sections);
      MEM_CHECK(pipeline->commands);
      for (k = 0; k < count_pipe_sections; k++) {
        token_list_pipeline = next_token_list(&split_by_pipe_delim);
        command = malloc(sizeof(Command));
        MEM_CHECK(command);
        pipeline->commands[k] = command;

        count_args = count_token_list_size(token_list_pipeline);
        /*printf("got passed %d args on loop #%d\n", count_args, k);
        fflush(stdout);
        printf("head %s iter %s\n", token_list_pipeline->head->data, token_list_pipeline->iterator->data);
        fflush(stdout);*/
        curr_token = next_token(token_list_pipeline);
        command->program = malloc(sizeof(char) *
          (strlen(curr_token.data) + NUL_TERM_SIZE));
        /*printf("copying %s into command on loop #%d\n", curr_token.data, k);
        fflush(stdout);*/
        strcpy(command->program, curr_token.data);
        cleanup_token(&curr_token);
        command->num_args = count_args - 1;
        command->arguments = malloc(sizeof(char *) *
          (count_args - 1));
        MEM_CHECK(command->arguments);
        for (l = 1; l < count_args; l++) {
          curr_token = next_token(token_list_pipeline);
          command->arguments[l - 1] = malloc(sizeof(char) *
            (strlen(curr_token.data) + NUL_TERM_SIZE));
          MEM_CHECK(command->arguments[l - 1]);
          /*printf("copying %s into args position %d on loop #%d\n", curr_token.data, l - 1, l);
          fflush(stdout);*/
          strcpy(command->arguments[l - 1], curr_token.data);
          /*printf("successful copy!\n");
          fflush(stdout);*/
          cleanup_token(&curr_token);
        }
      }
      cleanup_token_list_list(&split_by_pipe_delim);
    }
    cleanup_token_list_list(&split_by_async_delim);
  }
  cleanup_token_list_list(&split_by_sync_delim);

  return sync_sequence;
}

/*
 * cleans up all the dynamically allocated data for
 * a synchronous command sequence
 */
static void cleanup_sync_sequence(Async_sequence **sync_sequence) {
  Async_sequence **curr_async_sequence, **tmp_async_sequence;
  Pipeline **curr_pipeline, **tmp_pipeline;
  Command **curr_command, **tmp_command;
  char **curr_arg, **tmp_arg;
  int i, j, k;

  /* loop through all async sequences */
  curr_async_sequence = sync_sequence;
  while (*curr_async_sequence != NULL) {
    /* loop through all pipelines */
    curr_pipeline = (*curr_async_sequence)->pipelines;
    for (i = 0; i < (*curr_async_sequence)->num_pipelines; i++) {    
      /* loop through all commands */
      curr_command = (*curr_pipeline)->commands;
      for (j = 0; j < (*curr_pipeline)->num_commands; j++) {
        /* loop through all args */
        curr_arg = (*curr_command)->arguments;
        for (k = 0; k < (*curr_command)->num_args; k++) {
          tmp_arg = curr_arg + 1;
          /*printf("attempting to free %s from current command=%s\n", *curr_arg, (*curr_command)->program);
          fflush(stdout);*/
          free(*curr_arg);
          curr_arg = tmp_arg;
        }

        /* free the command itself */
        tmp_command = curr_command + 1;
        free((*curr_command)->program);
        free((*curr_command)->arguments);
        free(*curr_command);
        curr_command = tmp_command;
      }

      /* free the pipeline itself */
      tmp_pipeline = curr_pipeline + 1;
      free((*curr_pipeline)->commands);
      free(*curr_pipeline);
      curr_pipeline = tmp_pipeline;
    }

    /* free the async sequence itself */
    tmp_async_sequence = curr_async_sequence + 1;
    free((*curr_async_sequence)->pipelines);
    free(*curr_async_sequence);
    curr_async_sequence = tmp_async_sequence;
  }
  free(sync_sequence);
}

int main() {
  char line[MAX_LINE_SIZE];
  Token_list token_list;
  Async_sequence **sync_sequence;
  Async_sequence **curr_async_sequence;
  pid_t async_pid;
  int status;
  int async_sequence_num;

  /*
   * read, parse, execute loop
   * will only break if the exit
   * command is run in the shell
   * in which case this program
   * will quite with a call to exit()
   */
  while (1) {

    /* read from stdin */
    fgets(line, MAX_LINE_SIZE, stdin);

    /*printf("begin token parse\n");    
    fflush(stdout);*/
    /* parse the line into tokens */
    token_list = parse_tokens(line);
    /*printf("end token parse\n");
    fflush(stdout);

    printf("begin sequence parse\n");    
    fflush(stdout);*/
    /* convert the tokens into a synchronous
     * command sequence */
    sync_sequence = parse_synchronous_command_sequence(token_list);
    /*printf("end sequence parse\n");
    fflush(stdout);*/

    /* execute the commands being given */
    curr_async_sequence = sync_sequence;
    async_sequence_num = 0;
    while (*curr_async_sequence != NULL) {
      /* execute all the commands in the async sequence simultaneously
       * and wait for the last command in the async sequence to
       * complete before moving on to the next async sequence in
       * this synchronous sequence
       *
       * async_pid is the PID of the last process started in
       * the async_sequence which will be the procecss that we
       * must wait for completion
       * */
      /*printf("begin exec async sequence #%d\n", async_sequence_num);
      fflush(stdout);*/
      async_pid = execute_async_sequence(**curr_async_sequence);
      /*printf("end exec async sequence #%d\n", async_sequence_num);
      fflush(stdout);*/
      waitpid(async_pid, &status, 0);

      /* currently the shell has no support for examining the return state
       * of the process but eventually we will add support for it */

      curr_async_sequence++;
      async_sequence_num++;
    }

    /* don't forget to cleanup the dynamically allocated memory
     * on each loop */
    cleanup_sync_sequence(sync_sequence); 
    cleanup_token_list(&token_list);
  }

  exit(EXIT_SUCCESS);
}
