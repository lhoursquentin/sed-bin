#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdbool.h>

#include "status.h"

#define S_OPT_G 0x01
#define S_OPT_P 0x02

void d(Status *const status);
operation_ret D(Status *const status);
void equal(const Status *const status);
void g(Status *const status);
void G(Status *const status);
void h(Status *const status);
void H(Status *const status);
operation_ret n(Status *const status);
operation_ret N(Status *const status);
void p(const Status *const status);
void P(const Status *const status);
void q(const Status *const status);
void s(
  Status *const status,
  const char *const pattern,
  const char *const replace,
  const int opts
);
void x(Status *const status);

#endif /* OPERATIONS_H */
