#include "operations.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sed_script(Status *status) {
  // generated c code from sed script parsing goes here
}

int main(int argc, char **argv) {
  if (argc < 2) {
    return 1;
  }

  Status status = {
    (char[PATTERN_SIZE]){},
    (char[PATTERN_SIZE]){},
    false
  };

  while (fgets(status.pattern_space, PATTERN_SIZE, stdin)) {
    // TODO Dirty, multiple successive strlen on pattern_space, maybe I should
    // keep the length in Status and update it when needed?
    // This should also be done within a function (fgets + newline removal), to
    // avoid that tmp pattern_space variable becoming potentially pointing to
    // the hold after performing an x operation.
    char *pattern_space = status.pattern_space;
    int pattern_space_len = strlen(pattern_space);
    if (pattern_space_len && pattern_space[pattern_space_len - 1] == '\n') {
      pattern_space[pattern_space_len - 1] = 0;
    }

    s(&status, argv[1], argv[2]); // for testing

    // sed_script(&status);

    // x(&status);
    // s(&status, ".*", "space");
    // p(&status);
    // x(&status);

    // pattern_space might be different than status.pattern_space at this point
    // due to the `x' operation swapping the hold with the pattern
    printf("%s\n", status.pattern_space);
  }
  return EXIT_SUCCESS;
}
