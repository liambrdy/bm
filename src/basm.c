#define BM_IMPLEMENTATION
#include "bm.h"

Bm bm = {0};

char *shift(int *argc, char ***argv) {
    assert(*argc > 0);

    char *result = **argv;
    *argv += 1;
    *argc -= 1;

    return result;
}

void usage(FILE* stream, const char *program) {
    fprintf(stream, "Usage: %s <input.basm> <output.bm>\n", program);
}

int main(int argc, char *argv[]) {
    char *program = shift(&argc, &argv);

    if (argc == 0) {
        usage(stderr, program);
        fprintf(stderr, "ERROR: expected input\n");
        exit(1);
    }
    const char *inputFilePath = shift(&argc, &argv);

    if (argc == 0) {
        usage(stderr, program);
        fprintf(stderr, "ERROR: expected output\n");
        exit(1);
    }
    const char *outputFilePath = shift(&argc, &argv);

    StringView source = slurpFile(inputFilePath);

    bm.programSize = bmTranslateSource(source, bm.program, BM_PROGRAM_CAPACITY);

    bmSaveProgramToFile(&bm, outputFilePath);

    return 0;
}