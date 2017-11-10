/*
 * Copyright Davis Cook 2017
 */

/*
 * this helper function creates tokens
 * from the input to pshell by creating
 * a single token from each string of
 * non-whitespace characters or from each
 * string of characters inside double quotes
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tokenizer.h"

#define IS_WHITE_SPACE(character) ((character) == ' ' || (character) == '\t' || \
  (character) == '\n' || (character) == '\v' || (character) == '\f' || \
  (character) == '\r')

#define MAX_TOKEN_SIZE 300

#define WAS_QUOTED_ERROR -1

/*
 * define prototypes
 */
static void init_token(Token *token);

/*
 * initializes a token list
 */
void init_token_list(Token_list *token_list) {
  if (token_list == NULL) return;

  token_list->head = NULL;
  token_list->iterator = NULL;
}

/*
 * initializes a token
 */
static void init_token(Token *token) {
  if (token == NULL) return;

  token->data = NULL;
  token->was_quoted = WAS_QUOTED_ERROR;
  token->next = NULL;
}

/*
 * adds a new token to the end of a token list
 */
void add_token(Token_list *token_list, char *element, int was_quoted) {
  Token *token;

  if (token_list == NULL || element == NULL) return;

  token = malloc(sizeof(Token));
  token->next = NULL;
  token->data = malloc(sizeof(char) *
    (strlen(element) + NUL_TERM_SIZE));
  strcpy(token->data, element);
  token->was_quoted = was_quoted;

  if (token_list->iterator == NULL) {
    token_list->head = token;
  } else {
    token_list->iterator->next = token;
  }
  token_list->iterator = token;
}

/*
 * get a token list of all the
 * tokens in the input line
 */
Token_list parse_tokens(char *line) {
  int in_quotes = 0;
  int was_quoted = 0;
  int in_token = 0;
  int escape_next = 0;
  int i;
  char new_token[MAX_TOKEN_SIZE];
  int token_pos = 0;
  Token_list token_list;

  init_token_list(&token_list);

  /* return an empty list for a NULL pointer */
  if (line == NULL) return token_list;

  /* loop through all the characters in the line
   * in a single pass to generate the tokens */
  for (i = 0; i < strlen(line); i++) {

    /* fail if there is a token longer
     * than the max token size */
    if (token_pos >= MAX_TOKEN_SIZE) {
      /* cleanup any already used memory and
       * and return an empty list
       *
       * TODO: make this work with the error
       * code system for parsing to actually
       * be able to explain to the user why
       * this broke */
      cleanup_token_list(&token_list);
      init_token_list(&token_list);
      return token_list;
    }

    /* if we are in the middle of parsing a token
     * already in_token will be set and this code
     * will be executed */
    if (in_token) {
      
      /* switch the value of in_quotes if we find
       * a non-escaped double quote */
      if (!escape_next && line[i] == '"') {
        in_quotes = !in_quotes;
        if (in_quotes) {
          was_quoted = 1;
        }
      } else {

        /* in the case that the next character is a valid character to
         * add to the current token we first make sure it isn't a
         * backslash because if it's a backslash we set escape_next
         * to true and do not add it to the current token, otherwise
         * we add the valid character to the current token and unset
         * escape_next */
        if (!IS_WHITE_SPACE(line[i]) || in_quotes || escape_next) {
          if (!escape_next && line[i] == '\\') {
            escape_next = 1;
          } else {
            new_token[token_pos++] = line[i];
            escape_next = 0;
          }
        /* if the next character isn't a valid character we know
         * it's now time to end the current character by adding
         * it to the token_list and then resetting everything to
         * be ready for adding the next character */
        } else {
          new_token[token_pos] = '\0';
          token_pos = 0;
          in_token = 0;
          add_token(&token_list, new_token, was_quoted);
          was_quoted = 0;
        }
      }
    /* otherwise we are not already in the middle
     * of parsing a token so we ignore any whitespace
     * until we find a valid character to start a new token
     *
     * when that happens we set the proper values of
     * in_quotes and escape_next based on if the valid
     * character was a double quote or a backslash and
     * then set in_token to true */
    } else {
      if (!IS_WHITE_SPACE(line[i])) {
        if (line[i] == '"') {
          in_quotes = !in_quotes;
          if (in_quotes) {
            was_quoted = 1;
          }
        } else if (line[i] == '\\') {
          escape_next = 1;
        } else {
          new_token[token_pos++] = line[i];
        }
        in_token = 1;
      }
    }
  }

  /* TODO figure out a system for returning
   * parsing errors and AT LEAST send a parsing
   * error if we end up at the end of the parsing
   * with in_quotes still equal to 1 because all
   * opening quotes should have a closing pair
   * of quotes in a valid line to the shell */

  if (in_token) {
    new_token[token_pos] = '\0';
    add_token(&token_list, new_token, was_quoted);
  }

  return token_list;
}


/*
 * counts the size of a token list and resets
 * the iterator for that token list
 */
int count_token_list_size(Token_list *token_list) {
  int count;
  Token curr_token;

  count = 0;
  begin_iter(token_list);
  while (has_token(*token_list)) {
    curr_token = next_token(token_list);
    count++;
    cleanup_token(&curr_token);
  }
  begin_iter(token_list);

  return count;
}

/*
 * begin iterating over a token list
 */
void begin_iter(Token_list *token_list) {
  if (token_list == NULL) return;

  token_list->iterator = token_list->head;
}

/*
 * check if a token list has another token
 */
int has_token(Token_list token_list) {
  return (token_list.iterator != NULL);
}

/*
 * get the next token in a token list
 * 
 * returns a copy of the token
 * NOTE: the copy of the token will not
 * have its next member set
 *
 * this copy should be deallocated using
 * cleanup_token when you're done with it
 */
Token next_token(Token_list *token_list) {
  Token token;

  init_token(&token);

  if (token_list == NULL || token_list->iterator == NULL) return token;

  token.data = malloc(sizeof(char) *
    (strlen(token_list->iterator->data) + NUL_TERM_SIZE));
  strcpy(token.data, token_list->iterator->data);
  token.next = NULL;
  token.was_quoted = token_list->iterator->was_quoted;
  token_list->iterator = token_list->iterator->next;
  return token;
}

/*
 * free all the space used by a token
 */
void cleanup_token(Token *token) {
  if (token == NULL) return;

  free(token->data);
}

/*
 * free all the space used by a token list
 */
void cleanup_token_list(Token_list *token_list) {
  Token *curr, *tmp;

  if (token_list == NULL) return;

  curr = token_list->head;
  while (curr != NULL) {
    tmp = curr->next;
    free(curr->data);
    free(curr);
    curr = tmp;
  }
}
