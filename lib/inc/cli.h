#if defined(CLI_STATIC_IMPLEMENTATION)
#error "if this template is used as a libary, function can not be static!"
#endif

#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
#include "cli_t.h"