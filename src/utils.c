#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "globals.h"
#include "console.h"
#include "utils.h"
#include "str2num.h"
#include "label.h"
#include "instruction.h"
#include "io.h"
#include "assemble.h"
#include <stdarg.h>

// memory allocate size bytes, raise error if not available
void *allocateMemory(size_t size) {
    void *ptr = malloc(size);
    if(ptr == NULL) {
        error(message[ERROR_MEMORY],0);
    }
    return ptr;
}

// memory allocate a string, copy content and return pointer, or NULL if no memory
char *allocateString(char *name) {
    char *ptr = (char *)allocateMemory(strlen(name) + 1);
    if(ptr) {
        strcpy(ptr, name);
    }
    return ptr;
}

// return a base filename, stripping the given extension from it
void remove_ext (char* myStr, char extSep, char pathSep) {
    char *lastExt, *lastPath;
    // Error checks.
    if (myStr == NULL) return;
    // Find the relevant characters.
    lastExt = strrchr (myStr, extSep);
    lastPath = (pathSep == 0) ? NULL : strrchr (myStr, pathSep);
    // If it has an extension separator.
    if (lastExt != NULL) {
        // and it's to the right of the path separator.
        if (lastPath != NULL) {
            if (lastPath < lastExt) {
                // then remove it.
                *lastExt = '\0';
            }
        } else {
            // Has extension separator with no path separator.
            *lastExt = '\0';
        }
    }
}

void displayerror(const char *msg, const char *context, uint8_t level) {
    struct contentitem *ci = currentContent();

    if((global_errors == 1) || (level == LEVEL_WARNING)) {
        if(ci) {
            if(currentExpandedMacro) {
                printf("Macro [%s] in \"%s\" line %d - ",currentExpandedMacro->name, currentExpandedMacro->originfilename, currentExpandedMacro->originlinenumber+macrolinenumber);
            }
            else {
                printf("File \"%s\" line %d - ", ci->name, ci->currentlinenumber);
            }
        }
        printf("%s", msg);
        if(strlen(context)) {
            if(level == LEVEL_WARNING)
                vdp_set_text_colour(BRIGHT_WHITE);
            else
                vdp_set_text_colour(DARK_YELLOW);
            printf(" \'%s\'", context);
        }
        printf("\r\n");
    }
    vdp_set_text_colour(BRIGHT_WHITE);
}

void error(const char *msg, const char *contextformat, ...) {
    char context[LINEMAX+1];

    if(contextformat) {
        va_list args;
        va_start(args, contextformat);
        vsprintf(context, contextformat, args);
        va_end(args);
    }
    else context[0] = 0;

    vdp_set_text_colour(DARK_RED);
    global_errors++;
    errorreportlevel = currentStackLevel();

    displayerror(msg, context, LEVEL_ERROR);
}

void warning(const char *msg, const char *contextformat, ...) {
    char context[LINEMAX+1];

    if(contextformat) {
        va_list args;
        va_start(args, contextformat);
        vsprintf(context, contextformat, args);
        va_end(args);
    }
    else context[0] = 0;

    vdp_set_text_colour(DARK_YELLOW);
    issue_warning = true;

    displayerror(msg, context, LEVEL_WARNING);
}

void trimRight(char *str) {
    while(*str) str++;
    str--;
    while(isspace(*str)) str--;
    str++;
    *str = 0;
}

typedef enum {
    TOKEN_REGULAR,
    TOKEN_STRING,
    TOKEN_LITERAL,
    TOKEN_BRACKET
} tokenclass;

void getLabelToken(streamtoken_t *token, char *src) {
    token->start = src; // no need to remove leading spaces
    while(*src && (*src != ':') && (*src != ';')) src++;
    token->terminator = *src;
    token->next = src+1;
    *src = 0;

    return;
}
// fill the streamtoken_t object, according to the stream
// returns the number of Mnemonic characters found, or 0 if none
uint8_t getMnemonicToken(streamtoken_t *token, char *src) {
    uint8_t length = 0;

    // skip leading space
    while(*src && (isspace(*src))) src++;
    if(*src == 0) {
        memset(token, 0, sizeof(streamtoken_t));
        return 0;
    }
    token->start = src;
    while(!isspace(*src) && (*src != ';') && *src) {
        length++;
        src++;
    }
    token->terminator = *src;
    if(*src) token->next = src+1;
    else token->next = NULL;

    *src = 0; // terminate stream
    return length;
}

// point to one position after regular token, or to '0' in string
char * _findRegularTokenEnd(char *src) {
    while(*src) {
        if((*src == ';') || (*src == ',')) break;
        src++;
    }
    return src;
}

// point to one position after string token, or to '0' in the string
char * _findStringTokenEnd(char *src) {
    bool escaped = false;
    while(*src) {
        if(*src == '\\') escaped = !escaped;
        if((*src == '\"') && !escaped) break;
        src++;
    }
    if(*src) return src;
    else return src+1;
}
// point to one position after literal token, or to '0' in the string
char * _findLiteralTokenEnd(char *src) {
    bool escaped = false;
    while(*src) {
        if(*src == '\'') escaped = !escaped;
        if((*src == '\'') && !escaped) break;        
        src++;
    }
    if(*src) return src;
    else return src+1;
}


