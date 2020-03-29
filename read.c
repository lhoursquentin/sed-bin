#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "status.h"

bool read_pattern(Status *const status, char *const buf, const int size) {
  for (int i = 0; i < status->pending_output_counter; ++i) {
    puts(status->pending_output[i]);
  }
  status->pending_output_counter = 0;

  if (!fgets(buf, size, stdin)) {
    return 0;
  }
  status->sub_success = false;
  status->line_nb++;
  const int read_len = strlen(buf);
  if (read_len && buf[read_len - 1] == '\n') {
    buf[read_len - 1] = 0;
  }
  return 1;
}
