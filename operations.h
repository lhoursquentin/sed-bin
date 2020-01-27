#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdbool.h>

#include "status.h"

void g(Status *status);
void h(Status *status);
void p(const Status *status);
bool s(Status *status, const char* pattern, const char* replace);
void x(Status *status);

#endif /* OPERATIONS_H */
