#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

  const int pattern_space_len = strlen(pattern_space);
  const int replace_len = strlen(replace);

  int po;
  int ro;

  // fixed_replace = expand_replace()

  for (po = so, ro = 0; po < eo && ro < replace_len; ++po, ++ro) {
    pattern_space[po] = replace[ro];
  }

  if (po < eo) {
    memmove(
      pattern_space + po,
      pattern_space + eo,
      pattern_space_len - po
    );
  } else if (ro < replace_len) {
    memmove(
      pattern_space + eo + replace_len - ro,
      pattern_space + eo,
      pattern_space_len - eo
    );
    memmove(
      pattern_space + eo,
      replace + ro,
      replace_len - ro
    );

    pattern_space[pattern_space_len + replace_len - (eo - so)] = 0;
  }

  regfree(&regex);
  return 1;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    return 1;
  }
  char pattern_space[PATTERN_SIZE];
  char hold_space[HOLD_SIZE];
  int nb_read = read(STDIN_FILENO, &pattern_space, PATTERN_SIZE);
  pattern_space[nb_read] = 0;

  s(pattern_space, argv[1], argv[2]);
  puts(pattern_space);

  return 0;
}
