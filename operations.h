#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdbool.h>

#include "status.h"

#define S_OPT_G 0x01

void d(Status *status);
operation_ret D(Status *status);
void equal(Status *status);
void g(Status *status);
void G(Status *status);
void h(Status *status);
void H(Status *status);
operation_ret n(Status *status);
operation_ret N(Status *status);
void p(const Status *status);
void P(const Status *status);
void q(const Status *status);
void s(
  Status *status,
  const char* pattern,
  const char* replace,
  const int opts
);
void x(Status *status);

#endif /* OPERATIONS_H */
