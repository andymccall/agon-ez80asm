#include "globals.h"

/*
 * Global variables
 */
uint8_t errorreportlevel;
uint8_t maxstackdepth;
struct contentitem *currentcontentitem;
uint16_t sourcefilecount;
uint16_t binfilecount;
uint24_t filecontentsize;
bool filesbuffered;
unsigned int macrolinenumber;
unsigned int pass;
conditionalstate inConditionalSection;
macro_t *currentExpandedMacro;
uint24_t address;
uint16_t global_errors;
bool adlmode;
bool list_enabled;
bool consolelist_enabled;
uint8_t fillbyte;
uint24_t start_address;
bool coloroutput;
unsigned int labelcollisions;
bool ignore_truncation_warnings;
bool issue_warning;
uint24_t remaining_dsspaces;

tokenline_t currentline;

operand_t operand1;
operand_t operand2;
opcodesequence_t output;

char *message[] = {
    "Missing opening bracket in operand",
    "Missing closing bracket in operand",
    "Invalid number format",
    "Positive number required",
    "Unknown label",
    "Unknown label, invalid number",
    "Invalid label",
    "Missing operand",
    "Invalid mnemonic",
    "Invalid operand",
    "Operand(s) not matching mnemonic",
    "Error in opcode transformation",
    "Range error in immediate",
    "Invalid suffix",
    "Value truncated to 8 bit",
    "Value truncated to 16 bit",
    "Value truncated to 24 bit",
    "Suffix not matching mnemonic / ADL mode",
    "Index register offset exceeded",
    "String not terminated",
    "Invalid ADL mode",
    "Invalid assembler command",
    "New address lower than current PC address",
    "Address outside 16-bit range",
    "String format error",
    "Relative jump too large",
    "Invalid bit number (0-7)",
    "Illegal interrupt mode (0-2)",
    "Illegal restart address",
    "Value format error",
    "Missing label",
    "Wordsize larger than 16-bit while ADL set to 0",
    "Too many arguments",
    "Invalid list format",
    "Error creating label",
    "Label already defined",
    "Maximum number of local labels reached",
    "Unable to open include file",
    "Local label reference not allowed",
    "Invalid literal format",
    "Parse error",
    "Invalid operator",
    "Illegal unary opeator",
    "Argument is not a power of 2",
    "Maximum nested level of include files reached",
    "Recursive include detected",
    "Macro already defined",
    "Unfinished macro definition",
    "Macro start definition not found",
    "Invalid macro name",
    "Macro name too long",
    "Macro argument too long",
    "Maximum number of macro arguments reached",
    "Incorrect number of macro arguments",
    "Macro argument substitution length too long",
    "No global labels allowed in macro definition",
    "Error allocating memory for macro",
    "No macro definitions allowed inside a macro",
    "Calling macro from a macro",
    "Macro body larger than 2KB",
    "Illegal escape sequence in literal",
    "Input line too long",
    "Missing conditional expression",
    "Nested conditionals not supported",
    "Missing IF directive",
    "Missing ENDIF directive",
    "Internal error",
    "Illegal escape sequence in string",
    "String type not allowed",
    "Couldn't open file for writing global label table",
    "Error resetting input file(s)\r\n",
    "Error reading incbin file",
    "Error reading input file",
    "Error allocating memory",
    "Ignoring unsupported initializer value"
};
