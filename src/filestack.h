#ifndef FILESTACK_H
#define FILESTACK_H

#include <stdint.h>
#include "globals.h"

#define FILESTACK_MAXFILES  4

typedef struct {
    uint16_t linenumber;
    uint8_t fp;
    char filename[FILENAMEMAXLENGTH];
} filestackitem;

void filestackInit(void);
uint8_t filestackCount(void);
bool filestackPush(filestackitem *fs);
bool filestackPop(filestackitem *fs);

#endif