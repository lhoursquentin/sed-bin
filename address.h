#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdbool.h>

#include "status.h"

bool addr_nn(const Status *const status, const size_t start, const size_t end);
bool addr_nr(Status *const status, const size_t start, Regex *const end, const size_t id);
bool addr_rn(Status *const status, Regex *const start, const size_t end, const size_t id);
bool addr_rr(Status *const status, Regex *const start, Regex *const end, const size_t id);

bool addr_n(const Status *const status, const size_t line);
bool addr_r(Status *const status, Regex *const regex);

#endif /* ADDRESS_H */
