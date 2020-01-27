#include <stdbool.h>

#define PATTERN_SIZE 1024
#define MAX_MATCHES 9

typedef struct {
  char *pattern_space;
  char *hold_space;
  bool sub_success;
} Status;

void g(Status *status);
void h(Status *status);
void p(const Status *status);
bool s(Status *status, const char* pattern, const char* replace);
void x(Status *status);
