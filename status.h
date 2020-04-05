#ifndef STATUS_H
#define STATUS_H

#define PATTERN_SIZE 1024
#define MAX_MATCHES 9
#define MAX_ACTIVE_RANGES 100
#define MAX_PENDING_OUTPUT 100

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
  unsigned int last_line_nb;
  bool skip_read;
  const char *last_pattern;
  int *const range_ids;
  int *const suppressed_range_ids;
  const char **const pending_output;
  int pending_output_counter;
  char *const next_line;
} Status;

#endif /* STATUS_H */
