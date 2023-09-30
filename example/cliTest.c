#include <stdio.h>
#include "cli.h"

static size_t cliPrintCallback(const char * buffer, size_t len)
{
    for (unsigned int i = 0; i < len; i++)
    {
        putchar(buffer[i]);
    }
    return 0;
}

char cliInputBuffer[128];
static cliInstance_t s_cliInstance =
{
    .commandLinkedListRoot = &defaultCliRootEntry,
    .inputBuffer = cliInputBuffer,
    .inputBufferMaxSize = sizeof(cliInputBuffer),
    .inputBufferFilledSize = 0,
    .localEcho = false,
    .actionPending = false,
    .promptMessage = "\n\r$> ",
    .printFunction = cliPrintCallback
};

static void printHelloWorld(int argc, char const *argv[], cliPrint_func outputFunc)
{
    outputFunc("hello world!\n", 13);
}
static void ping(int argc, char const *argv[], cliPrint_func outputFunc)
{
    outputFunc("pong!\n", 6);
}
static void argPrinter(int argc, char const *argv[], cliPrint_func outputFunc)
{
    for (size_t i = 0; i < argc; i++)
    {
        const char argPrefix[] = "Argument ";
        outputFunc(argPrefix,sizeof(argPrefix));
        outputFunc("[",1);
        char index = i + 48;
        outputFunc(&index,1);
        outputFunc("]: ",3);

        outputFunc(argv[i],strlen(argv[i]));

        outputFunc(" (",2);
        switch (cli_classifyArgumentType(argv[i]))
        {
        case CLI_ARGUMENT_BINARY_LITERAL:
            {
                const char typeMsg[] = "Binary Literal";
                outputFunc(typeMsg, sizeof(typeMsg));
            }break;

        case CLI_ARGUMENT_DEC_INT:
            {
                const char typeMsg[] = "Signed Decimal Integer";
                outputFunc(typeMsg, sizeof(typeMsg));
            }break;
        
        case CLI_ARGUMENT_DEC_UINT:
            {
                const char typeMsg[] = "Unsigned Decimal Integer";
                outputFunc(typeMsg, sizeof(typeMsg));
            }break;

        case CLI_ARGUMENT_HEX_LITERAL:
            {
                const char typeMsg[] = "Hexadecimal Literal";
                outputFunc(typeMsg, sizeof(typeMsg));
            }break;
        
        case CLI_ARGUMENT_BYTE_ARRAY:
            {
                const char typeMsg[] = "Hexadecimal Byte Array";
                outputFunc(typeMsg, sizeof(typeMsg));
            }break;

        case CLI_ARGUMENT_STRING:
            {
                const char typeMsg[] = "String";
                outputFunc(typeMsg, sizeof(typeMsg));
            }break;
        default:
            break;
        }        
        outputFunc(")",1);
        outputFunc("\n\r",2);
    }
    
}

static void printDec(int argc, char const *argv[], cliPrint_func outputFunc)
{
    if(!argc)
        return;

    cliArgumentType_t type = cli_classifyArgumentType(argv[0]);
    switch (type)
    {
        case CLI_ARGUMENT_DEC_INT:
        case CLI_ARGUMENT_DEC_UINT:
        {
            unsigned int temp = cli_getSignedDecimal(argv[0]);
            cli_putUnsignedDecimal(outputFunc, temp);
        }break;

        case CLI_ARGUMENT_HEX_LITERAL:
        {
            unsigned int temp = cli_getUnsignedHex(argv[0]);
            cli_putUnsignedDecimal(outputFunc, temp);
        }break;

        default:
            break;
    }
}

static void printHex(int argc, char const *argv[], cliPrint_func outputFunc)
{
    if(!argc)
        return;

    cliArgumentType_t type = cli_classifyArgumentType(argv[0]);
    switch (type)
    {
        case CLI_ARGUMENT_DEC_INT:
        case CLI_ARGUMENT_DEC_UINT:
        {
            unsigned int temp = cli_getSignedDecimal(argv[0]);
            cli_putUnsignedHex(outputFunc, temp);
        }break;

        case CLI_ARGUMENT_HEX_LITERAL:
        {
            unsigned int temp = cli_getUnsignedHex(argv[0]);
            cli_putUnsignedHex(outputFunc, temp);
        }break;

        default:
            break;
    }
}

static void arrayCounter(int argc, char const *argv[], cliPrint_func outputFunc)
{
    if(!argc)
        return;

    cliArgumentType_t type = cli_classifyArgumentType(argv[0]);
    switch (type)
    {
        case CLI_ARGUMENT_BYTE_ARRAY:
        {
            unsigned int temp = cli_getByteArraySize(argv[0]);
            unsigned char buffer[temp];
            cli_getByteArrayElements(argv[0], buffer);

            outputFunc("num elements: ", 14);
            cli_putUnsignedDecimal(outputFunc, temp);

            for (size_t i = 0; i < temp; i++)
            {
                outputFunc("\r\n [",4);
                cli_putUnsignedDecimal(outputFunc, i);
                outputFunc("]: ",3);
                cli_putByteHex(outputFunc, buffer[i]);
            }
            
        }break;

        default:
            break;
    }
}

cliEntry_t helloWorldEntry =
{
    .commandCallName= "helloworld",
    .commandHelpText= "prints a simple Hello World",
    .execFunction = printHelloWorld,
    .next = NULL
};
cliEntry_t pingEntry =
{
    .commandCallName= "ping",
    .commandHelpText= "prints a pong!",
    .execFunction = ping,
    .next = NULL
};
cliEntry_t argPrinterEntry =
{
    .commandCallName= "argprint",
    .commandHelpText= "prints out all given args",
    .execFunction = argPrinter,
    .next = NULL
};
cliEntry_t printHexEntry =
{
    .commandCallName= "printhex",
    .commandHelpText= "prints out a given argument as a hexadecimal Value",
    .execFunction = printHex,
    .next = NULL
};
cliEntry_t printDec2DecEntry =
{
    .commandCallName= "printdec",
    .commandHelpText= "prints out a given decimal argument as a decimal Value",
    .execFunction = printDec,
    .next = NULL
};
cliEntry_t arrayCounterEntry =
{
    .commandCallName= "cntarr",
    .commandHelpText= "prints out the number of elements in a given Byte Array",
    .execFunction = arrayCounter,
    .next = NULL
};
int main(int argc, char const *argv[])
{
    cli_addCommand(&s_cliInstance, &helloWorldEntry);
    cli_addCommand(&s_cliInstance, &pingEntry);
    cli_addCommand(&s_cliInstance, &argPrinterEntry);
    cli_addCommand(&s_cliInstance, &printHexEntry);
    cli_addCommand(&s_cliInstance, &printDec2DecEntry);
    cli_addCommand(&s_cliInstance, &arrayCounterEntry);
    cli_clear(&s_cliInstance);

    while (1)
    {
        char c = getchar();

        cli_inputChar(&s_cliInstance, c);
        cli_tick(&s_cliInstance);
    }
}
