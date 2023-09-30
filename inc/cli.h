
#ifndef CLI_HEADER
#define CLI_HEADER

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


typedef size_t (* cliPrint_func)(const char * buffer, size_t len);
typedef void (* cliExec_func)(int argc, char const *argv[], cliPrint_func outputFunc);

typedef struct cliEntry_s
{
    const cliExec_func execFunction;
    const char *commandCallName;
    const char *commandHelpText;

    //Ignored while appending Command
    //Don't mess with this after calling cli_addCommand()
    struct cliEntry_s * next;
}cliEntry_t;

typedef struct cliInstance_s
{
    char *inputBuffer;
    size_t inputBufferFilledSize;
    const size_t inputBufferMaxSize;

    const char *promptMessage;
    bool localEcho;

    volatile bool actionPending;

    cliPrint_func printFunction;
    cliEntry_t *commandLinkedListRoot;
}cliInstance_t;

extern cliEntry_t defaultCliRootEntry; 

void cli_inputChar(cliInstance_t * instance, char inputChar);
void cli_tick(cliInstance_t * instance);
void cli_clear(cliInstance_t * instance);
void cli_addCommand(cliInstance_t * instance, cliEntry_t *command);
void cli_removeCommand(cliInstance_t * instance, cliEntry_t *command);

typedef enum cliArgumentType_e
{
    CLI_ARGUMENT_DEC_INT,
    CLI_ARGUMENT_DEC_UINT,
    CLI_ARGUMENT_BINARY_LITERAL,
    CLI_ARGUMENT_HEX_LITERAL,
    CLI_ARGUMENT_BYTE_ARRAY,
    CLI_ARGUMENT_UNDEFINED
}cliArgumentType_t;
#define CLI_ARGUMENT_STRING CLI_ARGUMENT_UNDEFINED

cliArgumentType_t cli_classifyArgumentType(const char * arg);
void    cli_putUnsignedDecimal(cliPrint_func output, unsigned int num);
void    cli_putUnsignedHex(cliPrint_func output, unsigned int num);
void    cli_putByteHex(cliPrint_func output, unsigned char byte);
void    cli_putNibbleHex(cliPrint_func output, unsigned char nibble);
unsigned int cli_getUnsignedDecimal(const char * arg);
unsigned int cli_getUnsignedHex(const char * arg);
int     cli_getSignedDecimal(const char * arg);
size_t  cli_getByteArraySize(const char * arg);
unsigned char cli_getByteHex(const char * arg);
size_t  cli_getByteArrayElements(const char * arg, unsigned char * buffer);
#endif
