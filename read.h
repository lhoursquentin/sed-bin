#ifndef READ_H
#define READ_H

#include <stdbool.h>

#include "status.h"

bool read_pattern(Status *status, char *buf, int size);
#endif /* READ_H */
