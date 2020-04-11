#include <assert.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "operations.h"
#include "status.h"
#include "read.h"

static int expand_replace(
    char *const replace_expanded,
    const char *const pattern_space,
    const char *const replace,
    const regmatch_t *pmatch) {
  const int replace_len = strlen(replace);
  bool found_backslash = false;
  int replace_expanded_index = 0;
  for (int replace_index = 0; replace_index < replace_len; ++replace_index) {
    const char replace_char = replace[replace_index];
    switch (replace_char) {
      case '\\':
        // double backslash case
        if (found_backslash) {
          replace_expanded[replace_expanded_index++] = '\\';
        }
        found_backslash = !found_backslash;
        break;
      case '&':
        if (!found_backslash) {
          const int so = pmatch[0].rm_so;
          const int eo = pmatch[0].rm_eo;
          memmove(
            replace_expanded + replace_expanded_index,
            pattern_space + so,
            eo
          );
          replace_expanded_index += eo - so;
        } else {
          replace_expanded[replace_expanded_index++] = replace_char;
          found_backslash = false;
        }
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
          found_backslash = false;
        } else {
          replace_expanded[replace_expanded_index++] = replace_char;
        }
        break;
      default:
        if (found_backslash) {
          found_backslash = false;
          if (replace_char == 'n') {
            replace_expanded[replace_expanded_index++] = '\n';
          }
        } else {
          replace_expanded[replace_expanded_index++] = replace_char;
        }
        break;
    }
  }
  return replace_expanded_index;
}

static int substitution(
    regex_t *const regex,
    char *pattern_space,
    const char *const replace,
    const bool first_sub_done
) {
  regmatch_t pmatch[MAX_MATCHES];
  if (regexec(
        regex,
        pattern_space,
        MAX_MATCHES,
        pmatch,
        first_sub_done ? REG_NOTBOL : 0
  )) {
    regfree(regex);
    return -1;
  }

  const int pattern_space_len = strlen(pattern_space);
  const int so = pmatch[0].rm_so; // start offset
  assert(so != -1);
  const int eo = pmatch[0].rm_eo; // end offset
  char replace_expanded[512]; // TODO abitrary size, might be too small
  const int replace_expanded_len =
    expand_replace(replace_expanded, pattern_space, replace, pmatch);

  // empty match, s/^/foo/ for instance
  if (eo == 0) {
    if (!first_sub_done) {
      memmove(
        pattern_space + replace_expanded_len,
        pattern_space,
        pattern_space_len + 1 // include \0
      );
      memmove(pattern_space, replace_expanded, replace_expanded_len);
      return replace_expanded_len;
    } else if (pattern_space_len == 1) {
      // case:  echo 'Hello ' | sed 's|[^ ]*|yo|g'
      pattern_space++;
      memmove(pattern_space, replace_expanded, replace_expanded_len);
      pattern_space[replace_expanded_len] = '\0';
      return replace_expanded_len + 1; // +1 since we did pattern_space++
    }
    return 1;
  }

  int po = 0;
  int ro = 0;

  for (po = so; po < eo && ro < replace_expanded_len; ++po, ++ro) {
    pattern_space[po] = replace_expanded[ro];
  }

  if (po < eo) {
    // Matched part was longer than replaced part, let's shift the rest to the
    // left.
    memmove(
      pattern_space + po,
      pattern_space + eo,
      pattern_space_len - po
    );
    return po;
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
    return so + replace_expanded_len;
  }
  return eo;
}

