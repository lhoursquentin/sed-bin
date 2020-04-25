#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "address.h"
#include "operations.h"
#include "read.h"
#include "status.h"

int main(int argc, char **argv) {
  Status status = {
    .pattern_space = (char[PATTERN_SIZE]){},
    .hold_space = (char[PATTERN_SIZE]){},
    .sub_success = false,
    .line_nb = 0,
    .last_line_nb = INT_MAX, // TODO UINT_MAX after cleaning up signed usage
    .skip_read = false,
    .last_regex = NULL,
    .range_ids = (int [MAX_ACTIVE_RANGES]){},
    .suppressed_range_ids = (int [MAX_ACTIVE_RANGES]){},
    .pending_outputs = (Pending_output[MAX_PENDING_OUTPUT]){},
    .pending_output_counter = 0,
    .next_line = (char[PATTERN_SIZE]){},
    .last_line_addr_present = false,
  };

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
