/*
 * Copyright Davis Cook 2017
 */

#ifndef PSHELL_STRUCTS_H
#define PSHELL_STRUCTS_H

struct command;
struct pipeline;
struct async_sequence;

typedef struct command {
  char *program;
  int num_args;
  char **arguments;
} Command;

typedef struct pipeline {
  int num_commands;
  struct command **commands;
} Pipeline;

typedef struct async_sequence {
  int num_pipelines;
  struct pipeline **pipelines;
} Async_sequence;

#endif