void s(
  Status *const status,
  const char *const pattern,
  const char *const replace,
  const int opts)
{
  status->last_pattern = pattern;
  regex_t regex;

  // FIXME we should compile only once, both loops and each line processed can
  // lead to compiling the same regex many times
  if (regcomp(&regex, pattern, 0)) {
    regfree(&regex);
    assert(false);
  }

  // TODO nth/w opts
  const bool opt_g = opts & S_OPT_G;
  const bool opt_p = opts & S_OPT_P;

  char *pattern_space = status->pattern_space;
  int pattern_offset = 0;
  bool first_sub_done = false;
  do {
    pattern_offset = substitution(
      &regex,
      pattern_space,
      replace,
      first_sub_done
    );
    if (pattern_offset == -1) {
      break;
    }
    // if opt_g is enabled then we want to avoid ^ to keep its meaning for the
    // next iterations
    first_sub_done = true;
    pattern_space += pattern_offset;
  } while (opt_g && pattern_space[0] && pattern_offset);

  regfree(&regex);

  if (first_sub_done) {
    status->sub_success = true;
    if (opt_p) {
      puts(status->pattern_space);
    }
  }
}

void x(Status *const status) {
  char *const pattern_space = status->pattern_space;
  char *const hold_space = status->hold_space;
  status->pattern_space = hold_space;
  status->hold_space = pattern_space;
}

void a(Status *const status, const char *const output) {
  status->pending_output[status->pending_output_counter++] = output;
}

void d(Status *const status) {
  status->pattern_space[0] = '\0';
}

operation_ret D(Status *const status) {
  char *const pattern_space = status->pattern_space;
  const char *const newline_location = strchr(pattern_space, '\n');
  if (newline_location == NULL) {
    pattern_space[0] = '\0';
    return CONTINUE;
  }

  memmove(
    pattern_space,
    newline_location + 1, // + 1 to start copying after the newline
    strlen(newline_location + 1) + 1 // last +1 to move \0 as well
  );
  status->skip_read = true;
  return CONTINUE;
}

void equal(const Status *const status) {
  const unsigned int line_nb = status->line_nb;
  printf("%d\n", line_nb);
}

void g(Status *const status) {
  char *const pattern_space = status->pattern_space;
  const char *const hold_space = status->hold_space;
  memcpy(
    pattern_space,
    hold_space,
    strlen(hold_space) + 1 // include \0
  );
}

void G(Status *status) {
  char *const pattern_space = status->pattern_space;
  const char *const hold_space = status->hold_space;
  const int pattern_space_len = strlen(pattern_space);
  memcpy(
    pattern_space + pattern_space_len + 1, // we'll place the \n in between
    hold_space,
    strlen(hold_space) + 1 // include \0
  );
  pattern_space[pattern_space_len] = '\n';
}

void h(Status *status) {
  const char *const pattern_space = status->pattern_space;
  char *const hold_space = status->hold_space;
  memcpy(
    hold_space,
    pattern_space,
    strlen(pattern_space) + 1 // include \0
  );
}

void H(Status *status) {
  const char *const pattern_space = status->pattern_space;
  char *const hold_space = status->hold_space;
  const int hold_space_len = strlen(hold_space);
  memcpy(
    hold_space + hold_space_len + 1, // we'll place the \n in between
    pattern_space,
    strlen(pattern_space) + 1 // include \0
  );
  hold_space[hold_space_len] = '\n';
}

void i(const char *const output) {
  puts(output);
}

operation_ret n(Status *const status) {
  puts(status->pattern_space);
  if (!read_pattern(status, status->pattern_space, PATTERN_SIZE)) {
    return BREAK;
  }
  return 0;
}

operation_ret N(Status *const status) {
  char *const pattern_space = status->pattern_space;
  const int pattern_space_len = strlen(pattern_space);
  if (!read_pattern(
        status,
        pattern_space + pattern_space_len + 1,
        PATTERN_SIZE - pattern_space_len - 1)
  ) {
    return BREAK;
  }
  pattern_space[pattern_space_len] = '\n';
  return 0;
}

void p(const Status *const status) {
  const char *const pattern_space = status->pattern_space;
  puts(pattern_space);
}

void P(const Status *const status) {
  const char *const pattern_space = status->pattern_space;
  printf(
    "%.*s\n",
    strchr(pattern_space, '\n') - pattern_space,
    pattern_space
  );
}

void q(const Status *const status) {
  p(status);
  exit(0);
}
