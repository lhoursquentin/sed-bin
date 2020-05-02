#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "address.h"
#include "operations.h"
#include "read.h"
#include "status.h"

static FILE *open_file(
  const char **const open_file_paths,
  FILE **const open_file_handles,
  const char *const filepath
) {
  size_t i;
  for (i = 0; open_file_paths[i]; ++i) {
    if (open_file_paths[i] == filepath) {
      return open_file_handles[i];
    }
    // No opened file and maxed out opened file capacity
    assert(i < MAX_WFILES);
  }
  open_file_paths[i] = filepath;
  FILE *const file_handle = fopen(filepath, "w");
  assert(file_handle);
  open_file_handles[i] = file_handle;
  return file_handle;
}

int main(int argc, char **argv) {
  Status status = {
    .pattern_space = (char[PATTERN_SIZE]){},
    .hold_space = (char[PATTERN_SIZE]){},
    .sub_success = false,
    .line_nb = 0,
    .last_line_nb = UINT_MAX,
    .skip_read = false,
    .last_regex = NULL,
    .range_ids = (size_t [MAX_ACTIVE_RANGES]){},
    .suppressed_range_ids = (size_t [MAX_ACTIVE_RANGES]){},
    .pending_outputs = (Pending_output[MAX_PENDING_OUTPUT]){},
    .pending_output_counter = 0,
    .next_line = (char[PATTERN_SIZE]){},
    .last_line_addr_present = false,
  };

  const char *open_file_paths[MAX_WFILES];
  FILE *open_file_handles[MAX_WFILES];

  #include "generated-init.c"

  while (true) {
    if (status.skip_read) {
      status.skip_read = false;
    } else if (!read_pattern(&status, status.pattern_space, PATTERN_SIZE)) {
      break;
    }
    status.skip_read = false;
    #include "generated.c"
    puts(status.pattern_space);
  }
  return EXIT_SUCCESS;
}
