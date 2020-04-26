#ifndef READ_H
#define READ_H

#include <stdbool.h>

#include "status.h"

bool read_pattern(Status *const status, char *const buf, const size_t size);
#endif /* READ_H */
