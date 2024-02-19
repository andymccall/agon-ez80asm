#ifndef UTILS_H
#define UTILS_H

#include "config.h"

typedef struct {
    char    start[TOKEN_MAX + 1];
    uint8_t length;
    char    *next;
    char    terminator;
} token_t;

// Token type that points to the (changed) underlying data string
typedef struct {
    char    *start;
    char    *next;
    char    terminator;
} streamtoken_t;

void text_RED(void);
void text_YELLOW(void);
void text_NORMAL(void);

void remove_ext (char* myStr, char extSep, char pathSep);
void trimRight(char *str);
void error(char* msg);
bool isEmpty(const char *str);
bool notEmpty(const char *str);
uint8_t getLineToken(token_t *token, char *src, char terminator);
uint8_t getOperatorToken(token_t *token, char *src);
void getLabelToken(streamtoken_t *token, char *src);
uint8_t getMnemonicToken(streamtoken_t *token, char *src);
uint8_t getOperandToken(streamtoken_t *token, char *src);
void parse_command(char *src);

#ifdef AGON
int strcasecmp(char *s1, char *s2);
#endif
#endif // UTILS_H
