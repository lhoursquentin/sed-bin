#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "operations.h"
#include "read.h"
#include "status.h"

void sed_script(Status *status) {
  // example for sed 'x;s/.*/space/;p;x'
  x(status);
  s(status, ".*", "space");
  p(status);
  x(status);
}

int main(int argc, char **argv) {
  Status status = {
    (char[PATTERN_SIZE]){},
    (char[PATTERN_SIZE]){},
    false
  };

  while (read_pattern(&status)) {
    // s(&status, argv[1], argv[2]); // for testing

    sed_script(&status);

    puts(status.pattern_space);
  }
  return EXIT_SUCCESS;
}
