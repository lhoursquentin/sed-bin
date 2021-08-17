#include <stdio.h>
#include <string.h>

#include "status.h"

#define READ_CHUNK_SIZE 512

static void handle_pending_ouput(Status *const status) {
  for (size_t i = 0; i < status->pending_output_counter; ++i) {
    const Pending_output p = status->pending_outputs[i];
    if (p.is_filepath) {
      FILE *const f = fopen(p.filepath, "r");
      if (!f) {
        continue;
      }
      char *read_chunk[READ_CHUNK_SIZE];
      size_t nread;
      while ((nread = fread(read_chunk, 1, READ_CHUNK_SIZE, f)) > 0) {
        fwrite(read_chunk, 1, nread, stdout);
      }
      fclose(f);
    } else {
      puts(p.direct_output);
    }
  }
  status->pending_output_counter = 0;
}

ssize_t read_pattern(Status *const status, char *const buf, const size_t size) {
  handle_pending_ouput(status);

  if (status->last_line_addr_present &&
      status->line_nb > 0 &&
      status->line_nb == status->last_line_nb) {
    return -1;
  }

  ssize_t read_len;
  if (!status->last_line_addr_present || status->line_nb == 0) {
    if (!fgets(buf, size, stdin)) {
      return -1;
    }
    read_len = strlen(buf);
  } else {
    read_len = status->next_line.length;
    if (read_len) {
      memcpy(buf, status->next_line.str, read_len);
    }
  }

  status->sub_success = false;
  status->line_nb++;

  // try to read the next line, if we fail then that means that the current line
  // is the last one
  if (status->last_line_addr_present) {
    if (fgets(status->next_line.str, size, stdin)) {
      status->next_line.length = strlen(status->next_line.str);
    } else {
      status->last_line_nb = status->line_nb;
    }
  }

  // fgets includes the newline, remove it
  if (read_len && buf[read_len - 1] == '\n') {
    read_len--;
    buf[read_len] = 0;
  }
  return read_len;
}
