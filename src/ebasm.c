#define BM_IMPLEMENTATION
#include "bm.h"

Bm bm = {0};

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: ./ebasm <input.ebasm> <output.bm>\n");
        fprintf(stderr, "expected input and output\n");
        exit(1);
    }

    const char *inputFilePath = argv[1];
    const char *outputFilePath = argv[2];

    StringView source = slurpFile(inputFilePath);

    bm.programSize = bmTranslateSource(source, bm.program, BM_PROGRAM_CAPACITY);

    bmSaveProgramToFile(bm.program, bm.programSize, outputFilePath);

    return 0;
}