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
s(status, "^s", "s", 0);
if (status->sub_success) { status->sub_success = false; goto s_cleaning; }
start_replacing:
s(status, "^s\\(.\\)\\(.*\\)\\1\\(.*\\)\\1g", "s(status, \"\\2\", \"\\3\", S_OPT_G)", 0);
if (status->sub_success) { status->sub_success = false; goto add_semi; }
s(status, "^s\\(.\\)\\(.*\\)\\1\\(.*\\)\\1", "s(status, \"\\2\", \"\\3\", 0)", 0);
if (status->sub_success) { status->sub_success = false; goto add_semi; }
s(status, "^[ghGHpx]$", "&(status)", 0);
if (status->sub_success) { status->sub_success = false; goto add_semi; }
s(status, "^t \\(.*\\)", "if (status->sub_success) { status->sub_success = false; goto \\1; }", 0);
if (status->sub_success) { status->sub_success = false; goto end; }
s(status, "^b ", "goto ", 0);
if (status->sub_success) { status->sub_success = false; goto add_semi; }
s(status, "^:[[:blank:]]*\\(.*\\)", "\\1:", 0);
if (status->sub_success) { status->sub_success = false; goto end; }

add_semi:
s(status, "^[^#].*", "&;", 0);

goto end;

s_cleaning:

s(status, "\\\\", "\\\\\\\\", S_OPT_G);
s(status, "\"", "\\\\\"", S_OPT_G);
if (status->sub_success) { status->sub_success = false; goto start_replacing; }
goto start_replacing;

end:
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
    // tests: s(&status, argv[1], argv[2], (argc > 3 && argv[3][0] == 'g') ? S_OPT_G : 0);
    sed_script(&status);
    puts(status.pattern_space);
  }
  return EXIT_SUCCESS;
}
