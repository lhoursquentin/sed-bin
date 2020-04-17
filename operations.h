#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdbool.h>

#include "status.h"

#define S_OPT_G 0x01
#define S_OPT_P 0x02

void a(Status *const status, const char *const output);
void c(Status *const status, const char *const output);
void d(Status *const status);
operation_ret D(Status *const status);
void equal(const Status *const status);
void g(Status *const status);
void G(Status *const status);
void h(Status *const status);
void H(Status *const status);
void i(const char *const output);
void l(const Status *const status);
operation_ret n(Status *const status);
operation_ret N(Status *const status);
void p(const Status *const status);
void P(const Status *const status);
void q(const Status *const status);
void r(Status *const status, const char *const filepath);
void s(
  Status *const status,
  Regex *const regex,
  const char *const replace,
  const int opts,
  const int nth,
  FILE *const f
);
void w(const Status *const status, FILE *const f);
void x(Status *const status);
void y(Status *const status, const char *const set1, const char *const set2);

#endif /* OPERATIONS_H */
