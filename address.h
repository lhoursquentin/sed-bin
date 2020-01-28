#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdbool.h>

#include "status.h"

bool addr_range_rr(const Status *status, const char *start, const char *end);
bool addr_range_rn(const Status *status, const char *start, const int end);
bool addr_range_nr(const Status *status, const int start, const char *end);
bool addr_range_nn(const Status *status, const int start, const int end);
bool addr_regex(const Status *status, const char *regex);
bool addr_number(const Status *status, const int line);

#endif /* ADDRESS_H */
