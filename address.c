#include <assert.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>

#include "address.h"

bool addr_rr(Status *const status, const char *const start, const char *const end) {
  return false; // TODO
}

bool addr_rn(Status *const status, const char *const start, const int end) {
  return false; // TODO
}

bool addr_nr(Status *const status, const int start, const char *const end) {
  return false; // TODO
}

bool addr_nn(Status *const status, const int start, const int end) {
  return false; // TODO
}

bool addr_r(Status *const status, const char *const regex) {
  const char *const pattern_space = status->pattern_space;
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

bool addr_n(const Status *status, const int line_nb) {
  return status->line_nb == line_nb;
}
