/*
 * Copyright Davis Cook 2017
 */

#ifndef SPLITTER_H
#define SPLITTER_H

#include "tokenizer.h"

/*
 * a linked list of token lists
 */
typedef struct token_list_node {
  Token_list *token_list;
  struct token_list_node *next;
} Token_list_node;

typedef struct token_list_list {
  Token_list_node *head, *iterator;
} Token_list_list;

/*
 * define function for building a
 * token list list
 */
Token_list_list split(Token_list token_list, char delimiter[]);

/*
 * define functions for traversing
 * a list of list of tokens
 */
int count_token_list_list_size(Token_list_list *token_list_list);
void token_list_list_begin_iter(Token_list_list *token_list_list);
int has_token_list(Token_list_list token_list_list);
Token_list *next_token_list(Token_list_list *token_list_list);
void cleanup_token_list_list(Token_list_list *token_list_list);

#endif
