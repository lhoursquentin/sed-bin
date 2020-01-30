#include <assert.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>

#include "address.h"

bool addr_range_rr(const Status *status, const char *start, const char *end) {
  return false; // TODO
}

bool addr_range_rn(const Status *status, const char *start, const int end) {
  return false; // TODO
}

bool addr_range_nr(const Status *status, const int start, const char *end) {
  return false; // TODO
}

bool addr_range_nn(const Status *status, const int start, const int end) {
  return false; // TODO
}

bool addr_regex(const Status *status, const char *regex) {
  char *pattern_space = status->pattern_space;
  regex_t regex_obj;

  if (regcomp(&regex_obj, regex, 0)) {
    regfree(&regex_obj);
    assert(false);
  }

  if (regexec(&regex_obj, pattern_space, 0, NULL, 0)) {
    regfree(&regex_obj);
    return false;
  }

  regfree(&regex_obj);
  return true;
}

bool addr_number(const Status *status, const int line_nb) {
  return status->line_nb == line_nb;
}
