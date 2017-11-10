/*
 * Copyright Davis Cook 2017
 */

/*
 * test for "process-helper.h"
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pshell-structs.h"
#include "process-helper.h"

#define TEST_SUCCEEDED 0
#define TEST_FAILED 1

/*
 * runs through a variety of pipelines
 * and confirms the proper creation of
 * the pipes
 */
int main() {
  Pipeline pipeline;
  pipeline.num_commands = 2;
  pipeline.commands = malloc(sizeof(Command *) * 2);
  pipeline.commands[0] = malloc(sizeof(Command));
  pipeline.commands[0]->program = malloc(sizeof(char) * 3);
  strcpy(pipeline.commands[0]->program, "ls");
  pipeline.commands[0]->num_args = 1;
  pipeline.commands[0]->arguments = malloc(sizeof(char *) * 2);
  pipeline.commands[0]->arguments[0] = malloc(sizeof(char) * 3);
  strcpy(pipeline.commands[0]->arguments[0], "-l");
  pipeline.commands[0]->arguments[1] = NULL;
  
  pipeline.commands[1] = malloc(sizeof(Command));
  pipeline.commands[1]->program = malloc(sizeof(char) * 5);
  strcpy(pipeline.commands[1]->program, "grep");
  pipeline.commands[1]->num_args = 1;
  pipeline.commands[1]->arguments = malloc(sizeof(char *) * 2);
  pipeline.commands[1]->arguments[0] = malloc(sizeof(char) * 4);
  strcpy(pipeline.commands[1]->arguments[0], "dco");
  pipeline.commands[1]->arguments[1] = NULL;

  execute_pipeline(pipeline);
}
