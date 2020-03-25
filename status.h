#ifndef STATUS_H
#define STATUS_H

#define PATTERN_SIZE 1024
#define MAX_MATCHES 9

typedef enum {
  CONTINUE,
  BREAK
} operation_ret;

#include <stdbool.h>

typedef struct {
  char *pattern_space;
  char *hold_space;
  bool sub_success;
  unsigned int line_nb;
  bool skip_read;
  const char *last_pattern;
} Status;

#endif /* STATUS_H */
