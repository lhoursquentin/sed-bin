#include <assert.h>
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "operations.h"
#include "status.h"
#include "read.h"

static size_t expand_replace(
    char *const replace_expanded,
    const char *const pattern_space,
    const char *const replace,
    const regmatch_t *pmatch) {
  const size_t replace_len = strlen(replace);
  bool found_backslash = false;
  size_t replace_expanded_index = 0;
  for (size_t replace_index = 0; replace_index < replace_len; ++replace_index) {
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
          const size_t so = pmatch[0].rm_so;
          const size_t eo = pmatch[0].rm_eo;
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
          const size_t so = pmatch[back_ref_index].rm_so;
          if (so == -1) {
            return -1;
          }
          const size_t eo = pmatch[back_ref_index].rm_eo;
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

static size_t substitution(
    regex_t *const regex,
    char *pattern_space,
    const char *const replace,
    size_t *const sub_nb,
    const size_t nth
) {
  regmatch_t pmatch[MAX_MATCHES];
  if (regexec(
        regex,
        pattern_space,
        MAX_MATCHES,
        pmatch,
        *sub_nb > 0 ? REG_NOTBOL : 0
  )) {
    // Can return 0 later as well in cases like s/^//, rely on sub_nb value to
    // check if substitution happened
    return 0;
  }

  (*sub_nb)++;

  const size_t so = pmatch[0].rm_so; // start offset
  assert(so != -1);
  const size_t eo = pmatch[0].rm_eo; // end offset
  if (nth > *sub_nb) {
    return eo;
  }
  char replace_expanded[PATTERN_SIZE]; // TODO abitrary size, might be too small
  const size_t replace_expanded_len =
    expand_replace(replace_expanded, pattern_space, replace, pmatch);

  const size_t pattern_space_len = strlen(pattern_space);
  // empty match, s/^/foo/ for instance
  if (eo == 0) {
    if (*sub_nb == 1) {
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

  size_t po = 0;
  size_t ro = 0;

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

void a(Status *const status, const char *const output) {
  Pending_output *const p =
    &status->pending_outputs[status->pending_output_counter++];
  p->is_filepath = false;
  p->direct_output = output;
}

void c(Status *const status, const char *const output) {
  char *const pattern_space = status->pattern_space;
  pattern_space[0] = '\0';
  puts(output);
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
  const size_t line_nb = status->line_nb;
  printf("%zu\n", line_nb);
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
  const size_t pattern_space_len = strlen(pattern_space);
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
  const size_t hold_space_len = strlen(hold_space);
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

void l(const Status *const status) {
  const char *const pattern_space = status->pattern_space;
  for (size_t i = 0, fold_counter = 0; pattern_space[i]; ++i, ++fold_counter) {
    const char c = pattern_space[i];
    if (fold_counter > 80) {
      puts("\\");
      fold_counter = 0;
    }
    if (isprint(c)) {
      if (c == '\\') { // needs to be doubled
        putchar('\\');
        fold_counter++;
      }
      putchar(c);
    } else {
      fold_counter++;
      switch (c) {
        case '\n':
          // POSIX states:
          // > [...] '\t', '\v' ) shall be written as the corresponding escape
          // > sequence; the '\n' in that table is not applicable
          //
          // toybox and gnu sed still print newlines as "\n", I'll choose to
          // stick to my understanding of POSIX there.
          puts("$");
          fold_counter = 0;
        case '\a':
          printf("\\a");
          break;
        case '\b':
          printf("\\b");
          break;
        case '\f':
          printf("\\f");
          break;
        case '\r':
          printf("\\r");
          break;
        case '\t':
          printf("\\t");
          break;
        case '\v':
          printf("\\v");
          break;
        default:
          fold_counter += 2; // 3 counting the beginning of the else branch
          printf("\\%03hho", c);
          break;
      }
    }
  }
  puts("$");
}

operation_ret N(Status *const status) {
  char *const pattern_space = status->pattern_space;
  const size_t pattern_space_len = strlen(pattern_space);
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

void r(Status *const status, const char *const filepath) {
  Pending_output *const p =
    &status->pending_outputs[status->pending_output_counter++];
  p->is_filepath = true;
  p->filepath = filepath;
}

void s(
  Status *const status,
  Regex *const regex,
  const char *const replace,
  const size_t opts,
  const size_t nth,
  FILE *const f)
{
  status->last_regex = regex;
  regex_t *const regex_obj = &regex->obj;

  if (!regex->compiled) {
    if (regcomp(regex_obj, regex->str, 0)) {
      assert(false);
    } else {
      regex->compiled = true;
    }
  }

  const bool opt_g = opts & S_OPT_G;
  const bool opt_p = opts & S_OPT_P;

  char *pattern_space = status->pattern_space;
  size_t pattern_offset = 0;
  size_t sub_nb = 0;
  do {
    pattern_offset = substitution(
      regex_obj,
      pattern_space,
      replace,
      &sub_nb,
      nth
    );
    pattern_space += pattern_offset;
  } while ((opt_g || nth > sub_nb) && pattern_space[0] && pattern_offset);

  if (sub_nb >= nth) {
    status->sub_success = true;
    if (opt_p) {
      puts(status->pattern_space);
    }
    w(status, f);
  }
}

void w(const Status *const status, FILE *const f) {
  if (f) {
    fputs(status->pattern_space, f);
    fputc('\n', f); // unlike puts, fputs doesn't add a trailing newline

    // Potential following reads on the same file within the same sed script
    // should return the up-to-date content, this is used in tests to avoid
    // external checks and is correctly handled by GNU sed
    fflush(f);
  }
}

void x(Status *const status) {
  char *const pattern_space = status->pattern_space;
  char *const hold_space = status->hold_space;
  status->pattern_space = hold_space;
  status->hold_space = pattern_space;
}

void y(Status *const status, const char *const set1, const char *const set2) {
  char *const pattern_space = status->pattern_space;
  // Not the most efficient, might refactor this if I move to a C++ translation
  for (size_t pattern_index = 0; pattern_space[pattern_index]; ++pattern_index) {
    for (size_t set_index = 0; set1[set_index] && set2[set_index]; ++set_index) {
      if (pattern_space[pattern_index] == set1[set_index]) {
        pattern_space[pattern_index] = set2[set_index];
      }
    }
  }
}
