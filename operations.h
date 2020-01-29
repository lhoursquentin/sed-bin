#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdbool.h>

#include "status.h"

#define S_OPT_G 0x01

void g(Status *status);
void h(Status *status);
void p(const Status *status);
void s(
  Status *status,
  const char* pattern,
  const char* replace,
  const int opts
);
void x(Status *status);

#endif /* OPERATIONS_H */
