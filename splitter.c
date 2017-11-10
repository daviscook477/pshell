/*
 * Copyright Davis Cook 2017
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "splitter.h"
#include "tokenizer.h"
#include "pshell.h"

/*
 * initializes a token list list
 */
static void init_token_list_list(Token_list_list *token_list_list) {
  if (token_list_list == NULL) return;

  token_list_list->head = NULL;
  token_list_list->iterator = NULL;
}

/*
 * initializes a token list node
 */
static void init_token_list_node(Token_list_node *token_list_node) {
  if (token_list_node == NULL) return;

  token_list_node->token_list = NULL;
  token_list_node->next = NULL;
}

/*
 * adds a new token list to the end of a token list list
 */
static void add_token_list(Token_list_list *token_list_list, Token_list *token_list) {
  Token_list_node *token_list_node;

  if (token_list_list == NULL || token_list == NULL) return;

  token_list_node = malloc(sizeof(Token_list_node));
  MEM_CHECK(token_list_node);
  init_token_list_node(token_list_node);
  token_list_node->token_list = token_list;

  if (token_list_list->iterator == NULL) {
    token_list_list->head = token_list_node;
  } else {
    token_list_list->iterator->next = token_list_node;
  }
  token_list_list->iterator = token_list_node;
}

/*
 * splits a list of tokens into a list of token lists
 * using a delimiter
 */
Token_list_list split(Token_list token_list, char delimiter[]) {
  Token_list_list token_list_list;
  Token_list *curr_token_list;
  Token curr_token;
  int in_list = 0;

  init_token_list_list(&token_list_list);

  /* avoid NULL pointer errors */
  if (delimiter == NULL) return token_list_list;

  /* init the current token list */
  curr_token_list = malloc(sizeof(Token_list));
  MEM_CHECK(curr_token_list);
  init_token_list(curr_token_list);

  begin_iter(&token_list);
  while (has_token(token_list)) {
    curr_token = next_token(&token_list);
    if (in_list) {
      if (strcmp(curr_token.data, delimiter) == 0) {
        add_token_list(&token_list_list, curr_token_list);
        in_list = 0;
        curr_token_list = malloc(sizeof(Token_list));
        MEM_CHECK(curr_token_list);
        init_token_list(curr_token_list);
      } else {
        add_token(curr_token_list, curr_token.data, curr_token.was_quoted);
      }
    } else {
      if (strcmp(curr_token.data, delimiter) == 0) {
        add_token_list(&token_list_list, curr_token_list);
        curr_token_list = malloc(sizeof(Token_list));
        MEM_CHECK(curr_token_list);
        init_token_list(curr_token_list);
      } else {
        add_token(curr_token_list, curr_token.data, curr_token.was_quoted);
        in_list = 1;
      }
    }

    cleanup_token(&curr_token);
  }

  if (in_list) {
    add_token_list(&token_list_list, curr_token_list);
  }

  return token_list_list;
}


/*
 * counts the size of a token list list and resets
 * the iterator for that token list list
 */
int count_token_list_list_size(Token_list_list *token_list_list) {
  int count;

  count = 0;
  token_list_list_begin_iter(token_list_list);
  while (has_token_list(*token_list_list)) {
    next_token_list(token_list_list);
    count++;
  }
  token_list_list_begin_iter(token_list_list);

  return count;
}

/*
 * begin iterating over a token list list
 */
void token_list_list_begin_iter(Token_list_list *token_list_list) {
  if (token_list_list == NULL) return;

  token_list_list->iterator = token_list_list->head;
}

/*
 * check if a token list list has another token list
 */
int has_token_list(Token_list_list token_list_list) {
  return (token_list_list.iterator != NULL);
}

/*
 * get the next token list in a token list list
 *
 * returns the token list at the current
 * position in the token list list
 * NOTE: this is NOT A COPY
 *
 * this token list does not need to
 * be deallocated as long as you call
 * cleanup_token_list_list at the end
 * but if you don't then you will need
 * to deallocate this token_list using
 * cleanup_token_list in "tokenizer.h"
 */
Token_list *next_token_list(Token_list_list *token_list_list) {
  Token_list *token_list;

  if (token_list_list == NULL ||
    token_list_list->iterator == NULL) return NULL;

  token_list = token_list_list->iterator->token_list;
  token_list_list->iterator = token_list_list->iterator->next;

  return token_list;
}

/*
 * free all the space used by a token list list
 */
void cleanup_token_list_list(Token_list_list *token_list_list) {
  Token_list_node *curr, *tmp;

  if (token_list_list == NULL) return;

  curr = token_list_list->head;
  while (curr != NULL) {
    tmp = curr->next;
    cleanup_token_list(curr->token_list);
    free(curr->token_list);
    free(curr);
    curr = tmp;
  }
}
