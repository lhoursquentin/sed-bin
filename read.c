#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "status.h"

bool read_pattern(Status *status) {
  if (!fgets(status->pattern_space, PATTERN_SIZE, stdin)) {
    return 0;
  }
  // TODO Dirty, multiple successive strlen on pattern_space, maybe I should
  // keep the length in Status and update it when needed?
  char *pattern_space = status->pattern_space;
  int pattern_space_len = strlen(pattern_space);
  if (pattern_space_len && pattern_space[pattern_space_len - 1] == '\n') {
    pattern_space[pattern_space_len - 1] = 0;
  }
  return 1;
}
