#include "error.h"

void exit_illegal_state(int err_code, std::string_view msg) {
  printf("executable exited on illegal state err code (%i)\n\t%s", err_code, msg.data());
  exit(err_code);
}
void exit_illegal_state(std::string_view msg) { exit_illegal_state(-1, msg); }
