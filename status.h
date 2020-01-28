#ifndef STATUS_H
#define STATUS_H

#define PATTERN_SIZE 1024
#define MAX_MATCHES 9

#include <stdbool.h>

typedef struct {
  char *pattern_space;
  char *hold_space;
  bool sub_success;
  int line_nb;
} Status;

#endif /* STATUS_H */
