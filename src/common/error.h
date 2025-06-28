#ifndef COMMON_ERROR_HEADER
#define COMMON_ERROR_HEADER
#include <string_view>
#include <stdlib.h>

void exit_illegal_state(int err_code, std::string_view msg);
void exit_illegal_state(std::string_view msg);

#endif
