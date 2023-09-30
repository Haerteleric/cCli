#include "cli.h"

static bool isValidBinNumber(const char * str, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if((str[i] > '1') || (str[i] < '0'))
        {
            return false;
        }
    }
    return true;
}

static bool isValidDecNumber(const char * str, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if((str[i] > '9') || (str[i] < '0'))
        {
            return false;
        }
    }
    return true;
}

static bool isValidHexNumber(const char * str, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if(
            ((str[i] <= '9') && (str[i] >= '0'))
            || ((str[i] <= 'F') && (str[i] >= 'A'))
            || ((str[i] <= 'f') && (str[i] >= 'a'))
        )
        {
            continue;
        }
        else 
        {
            return false;
        }
    }
    return true;
}

static size_t getElementsByteArray(const char * str, size_t len, unsigned char * buffer)
{
    size_t numNibblesPerByte = 0;
    size_t numElemets = 0;
    for (size_t i = 0; i < len; i++)
    {
        if(isValidHexNumber(&str[i],1))
        {
            if(numNibblesPerByte == 0)
            {
                if(buffer)
                {
                    buffer[numElemets] = cli_getByteHex(&str[i]);
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
    }
    return numElemets;
}

static char convertHex2UpperAlpha(char input)
{
    if(input >= 'a')
    {
        input -= 'a'-'A'; 
    }
    return input;
}

static unsigned char asciiHex2Unsigned(char input)
{
    input = convertHex2UpperAlpha(input);
    return input - (input >= 'A' ? ('A'- 0xA) : '0');
}

static char unsigned2AsciiHex(unsigned char input)
{
    return input + ( input >= 0xA ? ('A' - 0xA) : '0');
}

static unsigned int getArguments(char * inputBuffer, size_t length, char ** argumentsVector)
{
    unsigned numArguments = 0;
    bool argumentStarted = false;
    bool escaped = false;

    for (size_t i = 0; i < length; i++)
    {
        switch (inputBuffer[i])
        {
            case '\'':
            case '\"':
            {
                escaped = !escaped;
                if(argumentsVector)
                {
                    inputBuffer[i] = '\0'; //remove escape chars
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

void cli_inputChar(cliInstance_t * instance, char inputChar)
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

void cli_tick(cliInstance_t * instance)
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

void cli_clear(cliInstance_t * instance)
{
    instance->inputBufferFilledSize = 0;
    instance->actionPending = true;
}

void cli_addCommand(cliInstance_t * instance, cliEntry_t *command)
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

void cli_removeCommand(cliInstance_t * instance, cliEntry_t *command)
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

void cli_putNibbleHex(cliPrint_func output, unsigned char nibble)
{
    char nibbleAscii = unsigned2AsciiHex(nibble);
    output(&nibbleAscii,1);
}

void cli_putByteHex(cliPrint_func output, unsigned char byte)
{
    cli_putNibbleHex(output, (byte & 0b11110000) >> 4);
    cli_putNibbleHex(output, byte & 0b00001111);
}

void cli_putUnsignedDecimal(cliPrint_func output, unsigned int num)
{
    unsigned int decMask = 1000000000; // 4,294,967,295 is unsigned int Max (2^32)
    const char asciZero = '0';
    char currentDigit = 0;
    bool wordStarted = false;

    while (decMask)
    {
        currentDigit = num / decMask;
        num -=  decMask * currentDigit;
        
        if(wordStarted || (currentDigit > 0))
        {
            wordStarted = true;
            currentDigit += asciZero;
            output(&currentDigit, 1);
        }

        decMask /= 10;
    }

    if(!wordStarted)
    {
        output("0", 1);
    }
}

void cli_putUnsignedHex(cliPrint_func output, unsigned int num)
{
    output("0x", 2);

    bool wordStarted = false;
    const unsigned int mostSiginificantNibble = 0xF << (((sizeof(unsigned int)*2)-1)*4);
    
    for (size_t i = 0; i < sizeof(unsigned int)*2; i++)
    {
        
        char nibble = ((num & mostSiginificantNibble) >> (((sizeof(unsigned int)*2)-1)*4));
        if( wordStarted || (nibble > 0) )
        {
            wordStarted = true;
            cli_putNibbleHex(output,nibble);
        }
        num <<= 4;
    }
    
    if(!wordStarted)
    {
        output("0", 1);
    }
}

unsigned char cli_getByteHex(const char *arg)
{  
    unsigned char result = asciiHex2Unsigned(*arg);
    if(isValidHexNumber(++arg,1))
    {
        result <<= 4;
        result += asciiHex2Unsigned(*arg);
    }
    return result;
}

unsigned int cli_getUnsignedHex(const char * arg)
{
    arg += 2; //skip 0x Prefix
    unsigned int result = 0;
    for (size_t i = 0; i < sizeof(unsigned int)*2; i++)
    {
        if(arg[i] == '\0')
           break;

        result <<= 4;
        result |= asciiHex2Unsigned(arg[i]);
    }
    return result;
}

unsigned int cli_getUnsignedDecimal(const char * arg)
{
    unsigned int decMask = 1000000000; // 4,294,967,295 is unsigned int Max (2^32)
    unsigned int result = 0;
    char currentDigit = 0;

    while (decMask)
    {
        currentDigit = *(arg++);

        if(currentDigit == '\0')
            break;

        result *=10;

        //Offset ASCI
        currentDigit -= '0';

        result += currentDigit;
    }
    return result;
}

int cli_getSignedDecimal(const char * arg)
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
  return signum * cli_getUnsignedDecimal(arg);
}

size_t cli_getByteArraySize(const char * arg)
{
    return getElementsByteArray(arg+1, strlen(arg)-2, NULL);
}

size_t cli_getByteArrayElements(const char * arg, unsigned char * buffer)
{
    return getElementsByteArray(arg+1, strlen(arg)-2, buffer);
}

cliArgumentType_t cli_classifyArgumentType(const char * arg)
{
    if( arg[0] == '0' )
    {
        if(arg[1] == 'b')
        {
            if(isValidBinNumber(&arg[2],strlen(&arg[2])))
            {
                return CLI_ARGUMENT_BINARY_LITERAL;
            }
        }

        if(arg[1] == 'x')
        {
            if(isValidHexNumber(&arg[2],strlen(&arg[2])))
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
        if(isValidDecNumber(&arg[1],strlen(&arg[1])))
        {
            return CLI_ARGUMENT_DEC_INT;
        }
    }

    if (isValidDecNumber(arg,strlen(arg)))
    {
        return CLI_ARGUMENT_DEC_UINT;
    }

    return CLI_ARGUMENT_UNDEFINED;
}

/*--------------------------------------DEFAULT COMMAND--------------------------------------------------*/
/*-----------------------------THIS SHOULD BE LINKED LIST ROOT-------------------------------------------*/
static void printHelp(int argc, char const *argv[], cliPrint_func outputFunc);
//Must be root Entry to work properly
cliEntry_t defaultCliRootEntry =
{
    .commandCallName = "help",
    .execFunction = printHelp,
    .next = NULL
};
static void printHelp(int argc, char const *argv[], cliPrint_func outputFunc)
{
    cliEntry_t * entry = defaultCliRootEntry.next;
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

