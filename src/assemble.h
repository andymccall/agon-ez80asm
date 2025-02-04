#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>
#include "console.h"
#include "globals.h"
#include "utils.h"
#include "label.h"
#include "str2num.h"
#include "listing.h"
#include "macro.h"
#include "io.h"
#include "moscalls.h"

enum {
    STATE_LINESTART,
    STATE_MNEMONIC,
    STATE_SUFFIX,
    STATE_OPERAND1,
    STATE_OPERAND2,
    STATE_ASM_ARG,
    STATE_COMMENT,
    STATE_DONE,
    STATE_MISSINGOPERAND,
    STATE_ASM_STRINGARG,
    STATE_ASM_STRINGARG_CLOSEOUT,
    STATE_ASM_VALUE_ENTRY,
    STATE_ASM_VALUE_EXIT,
    STATE_ASM_VALUE,
    STATE_ASM_PARSE,
    STATE_ASM_VALUE_CLOSEOUT,
};


typedef enum {
    PS_START,
    PS_LABEL,
    PS_COMMAND,
    PS_OP1,
    PS_OP2,
    PS_COMMENT,
    PS_DONE,
    PS_ERROR
} parsestate;

struct contentitem {
    // Static items
    char         *name;
    unsigned int  size;
    char         *buffer;
    FILE         *fh;
    bool          filebuffered;
    void         *next;
    // Items changed during processing
    char         *readptr;
    uint16_t      currentlinenumber;
    char         *currentline;
    char         *currenterrorline;
    char          labelscope[MAXNAMELENGTH+1];
    uint8_t       inConditionalSection;
};

extern uint24_t passmatchcounter;
bool assemble(char *filename);
void emit_8bit(uint8_t value);
void emit_16bit(uint16_t value);
void emit_24bit(uint24_t value);

struct contentitem *contentPop(void);
bool                contentPush(struct contentitem *ci);
struct contentitem *currentContent(void);
uint8_t             currentStackLevel(void);

#endif // ASSEMBLE_H