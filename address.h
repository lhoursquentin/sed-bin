#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdbool.h>

#include "status.h"

bool addr_rr(const Status *status, const char *start, const char *end);
bool addr_rn(const Status *status, const char *start, const int end);
bool addr_nr(const Status *status, const int start, const char *end);
bool addr_nn(const Status *status, const int start, const int end);
bool addr_r(const Status *status, const char *regex);
bool addr_n(const Status *status, const int line);

#endif /* ADDRESS_H */
