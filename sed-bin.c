#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATTERN_SIZE 1024
#define HOLD_SIZE 1024
#define MAX_MATCHES 9

int s(char *pattern_space, const char* pattern, const char* replace) {
  regex_t regex;

  if (regcomp(&regex, pattern, 0)) {
    regfree(&regex);
    return 0;
  }

  regmatch_t pmatch[MAX_MATCHES];
  if (regexec(&regex, pattern_space, MAX_MATCHES, pmatch, 0)) {
    regfree(&regex);
    return 0;
  }

  const int so = pmatch[0].rm_so;
  const int eo = pmatch[0].rm_eo;
  const int replace_len = strlen(replace);
  int po;
  int ro;
  for (po = so, ro = 0; po < eo && ro < replace_len; ++po, ++ro) {
    pattern_space[po] = replace[ro];
  }
  regfree(&regex);
  return 1;
}

int main(void) {
  char pattern_space[PATTERN_SIZE];
  char hold_space[HOLD_SIZE];
  strcpy(pattern_space, "Hello World!");

  if (s(pattern_space, "lo", "ye"))
    puts(pattern_space);

  return 0;
}
