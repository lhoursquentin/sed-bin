#ifndef STATUS_H
#define STATUS_H

#define PATTERN_SIZE 8192
#define MAX_MATCHES 9
#define MAX_ACTIVE_RANGES 100
#define MAX_PENDING_OUTPUT 100
#define MAX_WFILES 10

typedef enum {
  CONTINUE,
  BREAK
} operation_ret;

#include <stdbool.h>
#include <regex.h>

typedef struct {
  bool is_filepath;
  union {
    const char *direct_output; // resulting from a cmd
    const char *filepath; // resulting from r cmd
  };
} Pending_output;

typedef struct {
  bool compiled;
  union {
    const char *str;
    regex_t obj;
  };
} Regex;

typedef struct {
  char *pattern_space;
  char *hold_space;
  bool sub_success;
  size_t line_nb;
  size_t last_line_nb;
  bool skip_read;
  Regex *last_regex;
  size_t *const range_ids;
  size_t *const suppressed_range_ids;
  Pending_output *const pending_outputs;
  size_t pending_output_counter;
  char *const next_line;
  bool last_line_addr_present;
} Status;

#endif /* STATUS_H */
