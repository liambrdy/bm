#define BM_IMPLEMENTATION
#include "bm.h"

Bm bm = {0};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./debasm <input.bm>\n");
        fprintf(stderr, "ERROR: no input is provided\n");
        exit(1);
    }

    bmLoadProgramFromFile(&bm, argv[1]);
    for (Word i = 0; i < bm.programSize; i++) {
        switch (bm.program[i].type) {
            case INST_NOP:
                printf("nop\n");
                break;
            case INST_PUSH:
                printf("push %ld\n", bm.program[i].operand);
                break;
            case INST_DUP:
                printf("dup %ld\n", bm.program[i].operand);
                break;
            case INST_PLUS:
                printf("plus\n");
                break;
            case INST_MINUS:
                printf("minus\n");
                break;
            case INST_MULT:
                printf("mult\n");
                break;
            case INST_DIV:
                printf("div\n");
                break;
            case INST_JMP:
                printf("jmp %ld\n", bm.program[i].operand);
                break;
            case INST_JMP_IF:
                printf("jmp_if %ld\n", bm.program[i].operand);
                break;
            case INST_EQ:
                printf("eq\n");
                break;
            case INST_HALT:
                printf("halt\n");
                break;
            case INST_PRINT_DEBUG:
                printf("print_debug\n");
                break;
            default:
                assert(0 && "Unreachable");
        }
    }
}