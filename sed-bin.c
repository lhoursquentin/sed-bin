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
    .skip_read = false,
  };

  while (true) {
    if (status.skip_read) {
      status.skip_read = false;
    } else if (!read_pattern(&status, status.pattern_space, PATTERN_SIZE)) {
      break;
    }
    status.skip_read = false;
    // FIXME reset substitution success value
    #include "generated.c"
    puts(status.pattern_space);
  }
  return EXIT_SUCCESS;
}
