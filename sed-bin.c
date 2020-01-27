#include <stdbool.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PATTERN_SIZE 1024
#define MAX_MATCHES 9

typedef struct {
  char *pattern_space;
  char *hold_space;
  bool sub_success;
} Status;

int expand_replace(
    char *replace_expanded,
    const char *pattern_space,
    const char *replace,
    const regmatch_t *pmatch) {
  const int replace_len = strlen(replace);
  bool found_backslash = false;
  int replace_expanded_index = 0;
  char replace_char;
  for (int replace_index = 0; replace_index < replace_len; ++replace_index) {
    replace_char = replace[replace_index];
    switch (replace_char) {
      case '\\':
        // double backslash case
        if (found_backslash) {
          replace_expanded[replace_expanded_index++] = '\\';
        }
        found_backslash = !found_backslash;
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if (found_backslash) {
          const char back_ref_index = replace_char - '0';
          const int so = pmatch[back_ref_index].rm_so;
          if (so == -1) {
            return -1;
          }
          const int eo = pmatch[back_ref_index].rm_eo;
          memmove(
            replace_expanded + replace_expanded_index,
            pattern_space + so,
            eo
          );
          replace_expanded_index += eo - so;
          found_backslash = 0;
        } else {
          replace_expanded[replace_expanded_index++] = replace_char;
        }
        break;
      default:
        replace_expanded[replace_expanded_index++] = replace_char;
        if (found_backslash) {
          // ignore for now, at some point it might be nice to handle \x, \o, \n
          // etc.
          found_backslash = 0;
        }
        break;
    }
  }
  return replace_expanded_index;
}

bool s(Status *status, const char* pattern, const char* replace) {
  regex_t regex;

  // FIXME we should compile only once, both loops and each line processed can
  // lead to compiling the same regex many times
  if (regcomp(&regex, pattern, 0)) {
    regfree(&regex);
    return 0;
  }

  char *pattern_space = status->pattern_space;
  regmatch_t pmatch[MAX_MATCHES];
  if (regexec(&regex, pattern_space, MAX_MATCHES, pmatch, 0)) {
    regfree(&regex);
    return 0;
  }

  const int so = pmatch[0].rm_so; // start offset
  const int eo = pmatch[0].rm_eo; // end offset

  char replace_expanded[512]; // TODO abitrary size, might be too small
  const int replace_expanded_len =
    expand_replace(replace_expanded, pattern_space, replace, pmatch);
  const int pattern_space_len = strlen(pattern_space);

  int po;
  int ro;

  for (po = so, ro = 0; po < eo && ro < replace_expanded_len; ++po, ++ro) {
    pattern_space[po] = replace_expanded[ro];
  }

  if (po < eo) {
    memmove(
      pattern_space + po,
      pattern_space + eo,
      pattern_space_len - po
    );
  } else if (ro < replace_expanded_len) {
    memmove(
      pattern_space + eo + replace_expanded_len - ro,
      pattern_space + eo,
      pattern_space_len - eo
    );
    memmove(
      pattern_space + eo,
      replace_expanded + ro,
      replace_expanded_len - ro
    );

    pattern_space[pattern_space_len + replace_expanded_len - (eo - so)] = 0;
  }

  regfree(&regex);
  return true;
}

void x(Status *status) {
  char *pattern_space = status->pattern_space;
  char *hold_space = status->hold_space;
  status->pattern_space = hold_space;
  status->hold_space = pattern_space;
}

void g(Status *status) {
  char *pattern_space = status->pattern_space;
  const char *hold_space = status->hold_space;
  memcpy(
    pattern_space,
    hold_space,
    strlen(hold_space)
  );
}

void h(Status *status) {
  const char *pattern_space = status->pattern_space;
  char *hold_space = status->hold_space;
  memcpy(
    hold_space,
    pattern_space,
    strlen(pattern_space)
  );
}

void p(const Status *status) {
  const char *pattern_space = status->pattern_space;
  printf("%s\n", pattern_space);
}

void sed_script(Status *status) {
  // generated c code from sed script parsing goes here
}

int main(int argc, char **argv) {
  if (argc < 2) {
    return 1;
  }

  Status status = {
    (char[PATTERN_SIZE]){},
    (char[PATTERN_SIZE]){},
    false
  };

  while (fgets(status.pattern_space, PATTERN_SIZE, stdin)) {
    // TODO Dirty, multiple successive strlen on pattern_space, maybe I should
    // keep the length in Status and update it when needed?
    // This should also be done within a function (fgets + newline removal), to
    // avoid that tmp pattern_space variable becoming potentially pointing to
    // the hold after performing an x operation.
    char *pattern_space = status.pattern_space;
    int pattern_space_len = strlen(pattern_space);
    if (pattern_space_len && pattern_space[pattern_space_len - 1] == '\n') {
      pattern_space[pattern_space_len - 1] = 0;
    }

    s(&status, argv[1], argv[2]); // for testing

    // sed_script(&status);

    // x(&status);
    // s(&status, ".*", "space");
    // p(&status);
    // x(&status);

    // pattern_space might be different than status.pattern_space at this point
    // due to the `x' operation swapping the hold with the pattern
    printf("%s\n", status.pattern_space);
  }
  return EXIT_SUCCESS;
}
