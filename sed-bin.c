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
    .line_nb = 1,
  };

  while (read_pattern(&status)) {
    // FIXME reset substitution success value
    #include "generated.c"
    puts(status.pattern_space);
  }
  return EXIT_SUCCESS;
}
