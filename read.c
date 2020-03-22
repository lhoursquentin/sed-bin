#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "status.h"

bool read_pattern(Status *status, char *buf, int size) {
  if (!fgets(buf, size, stdin)) {
    return 0;
  }
  status->line_nb++;
  int read_len = strlen(buf);
  if (read_len && buf[read_len - 1] == '\n') {
    buf[read_len - 1] = 0;
  }
  return 1;
}
