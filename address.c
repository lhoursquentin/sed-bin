#include <stdbool.h>
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
  return false; // TODO
}

bool addr_number(const Status *status, const int line_nb) {
  return status->line_nb == line_nb;
}
