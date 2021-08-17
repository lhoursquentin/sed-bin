#include <assert.h>
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "operations.h"
#include "read.h"
#include "status.h"

static const char *const get_nearest_newline_ptr(
  const char *const s,
  size_t limit
) {
  for (size_t i = 0; i < limit; ++i) {
    if (s[i] == '\n') {
      return s + i;
    }
  }
  return NULL;
};

static size_t expand_replace(
  char *const replace_expanded,
  const char *const pattern_space,
  const char *const replace,
  const regmatch_t *pmatch
) {
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
          const regoff_t so = pmatch[0].rm_so;
          const regoff_t eo = pmatch[0].rm_eo;
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
          const size_t back_ref_index = replace_char - '0';
          const regoff_t so = pmatch[back_ref_index].rm_so;
          // case when there is match but the capture group is empty:
          //   echo foo | sed 's/\(x\)*foo/\1bar/'
          // here the substitution is done but \1 is empty
          if (so != -1) {
            const regoff_t eo = pmatch[back_ref_index].rm_eo;
            memmove(
              replace_expanded + replace_expanded_index,
              pattern_space + so,
              eo
            );
            replace_expanded_index += eo - so;
          }
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
  const size_t pattern_space_len,
  const char *const replace,
  size_t *const sub_nb,
  const size_t nth,
  ssize_t *const nb_chars_removed
) {
  regmatch_t pmatch[MAX_MATCHES];
  // unfortunately regexec does not allow to pass a custom length, requiring a
  // 0 char insertion
  pattern_space[pattern_space_len] = '\0';
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

  const regoff_t so = pmatch[0].rm_so; // start offset
  assert(so != -1);
  const regoff_t eo = pmatch[0].rm_eo; // end offset
  if (nth > *sub_nb) {
    return eo;
  }

  // TODO arbitrary size, might be too small
  char replace_expanded[PATTERN_SIZE];
  const size_t replace_expanded_len =
    expand_replace(replace_expanded, pattern_space, replace, pmatch);
  (*nb_chars_removed) = eo - so - replace_expanded_len;

  // empty match, s/^/foo/ for instance
  if (eo == 0) {
    if (*sub_nb == 1) {
      memmove(
        pattern_space + replace_expanded_len,
        pattern_space,
        pattern_space_len
      );
      memmove(pattern_space, replace_expanded, replace_expanded_len);
      return replace_expanded_len;
    } else if (pattern_space_len == 1) {
      // case:  echo 'Hello ' | sed 's|[^ ]*|yo|g'
      pattern_space++;
      memmove(pattern_space, replace_expanded, replace_expanded_len);
      return replace_expanded_len + 1; // +1 since we did pattern_space++
    }
    (*nb_chars_removed) = 0;
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
  status->pattern_space.length = 0;
  puts(output);
}

void d(Status *const status) {
  status->pattern_space.length = 0;
}

operation_ret D(Status *const status) {
  char *const pattern_space = status->pattern_space.str;
  const char *const newline_location = get_nearest_newline_ptr(
    pattern_space,
    status->pattern_space.length
  );
  if (newline_location == NULL) {
    status->pattern_space.length = 0;
    return CONTINUE;
  }

  const size_t first_line_length = newline_location - pattern_space;
  status->pattern_space.length -= first_line_length + 1; // + 1 for \n
  // Backward memmove instead of moving the pattern space ptr forward because
  // this would mean losing part of the limited stack space that we have
  memmove(
    pattern_space,
    newline_location + 1, // + 1 to start copying after the newline
    status->pattern_space.length
  );
  status->skip_read = true;
  return CONTINUE;
}

void equal(const Status *const status) {
  const size_t line_nb = status->line_nb;
  printf("%zu\n", line_nb);
}

void g(Status *const status) {
  memcpy(
    status->pattern_space.str,
    status->hold_space.str,
    status->hold_space.length
  );
  status->pattern_space.length = status->hold_space.length;
}

void G(Status *status) {
  char *const pattern_space = status->pattern_space.str;
  const char *const hold_space = status->hold_space.str;
  const size_t pattern_space_len = status->pattern_space.length;
  const size_t hold_space_len = status->hold_space.length;
  memcpy(
    pattern_space + pattern_space_len + 1, // we'll place the \n in between
    hold_space,
    hold_space_len
  );
  pattern_space[pattern_space_len] = '\n';
  status->pattern_space.length += hold_space_len + 1;
}

void h(Status *status) {
  memcpy(
    status->hold_space.str,
    status->pattern_space.str,
    status->pattern_space.length
  );
  status->hold_space.length = status->pattern_space.length;
}

void H(Status *status) {
  char *const hold_space = status->hold_space.str;
  const char *const pattern_space = status->pattern_space.str;
  const size_t hold_space_len = status->hold_space.length;
  const size_t pattern_space_len = status->pattern_space.length;
  memcpy(
    hold_space + hold_space_len + 1, // we'll place the \n in between
    pattern_space,
    pattern_space_len
  );
  hold_space[hold_space_len] = '\n';
  status->hold_space.length += pattern_space_len + 1;
}

void i(const char *const output) {
  puts(output);
}

void l(const Status *const status) {
  const char *const pattern_space = status->pattern_space.str;
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

operation_ret n(Status *const status) {
  if (!status->suppress_default_output) {
    p(status);
  }
  ssize_t nb_chars_read = read_pattern(
    status,
    status->pattern_space.str,
    PATTERN_SIZE
  );
  if (nb_chars_read == -1) {
    return BREAK;
  }
  status->pattern_space.length = nb_chars_read;
  return 0;
}

operation_ret N(Status *const status) {
  char *const pattern_space = status->pattern_space.str;
  const size_t pattern_space_len = status->pattern_space.length;
  ssize_t nb_chars_read = read_pattern(
    status,
    pattern_space + pattern_space_len + 1,
    PATTERN_SIZE - pattern_space_len - 1
  );
  if (nb_chars_read == -1) {
    return BREAK;
  }
  pattern_space[pattern_space_len] = '\n';
  status->pattern_space.length += nb_chars_read + 1;
  return 0;
}

void p(const Status *const status) {
  fwrite(
    status->pattern_space.str,
    sizeof(char),
    status->pattern_space.length,
    stdout
  );
  putchar('\n');
  fflush(stdout);
}

void P(const Status *const status) {
  const char *const pattern_space = status->pattern_space.str;
  const char *const newline_location = get_nearest_newline_ptr(
    pattern_space,
    status->pattern_space.length
  );
  if (newline_location) {
    const unsigned int first_line_length = newline_location - pattern_space;
    fwrite(
      status->pattern_space.str,
      sizeof(char),
      first_line_length + 1,
      stdout
    );
  } else {
    p(status);
  }
}

void q(const Status *const status) {
  if (!status->suppress_default_output) {
    p(status);
  }
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
  FILE *const f
) {
  status->last_regex = regex;
  regex_t *const regex_obj = &regex->obj;

  if (!regex->compiled) {
    assert(regcomp(regex_obj, regex->str, 0) == 0);
    regex->compiled = true;
  }

  const bool opt_g = opts & S_OPT_G;
  const bool opt_p = opts & S_OPT_P;

  char *pattern_space = status->pattern_space.str;
  const size_t initial_pattern_space_len = status->pattern_space.length;
  size_t sub_nb = 0;
  size_t total_offset = 0;
  ssize_t total_nb_chars_removed = 0;
  do {
    const size_t initial_sub_nb = sub_nb;
    ssize_t nb_chars_removed = 0;
    const size_t pattern_offset = substitution(
      regex_obj,
      pattern_space,
      initial_pattern_space_len - total_nb_chars_removed - total_offset,
      replace,
      &sub_nb,
      nth,
      &nb_chars_removed
    );
    if (initial_sub_nb == sub_nb) {
      break;
    }
    total_offset += pattern_offset;
    pattern_space += pattern_offset;
    total_nb_chars_removed += nb_chars_removed;
  } while (
    (opt_g || nth > sub_nb) &&
    initial_pattern_space_len - total_nb_chars_removed > total_offset
  );

  if (sub_nb >= nth) {
    status->pattern_space.length -= total_nb_chars_removed;
    status->sub_success = true;
    if (opt_p) {
      p(status);
    }
    w(status, f);
  }
}

void w(const Status *const status, FILE *const f) {
  if (f) {
    fwrite(
      status->pattern_space.str,
      sizeof(char),
      status->pattern_space.length,
      f
    );
    fputc('\n', f);

    // Potential following reads on the same file within the same sed script
    // should return the up-to-date content, this is used in tests to avoid
    // external checks and is correctly handled by GNU sed
    fflush(f);
  }
}

void x(Status *const status) {
  const String pattern_space = status->pattern_space;
  const String hold_space = status->hold_space;
  status->pattern_space = hold_space;
  status->hold_space = pattern_space;
}

void y(Status *const status, const char *const set1, const char *const set2) {
  char *const pattern_space = status->pattern_space.str;
  // Not the most efficient, might refactor this if I move to a C++ translation
  for (size_t pattern_index = 0; pattern_space[pattern_index]; ++pattern_index) {
    for (size_t set_index = 0; set1[set_index] && set2[set_index]; ++set_index) {
      if (pattern_space[pattern_index] == set1[set_index]) {
        pattern_space[pattern_index] = set2[set_index];
      }
    }
  }
}
