#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "operations.h"
#include "read.h"
#include "status.h"

void sed_script(Status *status) {
  // Insert generated sed to c code here
}

int main(int argc, char **argv) {
  Status status = {
    (char[PATTERN_SIZE]){},
    (char[PATTERN_SIZE]){},
    false
  };

  while (read_pattern(&status)) {
    sed_script(&status);
    puts(status.pattern_space);
  }
  return EXIT_SUCCESS;
}
