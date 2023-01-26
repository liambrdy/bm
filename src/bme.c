#define BM_IMPLEMENTATION
#include "bm.h"

Bm bm = {0};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./bme <input.bm>\n");
        fprintf(stderr, "ERROR: expected input\n");
        exit(1);
    }

    bmLoadProgramFromFile(&bm, argv[1]);
    Err err = bmExecuteProgram(&bm, -1);
    
    bmDumpStack(stdout, &bm);

    if (err != ERR_OK) {
        fprintf(stderr, "ERROR: %s\n", errAsCStr(err));
        return 1;
    }

    return 0;
}