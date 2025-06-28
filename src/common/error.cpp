#include "error.h"
#include <cstdlib>

void exit_illegal_state(int err_code, std::string_view msg) {
  std::abort();
}
void exit_illegal_state(std::string_view msg) { exit_illegal_state(-1, msg); }
