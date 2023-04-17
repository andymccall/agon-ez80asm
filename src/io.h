#ifndef IO_H
#define IO_H

#include "config.h"

bool openFile(uint8_t *file, char *name, uint8_t mode);
bool reOpenFile(uint8_t number, uint8_t mode);
void prepare_filenames(char *input_filename);
void getMacroFilename(char *filename, char *macroname);
void closeAllFiles();
bool openfiles(void);
void addFileDeleteList(char *name);
void deleteFiles(void);

char *agon_fgets(char *s, int size, uint8_t fileid);
int agon_fputs(char *s, uint8_t fileid);
size_t agon_fwrite(void *ptr, size_t size, size_t nmemb, uint8_t fileid);
size_t agon_fread(void *ptr, size_t size, size_t nmemb, uint8_t fileid);

void outputBufferedWrite(unsigned char s);
void outputBufferInit(void);
void outputBufferFlush(void);

#endif // IO_H