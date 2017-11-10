/*
 * Copyright Davis Cook 2017
 */

/*
 * test for "tokenizer.h"
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tokenizer.h"

#define TEST_SUCCEEDED 0
#define TEST_FAILED 1

/*
 * runs through a variety of input lines and
 * confirms that tokenizer creates the right
 * tokens for each input line
 */
int main() {
  Token_list token_list;
  int num_tests = 6;
  char *test_input[] = {"Hello World",
    "Bob",
    " Hello World!    \t",
    " \"ls\" -\"l\" \"-\"a",
    "\\\"",
    "\\ \\  \\\\"};
  char *expected_output[][4] = {{"Hello", "World", NULL},
    {"Bob", NULL},
    {"Hello", "World!", NULL},
    {"ls", "-l", "-a", NULL},
    {"\"", NULL},
    {"  ", "\\", NULL}};
  int expected_quotes[][4] = {{0, 0},
    {0},
    {0, 0},
    {1, 1, 1},
    {0},
    {0, 0}};
  int i, j;
  Token token;

  for (i = 0; i < num_tests; i++) {
    printf("Testing the parser on \"%s\"\n", test_input[i]);
    token_list = parse_tokens(test_input[i]);
    begin_iter(&token_list);
    if (!has_token(token_list)) {
      printf("Parsing tokens failed!\n");
      exit(TEST_FAILED);
    }
    j = 0;
    while (has_token(token_list) && expected_output[i][j] != NULL) {
      token = next_token(&token_list);
      if (strcmp(token.data, expected_output[i][j]) != 0) {
        printf("Tokens not as expected!\n");
        exit(TEST_FAILED);
      } else {
        printf("Token as expected!\n");
      }
      if (token.was_quoted != expected_quotes[i][j]) {
        printf("Token quote state not as expected!\n");
      } else {
        printf("Token quote state as expected!\n");
      }
      printf("Expected: \"%s\", Got: \"%s\"\n", expected_output[i][j], token.data);
      printf("Expected quote state: %d, Got: %d\n", expected_quotes[i][j], token.was_quoted);
      j++;
      cleanup_token(&token);
    }
    if (has_token(token_list)) {
      printf("Had too many tokens!\n");
      token = next_token(&token_list);
      printf("Extra token: \"%s\"\n", token.data);
      cleanup_token(&token);
    }
    if (expected_output[i][j] != NULL) {
      printf("Had too few tokens!\n");
      printf("Missing token: \"%s\"\n", expected_output[i][j]);
    }
    cleanup_token_list(&token_list);
    printf("\n");
  }

  exit(TEST_SUCCEEDED);
}
