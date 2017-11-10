/*
 * Copyright Davis Cook 2017
 */

#ifndef TOKENIZER_H
#define TOKENIZER_H

/*
 * the size of the extra NUL character
 * that must be used to terminate all
 * characters
 */
#define NUL_TERM_SIZE 1

/*
 * a linked list of tokens
 */
typedef struct token {
  char *data;
  int was_quoted;
  struct token *next;
} Token;

typedef struct token_list {
  Token *head, *iterator;
} Token_list;

/*
 * define functions for building a
 * token list
 */
void init_token_list(Token_list *token_list);
void add_token(Token_list *token_list, char *element, int was_quoted);
Token_list parse_tokens(char *line);


/*
 * define functions for traversing
 * a list of tokens
 */
int count_token_list_size(Token_list *token_list);
void begin_iter(Token_list *token_list);
int has_token(Token_list token_list);
Token next_token(Token_list *token_list);
void cleanup_token(Token *token);
void cleanup_token_list(Token_list *token_list);

#endif
