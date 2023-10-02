/**
 * Portable CLI template implementation
 * 
 * DEPENDS ON :
 *  cSuite/cAsciiParser : asciiParser.h
 *  cSuite/cAsciiPrinter : asciiPrinter.h
 * 
 * Author:    Haerteleric
 * 
 * MIT License
 * 
 * Copyright (c) 2023 Eric Härtel
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#include <stddef.h>
#include <stdbool.h>
#include <string.h>


#if !defined(_ASCII_PARSER_INCLUDED) || !defined(_ASCII_PRINTER_INCLUDED)
#error "this template depends on cAsciiParser.h & cAsciiPrinter.h include them before this template via extern or cSuite"
#endif

#ifndef _CLI_INCLUDED
#define _CLI_INCLUDED
#endif


typedef unsigned int (* cliPrint_func)(const char * buffer, unsigned  int len);
typedef void (* cliExec_func)(int argc, char const *argv[], cliPrint_func outputFunc);

#ifndef _CLI_ENTRY_STRUCT_DEFINED
#define _CLI_ENTRY_STRUCT_DEFINED

typedef struct cliEntry_s
{
    const cliExec_func execFunction;
    const char *commandCallName;
    const char *commandHelpText;

    //Ignored on init
    //Don't mess with this after calling cli_addCommand()
    struct cliEntry_s * next;
}cliEntry_t;

#endif //_CLI_ENTRY_STRUCT_DEFINED


#ifndef _CLI_INSTANCE_STRUCT_DEFINED
#define _CLI_INSTANCE_STRUCT_DEFINED

typedef struct cliInstance_s
{
    char *inputBuffer;
    unsigned  int inputBufferFilledSize;
    const unsigned  int inputBufferMaxSize;

    const char *promptMessage;
    bool localEcho;

    volatile bool actionPending;

    cliPrint_func printFunction;
    cliEntry_t *commandLinkedListRoot;
}cliInstance_t;

#endif //_CLI_INSTANCE_STRUCT_DEFINED

#ifndef _CLI_ARG_TYPE_ENUM_DEFINED
#define _CLI_ARG_TYPE_ENUM_DEFINED

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

#endif //_CLI_ARG_TYPE_ENUM_DEFINED

extern cliEntry_t defaultCliRootEntry; 

#ifndef CLI_ONLY_PROTOTYPE_DECLARATION
//INTERNAL STATIC SECTION
//should not be included into Prototype include
//As such they are always Static

inline static unsigned int getArguments(char * inputBuffer, unsigned int length, char ** argumentsVector)
{
    unsigned numArguments = 0;
    bool argumentStarted = false;
    bool escaped = false;

    for (unsigned int i = 0; i < length; i++)
    {
        switch (inputBuffer[i])
        {
            case '\'':
            case '\"':
            {
                escaped = !escaped;

                //check if the argument Vector is being created
                if(argumentsVector) 
                {
                    //remove escape chars if so
                    inputBuffer[i] = '\0'; 
                }

            }break;

            case ' ':
            case '\0':
            {
                if (!escaped)
                {
                    argumentStarted = false;
                    if(argumentsVector)
                    {
                        inputBuffer[i] = '\0'; //terminate the argument strings
                    }
                }
            }break;

            default:
            {
                if(!argumentStarted)
                {
                    argumentStarted = true;
                    if(argumentsVector)
                    {
                        argumentsVector[numArguments] = (char *) &inputBuffer[i];
                    }
                    numArguments++;
                }
            }
            break;
        }
    }

    if(escaped)
    {
        inputBuffer[length-1] = '\0';
    }

    return numArguments;
}

static unsigned int getElementsByteArray(const char * str, unsigned char * buffer)
{
    unsigned int numNibblesPerByte = 0;
    unsigned int numSpacerChars = 0;
    unsigned int numElemets = 0;

    if(*(str++) != '{')
    {
        return 0;
    }

    while((*str) != '}')
    {
        if(ascii_isValidHexadecimalChar(*str))
        {
            if(numNibblesPerByte == 0)
            {
                if(buffer)
                {
                    buffer[numElemets] = ascii_parseByteHex(str);
                }
                numElemets++;
            }

            if(++numNibblesPerByte > 2)
            {
                return 0;
            }
        }
        else if(numNibblesPerByte == 0)
        {
            return 0;
        }
        else
        {
            numNibblesPerByte = 0;
        }

        str++;
    }
    return numElemets;
}

#endif// INTERNAL STATIC SECTION

#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
void cli_inputChar(cliInstance_t * instance, char inputChar)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    switch (inputChar)
    {
        //Check if Return got hit
        case '\n':
        case '\r':
        {
            instance->actionPending = true;
        } break;

        default:
        {
            if(!instance->actionPending && (instance->inputBufferFilledSize < (instance->inputBufferMaxSize - 1))) //always needs space for trailing \0
            {
                instance->inputBuffer[instance->inputBufferFilledSize++] = inputChar;
                
                if(instance->localEcho)
                {
                    instance->printFunction(&inputChar, 1);
                }
            }
        } break;
    }
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)



#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
void cli_tick(cliInstance_t * instance)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    //check if there is data to be parsed
    if(instance->actionPending)
    {
        //add a string termination for the argument parser
        instance->inputBuffer[instance->inputBufferFilledSize++] = '\0';
        
        //check the amount of Arguments
        unsigned int numArguments = getArguments(
            instance->inputBuffer, 
            instance->inputBufferFilledSize, 
            NULL
        );

        //alloc Argument array in Stack
        char * argumentsVector[numArguments];
        //fill Argument Array
        getArguments(
            instance->inputBuffer,
            instance->inputBufferFilledSize, 
            argumentsVector
        );

        //loop through the command linked list till one or none matches
        cliEntry_t * command = instance->commandLinkedListRoot;
        while(command)
        {
            int commandLength = strlen(command->commandCallName);
            
            //Argument 0 holds the given command call
            if( strncmp(argumentsVector[0], command->commandCallName, commandLength) == 0)
            {
                //Found Matching Command
                
                if(instance->localEcho)
                {
                    //Lr-Cr before exec
                    instance->printFunction("\n\r",2);
                } 
                
                //Exec Command
                command->execFunction(
                    (numArguments-1), //command Call-Name is not needed inside the Handler
                    (numArguments > 1 ? (const char **) &argumentsVector[1] : NULL), 
                    instance->printFunction
                );
                command = NULL;
            }
            else
            {
                //Go to next Command in list
                command = (command->next != command ? command->next : NULL);
            }
        }

        //reset Buffer
        instance->inputBufferFilledSize = 0;
        instance->actionPending = false;

        if(instance->promptMessage)
        {
            instance->printFunction(instance->promptMessage, strlen(instance->promptMessage));
        }
    }
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)



#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
void cli_clear(cliInstance_t * instance)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    instance->inputBufferFilledSize = 0;
    instance->actionPending = true;
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)


#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
void cli_addCommand(cliInstance_t * instance, cliEntry_t *command)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    cliEntry_t * lastCommand = instance->commandLinkedListRoot;

    if(lastCommand == NULL)
    {
            instance->commandLinkedListRoot = command;
    }
    else
    {
        //find last command of linked List
        while ( ( lastCommand->next != lastCommand ) && (lastCommand->next != NULL) )
        {
            lastCommand = lastCommand->next;
        }
        //Append command to linked List
        lastCommand->next = command;
    }

    //mark command as new end of linked List
    command->next = command;
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)


#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
void cli_removeCommand(cliInstance_t * instance, cliEntry_t *command)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    //check if there is something to remove
    if(instance->commandLinkedListRoot == NULL)
    {
        return;
    }

    //check if the command to be removed is the current root command
    if (instance->commandLinkedListRoot == command)
    {
        //check if command is the only command in linked List
        if(command->next == command)
        {
            //mark Instance as empty
            instance->commandLinkedListRoot = NULL;   
        }
        else
        {
            //make the next command the new root command
            instance->commandLinkedListRoot = command->next;   
        }
    }
    else
    {
        //find previous Entry in Linked List
        cliEntry_t * previousEntry = instance->commandLinkedListRoot;
        while ( previousEntry->next != command)
        {
            previousEntry = previousEntry->next;
        }
        
        //remove command from linked List
        previousEntry->next = command->next;
    }

    //Mark Command as not at part of a linked List 
    command->next = NULL; 
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)


#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
cliArgumentType_t cli_classifyArgumentType(const char * arg)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    if( arg[0] == '0' )
    {
        if(arg[1] == 'b')
        {
            if(ascii_onlyContainsValidBinaryChars(&arg[2]))
            {
                return CLI_ARGUMENT_BINARY_LITERAL;
            }
        }

        if(arg[1] == 'x')
        {
            if(ascii_onlyContainsValidHexadecimalChars(&arg[2]))
            {
                return CLI_ARGUMENT_HEX_LITERAL;
            }
        }
    }

    if( arg[0] == '{' )
    {
        if(arg[strlen(arg)-1] == '}')
        {
            return CLI_ARGUMENT_BYTE_ARRAY;
        }
    }

    if( ( arg[0] == '-' ) || ( arg[0] == '+' ) )
    {
        if(ascii_onlyContainsValidDecimalChars(&arg[1])) 
        {
            return CLI_ARGUMENT_DEC_INT;
        }
    }

    if (ascii_onlyContainsValidDecimalChars(arg))
    {
        return CLI_ARGUMENT_DEC_UINT;
    }

    return CLI_ARGUMENT_UNDEFINED;
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)


#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
void cli_putUnsignedHex(cliPrint_func output, unsigned int num)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
#ifndef CLI_NO_HEX_PREFIX_OUTPUT
    output("0x", 2);
#endif

    ascii_putHexLittleEndian((write_func)output,&num, sizeof(num));
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)


#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
void cli_putByteHex(cliPrint_func output, unsigned char byte)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    ascii_putByteHex((write_func)output, byte);
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)

#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
void    cli_putNibbleHex(cliPrint_func output, unsigned char nibble)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    ascii_putNibbleHex((write_func)output, nibble);
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)


#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
void    cli_putUnsignedDecimal(cliPrint_func output, unsigned int num)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    ascii_putUnsignedDecimal((write_func)output, num);
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)


#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
unsigned int cli_getUnsignedHex(const char * arg)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    return ascii_parseUnsignedHex(arg+2); // skip 0x Prefix
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)


#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
unsigned int cli_getUnsignedDecimal(const char * arg)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    return ascii_parseUnsignedDecimal(arg);
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)


#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
int cli_getSignedDecimal(const char * arg)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    int signum = 1;
    if(*arg == '-')
    {
        signum = -1;
        arg++;
    }
    if(*arg == '+')
    {
        arg++;
    }
    return signum * ascii_parseUnsignedDecimal(arg);
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)




#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
unsigned char cli_getByteHex(const char * arg)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    return ascii_parseByteHex(arg);
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)




#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
unsigned  int  cli_getByteArraySize(const char * arg)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    return getElementsByteArray(arg, NULL);
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)




#ifdef CLI_INLINE_IMPLEMENTATION
inline
#endif 
#ifdef CLI_STATIC_IMPLEMENTATION
static
#endif 
unsigned  int  cli_getByteArrayElements(const char * arg, unsigned char * buffer)
#ifdef CLI_ONLY_PROTOTYPE_DECLARATION
;
#else
{
    return getElementsByteArray(arg, buffer);
}
#endif // NOT(CLI_ONLY_PROTOTYPE_DECLARATION)



#if !defined(CLI_ONLY_PROTOTYPE_DECLARATION) && defined(CLI_IMPLEMENT_HELP_FUNC_COMMAND)
/*--------------------------------------DEFAULT COMMAND--------------------------------------------------*/
/*-----------------------------THIS SHOULD BE LINKED LIST ROOT-------------------------------------------*/
static void printHelp(int argc, char const *argv[], cliPrint_func outputFunc);
//Must be root Entry to work properly
cliEntry_t rootHelpEntry =
{
    .commandCallName = "help",
    .execFunction = printHelp,
    .next = NULL
};
static void printHelp(int argc, char const *argv[], cliPrint_func outputFunc)
{
    cliEntry_t * entry = rootHelpEntry.next;
    while (entry)
    {
        //Command
        outputFunc("[",1);
        outputFunc(entry->commandCallName,strlen(entry->commandCallName));
        outputFunc("]",1);
        outputFunc("\r\n", 2);
        //Help text
        outputFunc(entry->commandHelpText,strlen(entry->commandHelpText));
        outputFunc("\r\n", 2);
        outputFunc("\r\n", 2);

        //Goto next command
        entry = ( entry->next != entry ? entry->next : NULL );
    } 
}
#endif