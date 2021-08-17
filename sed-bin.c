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
    .pattern_space = {
      .str = (char[PATTERN_SIZE]){0},
      .length = 0
    },
    .hold_space = {
      .str = (char[PATTERN_SIZE]){0},
      .length = 0
    },
    .sub_success = false,
    .line_nb = 0,
    .last_line_nb = UINT_MAX,
    .skip_read = false,
    .last_regex = NULL,
    .range_ids = (size_t [MAX_ACTIVE_RANGES]){0},
    .suppressed_range_ids = (size_t [MAX_ACTIVE_RANGES]){0},
    .pending_outputs = (Pending_output[MAX_PENDING_OUTPUT]){{0}},
    .pending_output_counter = 0,
    .next_line = {
      .str = (char[PATTERN_SIZE]){0},
      .length = 0
    },
    .last_line_addr_present = false,
    .suppress_default_output = false,
  };

  if (argc > 1) {
    assert(strcmp(argv[1], "-n") == 0 && argc == 2);
    status.suppress_default_output = true;
  }

  const char *open_file_paths[MAX_WFILES] = {NULL};
  FILE *open_file_handles[MAX_WFILES] = {NULL};

  #include "generated-init.c"

  while (true) {
    if (status.skip_read) {
      status.skip_read = false;
    } else {
      size_t nb_chars_read = read_pattern(
        &status,
        status.pattern_space.str,
        PATTERN_SIZE
      );
      if (nb_chars_read == -1) {
        break;
      }
      status.pattern_space.length = nb_chars_read;
    }
    status.skip_read = false;
    #include "generated.c"
    if (!status.suppress_default_output) {
      p(&status);
    }
  }
  return EXIT_SUCCESS;
}