// fill the streamtoken_t object, parse it as an operand
// returns the number of Operator characters found, or 0 if none
uint8_t getOperandToken(streamtoken_t *token, char *src) {
    uint8_t length = 0;
    bool escaped = false;

    // skip leading space
    while(*src && (isspace(*src))) src++;
    if(*src == 0) {
        memset(token, 0, sizeof(streamtoken_t));
        return 0;
    }
    token->start = src;

    // hunt for end-character (0 , or ; in normal non-literal mode)
    while(*src) {
        if(*src == '\'') escaped = !escaped;
        if(!escaped && ((*src == ',') || (*src == ';'))) break;
        src++;
        length++;
    }

    token->terminator = *src;
    if(*src) token->next = src+1;
    else token->next = NULL;

    if(length) {
        *src-- = 0; // terminate early and revert one character
        while(isspace(*src)) { // remove trailing space(s)
            *src-- = 0; // terminate on trailing spaces
            if(length-- == 0) break;
        }
    }
    return length;
}

uint8_t getDefineValueToken(streamtoken_t *token, char *src) {
    uint8_t length = 0;
    tokenclass state;
    bool escaped = false;
    bool terminated;

    // skip leading space
    while(*src && (isspace(*src))) src++;
    if(*src == 0) {
        memset(token, 0, sizeof(streamtoken_t));
        return 0;
    }
    token->start = src;

    switch(*src) {
        case '\"':
            state = TOKEN_STRING;
            src++;
            length++;
            break;
        case '\'':
            state = TOKEN_LITERAL;
            src++;
            length++;
            break;
        case '(':
            state = TOKEN_BRACKET;
            src++;
            length++;
            break;
        default:
            state = TOKEN_REGULAR;
    }

    while(*src) {
        terminated = false;
        switch(state) {
            case TOKEN_STRING:
                switch(*src) {
                    case '\\':
                        escaped = !escaped;
                        break;
                    case '\"':
                        if(!escaped) state = TOKEN_REGULAR;
                        escaped = false;
                        break;
                    default:
                        escaped = false;
                        break;
                }
                break;
            case TOKEN_LITERAL:
                switch(*src) {
                    case '\\':
                        escaped = !escaped;
                        break;
                    case '\'':
                        if(!escaped) state = TOKEN_REGULAR;
                        escaped = false;
                        break;
                    default:
                        escaped = false;
                        break;
                }
                break;
            case TOKEN_BRACKET:
                if(*src == ')') state = TOKEN_REGULAR;
                break;
            case TOKEN_REGULAR:
                terminated = ((*src == ';') || (*src == ',') || (*src == '='));
                break;            
        }
        if(terminated) break;
        src++;
        length++;
    }

    token->terminator = *src;
    if(*src) token->next = src+1;
    else token->next = NULL;

    if(length) {
        *src-- = 0; // terminate early and revert one character
        while(isspace(*src)) { // remove trailing space(s)
            *src-- = 0; // terminate on trailing spaces
            if(length-- == 0) break;
        }
    }
    if(state == TOKEN_STRING) error(message[ERROR_STRING_NOTTERMINATED],0);
    return length;
}

// operator tokens assume the following input
// - no ',' ';' terminators
// - only 'operator' terminators or a 0
uint8_t getOperatorToken(streamtoken_t *token, char *src) {
    uint8_t length = 0;
    bool normalmode = true;
    bool shift = false, error = false;

    // skip leading space
    while(*src && (isspace(*src))) src++;

    if(*src == 0) { // empty string
        memset(token, 0, sizeof(streamtoken_t));
        return 0;
    }
    token->start = src;

    // check for literal mode at start, tokens like '\n'
    if(*src == '\'') {
        src++;
        length++;
        normalmode = false;
    }

    while(*src) {
        if(*src == '\'') normalmode = !normalmode;
        if(normalmode && strchr("+-*<>&|^~/",*src)) { // terminator found
            if((*src == '<') || (*src == '>')) {                
                if(*(src+1) == *src) {
                    shift = true;
                }
                else {
                    error = true;
                }
            }
            break;
        }
        src++;
        length++;
    }

    if(!error) token->terminator = *src;
    else {
        token->terminator = '!';
    }
    if(*src) token->next = shift?src+2:src+1;
    else token->next = NULL;

    if(length) {
        *src-- = 0; // terminate early and revert one character
        while(isspace(*src)) { // remove trailing space(s)
            *src-- = 0; // terminate on trailing spaces
            if(length-- == 0) break;
        }
    }
    return length;
}

void validateRange8bit(int32_t value, const char *name) {
    if(!(ignore_truncation_warnings)) {
        if((value > 0xff) || (value < -128)) {
            warning(message[WARNING_TRUNCATED_8BIT],"%s",name);
        }
    }
}

