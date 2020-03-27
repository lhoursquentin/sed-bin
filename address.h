#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdbool.h>

#include "status.h"

bool addr_rr(Status *const status, const char *const start, const char *const end, const int id);
bool addr_rn(Status *const status, const char *const start, const int end, const int id);
bool addr_nr(Status *const status, const int start, const char *const end, const int id);
bool addr_nn(Status *const status, const int start, const int end, const int id);
bool addr_r(Status *const status, const char * const regex);
bool addr_n(const Status *const status, const int line);

#endif /* ADDRESS_H */
