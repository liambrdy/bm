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

void usage(FILE *stream, const char *program) {
    fprintf(stream, "Usage: %s -i <input.bm> [-l <limit>] [-h]\n", program);
}

int main(int argc, char *argv[]) {
    const char *program = shift(&argc, &argv);
    const char *inputFilePath = NULL;
    int limit = -1;

    while (argc > 0) {
        const char *flag = shift(&argc, &argv);

        if (strcmp(flag, "-i") == 0) {
            if (argc == 0) {
                usage(stderr, program);
                fprintf(stderr, "ERROR: No argument is provided for flag `%s`\n", flag);
                exit(1);
            }

            inputFilePath = shift(&argc, &argv);
        } else if (strcmp(flag, "-l") == 0) {
            if (argc == 0) {
                usage(stderr, program);
                fprintf(stderr, "ERROR: No argument is provided for flag `%s`\n", flag);
                exit(1);
            }

            limit = atoi(shift(&argc, &argv));
        } else if (strcmp(flag, "-h") == 0) {
            usage(stdout, program);
            exit(0);
        } else {
            usage(stderr, program);
            fprintf(stderr, "ERROR: Unkown flag `%s`\n", flag);
        }
    }

    if (inputFilePath == NULL) {
        usage(stderr, program);
        fprintf(stderr, "ERROR: Input not provided\n");
        exit(1);
    }

    bmLoadProgramFromFile(&bm, inputFilePath);
    Err err = bmExecuteProgram(&bm, limit);
    
    bmDumpStack(stdout, &bm);

    if (err != ERR_OK) {
        fprintf(stderr, "ERROR: %s\n", errAsCStr(err));
        return 1;
    }

    return 0;
}