void validateRange16bit(int32_t value, const char *name) {
    if(!(ignore_truncation_warnings)) {
        if((value > 0xffff) || (value < -32768)) {
            warning(message[WARNING_TRUNCATED_16BIT],"%s",name);
        }
    }
}

void validateRange24bit(int32_t value, const char *name) {
    if(!(ignore_truncation_warnings)) {
        if((value > 0xffffff) || (value < -8388608)) {
            warning(message[WARNING_TRUNCATED_24BIT],"%s",name);
        }
    }
}

// Returns the value of an escaped character \c, or 255 if illegal
uint8_t getEscapedChar(char c) {
    switch(c) {
        case 'a':
            return(0x07); // Alert, beep
        case 'b':
            return(0x08); // Backspace
        case 'e':
            return(0x1b); // Escape
        case 'f':
            return(0x0c); // Formfeed
        case 'n':
            return(0x0a); // Newline
        case 'r':
            return(0x0d); // Carriage return
        case 't':
            return(0x09); // Horizontab tab
        case 'v':
            return(0x0b); // Vertical tab
        case '\\':
            return('\\'); // Backslash
        case '\'':
            return('\''); // Single quotation mark
        case '\"':
            return('\"'); // Double quotation mark
        case '?':
            return('?');  // Question mark
        default:
            return(0xff);
    }
}

// Get the ascii value from a single 'x' token.
uint8_t getLiteralValue(char *string) {
    uint8_t len = strlen(string);

    if((len == 3) && (string[2] == '\'')) {
        return string[1];
    }

    if((len == 4) && (string[3] == '\'')) {
        uint8_t c = getEscapedChar(string[2]);
        if(c == 0xff) {
            error(message[ERROR_ILLEGAL_ESCAPELITERAL],0);
            return 0;
        }
        return c;
    }

    error(message[ERROR_ASCIIFORMAT],0);
    return 0;
}

// Get the value from a sequence of 0-n labels and values, separated by +/- operators
// Examples:
// labela+5
// labelb-1
// labela+labelb+offset1-1
// The string should not contain any spaces, needs to be a single token
int32_t getValue(char *str, bool req_firstpass) {
    streamtoken_t token;
    label_t *lbl;
    int32_t total, tmp;
    uint8_t length;
    char prev_op = '+', unary = 0;
    bool expect = true;

    if((pass == 1) && !req_firstpass) return 0;

    total = 0;
    while(str) {
        length = getOperatorToken(&token, str);
        if(length == 0) { // at begin, or middle, OK. Expect catch at end
            expect = true;
            unary = token.terminator;
        }
        else { // normal processing
            lbl = findLabel(token.start);
            if(lbl) {
                tmp = lbl->address;
            }
            else {
                if(token.start[0] == '\'') tmp = getLiteralValue(token.start);
                else {
                    tmp = str2num(token.start, length);
                    if(err_str2num) {
                        if(pass == 1) {
                            // Yet unknown label, number incorrect
                            // We only get here if req_firstpass is true, so error
                            error(message[ERROR_INVALIDNUMBER],"%s",token.start);
                            return 0;
                        }
                        else {
                            // Unknown label and number incorrect
                            error(message[ERROR_INVALIDLABELORNUMBER], "%s", token.start);                            
                            return 0;
                        }
                    }
                }
            }
            if(unary) {
                switch(unary) {
                    case '-': tmp = -tmp; break;
                    case '~': tmp = ~tmp; break;
                    case '+': break;
                    default:
                        error(message[ERROR_UNARYOPERATOR],"%c",unary);
                        return 0;
                }
                unary = 0; // reset
                expect = false;
            }
            switch(prev_op) {
                case '+': total += tmp; break;
                case '-': total -= tmp; break;
                case '*': total *= tmp; break;
                case '<': total = total << tmp; break;
                case '>': total = total >> tmp; break;
                case '&': total = total & tmp;  break;
                case '|': total = total | tmp;  break;
                case '^': total = total ^ tmp;  break;
                case '~': total = total + ~tmp; break;
                case '/': total = total / tmp;  break;
                case '!':
                default:
                    error(message[ERROR_OPERATOR],"%c",prev_op);
                    return total;
            }
            prev_op = token.terminator;
            expect = false;
        }
        str = token.next;
    }
    if(expect) {
        error(message[ERROR_MISSINGOPERAND],0);
        return 0;
    }
    return total;
}

// efficient strcpy/strcat compound function
uint8_t strcompound(char *dest, const char *src1, const char *src2) {
    uint8_t len = 0;

    while(*src1) {
        *dest++ = *src1++;
        len++;
    }
    while(*src2) {
        *dest++ = *src2++;
        len++;
    }
    *dest = 0;
    return len;
}

char * _nextline_ptr;

uint16_t getnextline(char *dst) {
    uint16_t len = 0;

    while(*_nextline_ptr) {
        *dst++ = *_nextline_ptr;
        len++;
        if(*_nextline_ptr++ == '\n') {
            break;
        }
    }
    *dst = 0;
    return len;
}

void resetnextline(char *src) {
    _nextline_ptr = src;
}
