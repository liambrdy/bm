#define BM_IMPLEMENTATION
#include "bm.h"

Bm bm = {0};

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./bmi <input.bm>\n");
        fprintf(stderr, "ERROR: expected input\n");
        exit(1);
    }

    bmLoadProgramFromFile(&bm, argv[1]);
    for (int i = 0; i < BM_EXECUTION_LIMIT && !bm.halt; ++i) {
        Err trap = bmExecuteInst(&bm);
        if (trap != ERR_OK) {
            fprintf(stderr, "Error: %s\n", errAsCStr(trap));
            exit(1);
        }
    }
    
    bmDumpStack(stdout, &bm);

    return 0;
}