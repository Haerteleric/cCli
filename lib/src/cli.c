#if defined(CLI_STATIC_IMPLEMENTATION)
#error "if this template is used as a libary, function can not be static!"
#endif

#define ASCII_PARSER_STATIC_IMPLEMENTATION
#define ASCII_PRINTER_STATIC_IMPLEMENTATION 
#include "asciiParser_t.h"
#include "asciiPrinter_t.h"

#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
#undef CLI_ONLY_PROTOTYPE_DECLARATION
#endif
#include "cli_t.h" 