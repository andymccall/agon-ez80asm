#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "globals.h"
#include "utils.h"
#include "assemble.h"
#include "label.h"
#include "stdint.h"

int main(int argc, char *argv[])
{
    if(argc < 2){
        printf("Usage: asm <filename> [-l]\n");
        return 0;
    }

    prepare_filenames(argv[1]);
    if(!openfiles()) return 0;

    debug_enabled = false;
    if((argc == 3) && (strcmp(argv[2], "-d") == 0)) debug_enabled = true;
    if((argc == 3) && (strcmp(argv[2], "-l") == 0)) consolelist_enabled = true;

    // Init tables
    initGlobalLabelTable();
    initLocalLabelTable();
    initAnonymousLabelTable();

    // Assemble input to output
    assemble();
    if(global_errors) {
        //remove(filename[FILE_OUTPUT]);
        printf("Error in input\n");
    }
    else printf("%d bytes\n", totalsize);
 
    closeAllFiles();   
    return 0;
}