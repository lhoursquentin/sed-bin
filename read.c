#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "status.h"

bool read_pattern(Status *const status, char *const buf, const int size) {
  for (int i = 0; i < status->pending_output_counter; ++i) {
    puts(status->pending_output[i]);
  }
  status->pending_output_counter = 0;

  if (status->line_nb > 0 && status->line_nb == status->last_line_nb) {
    return 0;
  }

  int read_len;
  if (status->line_nb == 0) {
    if (!fgets(buf, size, stdin)) {
      return 0;
    }
    read_len = strlen(buf);
  } else {
    read_len = strlen(status->next_line);
    if (read_len) {
      memcpy(buf, status->next_line, read_len);
    }
  }

  status->sub_success = false;
  status->line_nb++;

  // try to read the next line, if we fail then that means that the current line
  // is the last one
  if (!fgets(status->next_line, size, stdin)) {
    status->last_line_nb = status->line_nb;
  }

  // fgets includes the newline, remove it
  if (read_len && buf[read_len - 1] == '\n') {
    buf[read_len - 1] = 0;
  }
  return 1;
}
