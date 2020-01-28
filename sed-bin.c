#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "address.h"
#include "operations.h"
#include "read.h"
#include "status.h"

void sed_script(Status *status) {
  // Insert generated sed to c code here
  return;
}

int main(int argc, char **argv) {
  Status status = {
    .pattern_space = (char[PATTERN_SIZE]){},
    .hold_space = (char[PATTERN_SIZE]){},
    .sub_success = false,
    .line_nb = 1,
  };

  while (read_pattern(&status)) {
    sed_script(&status);
    puts(status.pattern_space);
  }
  return EXIT_SUCCESS;
}
