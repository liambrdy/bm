#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <errno.h>

#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define BM_EXECUTION_LIMIT 100

typedef int64_t Word;

typedef enum {
    ERR_OK = 0,
    ERR_STACK_OVERFLOW,
    ERR_STACK_UNDERFLOW,
    ERR_ILLEGAL_INST,
    ERR_ILLEGAL_OPERAND,
    ERR_DIV_BY_ZERO,
    ERR_ILLEGAL_INST_ACCESS,
} Err;

const char *errAsCStr(Err trap) {
    switch (trap) {
        case ERR_OK: return "ERR_OK";
        case ERR_STACK_OVERFLOW: return "ERR_STACK_OVERFLOW";
        case ERR_STACK_UNDERFLOW: return "ERR_STACK_UNDERFLOW";
        case ERR_ILLEGAL_INST: return "ERR_ILLEGAL_INST";
        case ERR_ILLEGAL_OPERAND: return "ERR_ILLEGAL_OPERAND";
        case ERR_DIV_BY_ZERO: return "ERR_DIV_BY_ZERO";
        case ERR_ILLEGAL_INST_ACCESS: return "ERR_ILLEGAL_INST_ACCESS";
        default: assert(0 && "errAsCStr: Unreachable");
    }
}

typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_DUP,
    INST_PLUS,
    INST_MINUS,
    INST_MULT,
    INST_DIV,
    INST_JMP,
    INST_JMP_IF,
    INST_EQ,
    INST_HALT,
    INST_PRINT_DEBUG,
} InstType;

const char *instTypeAsCStr(InstType type) {
    switch (type) {
        case INST_NOP: return "INST_NOP";
        case INST_PUSH: return "INST_PUSH";
        case INST_DUP: return "INST_DUP";
        case INST_PLUS: return "INST_PLUS";
        case INST_MINUS: return "INST_MINUS";
        case INST_MULT: return "INST_MULT";
        case INST_DIV: return "INST_DIV";
        case INST_JMP: return "INST_JMP";
        case INST_JMP_IF: return "INST_JMP_IF";
        case INST_EQ: return "INST_EQ";
        case INST_HALT: return "INST_HALT";
        case INST_PRINT_DEBUG: return "INST_PRINT_DEBUG";
        default: assert(0 && "instTypeAsCStr: Unreachable");
    }
}

typedef struct {
    InstType type;
    Word operand;
} Inst;

typedef struct {
    Word stack[BM_STACK_CAPACITY];
    Word stackSize;

    Inst program[BM_PROGRAM_CAPACITY];
    Word programSize;
    Word ip;


    int halt;
} Bm;

#define MAKE_INST_PUSH(value) {.type = INST_PUSH, .operand = (value)}
#define MAKE_INST_DUP(addr) {.type = INST_DUP, .operand = (addr)}
#define MAKE_INST_PLUS {.type = INST_PLUS}
#define MAKE_INST_MINUS {.type = INST_MINUS}
#define MAKE_INST_MULT {.type = INST_MULT}
#define MAKE_INST_DIV {.type = INST_DIV}
#define MAKE_INST_JMP(addr) {.type = INST_JMP, .operand = (addr)}
#define MAKE_INST_JMP_IF(addr) {.type = INST_JMP_IF, .operand = (addr)}
#define MAKE_INST_EQ {.type = INST_EQ}
#define MAKE_INST_HALT {.type = INST_HALT}
#define MAKE_INST_PRINT_DEBUG {.type = INST_PRINT_DEBUG}

Err bmExecuteInst(Bm *bm) {
    if (bm->ip < 0 || bm->ip >= bm->programSize) {
        return ERR_ILLEGAL_INST_ACCESS;
    }

    Inst inst = bm->program[bm->ip];

    switch (inst.type) {
        case INST_NOP:
            bm->ip += 1;
            break;

        case INST_PUSH:
            if (bm->stackSize >= BM_STACK_CAPACITY) {
                return ERR_STACK_OVERFLOW;
            }
            bm->stack[bm->stackSize++] = inst.operand;
            bm->ip += 1;
            break;

        case INST_DUP:
            if (bm->stackSize >= BM_STACK_CAPACITY) {
                return ERR_STACK_OVERFLOW;
            }

            if (bm->stackSize - inst.operand <= 0) {
                return ERR_STACK_UNDERFLOW;
            }

            if (inst.operand < 0) {
                return ERR_ILLEGAL_OPERAND;
            }

            bm->stack[bm->stackSize] = bm->stack[bm->stackSize - 1 - inst.operand];

            bm->stackSize += 1;
            bm->ip += 1;
            
            break;

        case INST_PLUS:
            if (bm->stackSize < 2) {
                return ERR_STACK_UNDERFLOW;
            }
            bm->stack[bm->stackSize - 2] += bm->stack[bm->stackSize - 1];
            bm->stackSize -= 1;
            bm->ip += 1;
            break;

        case INST_MINUS:
            if (bm->stackSize < 2) {
                return ERR_STACK_UNDERFLOW;
            }
            bm->stack[bm->stackSize - 2] -= bm->stack[bm->stackSize - 1];
            bm->stackSize -= 1;
            bm->ip += 1;
            break;

        case INST_MULT:
            if (bm->stackSize < 2) {
                return ERR_STACK_UNDERFLOW;
            }
            bm->stack[bm->stackSize - 2] *= bm->stack[bm->stackSize - 1];
            bm->stackSize -= 1;
            bm->ip += 1;
            break;

        case INST_DIV:
            if (bm->stackSize < 2) {
                return ERR_STACK_UNDERFLOW;
            }

            if (bm->stack[bm->stackSize - 1] == 0) {
                return ERR_DIV_BY_ZERO;
            }

            bm->stack[bm->stackSize - 2] /= bm->stack[bm->stackSize - 1];
            bm->stackSize -= 1;
            bm->ip += 1;
            break;

        case INST_JMP:
            bm->ip = inst.operand;
            break;

        case INST_JMP_IF:
            if (bm->stackSize < 1) {
                return ERR_STACK_UNDERFLOW;
            }

            if (bm->stack[bm->stackSize - 1]) {
                bm->stackSize -= 1;
                bm->ip = inst.operand;
            } else {
                bm->ip += 1;
            }

            break;

        case INST_EQ:
            if (bm->stackSize < 2) {
                return ERR_STACK_UNDERFLOW;
            }
            bm->stack[bm->stackSize - 2] = bm->stack[bm->stackSize - 1] == bm->stack[bm->stackSize - 2];
            bm->stackSize -= 1;
            bm->ip += 1;
            break;

        case INST_HALT:
            bm->halt = 1;
            break;

        case INST_PRINT_DEBUG:
            if (bm->stackSize < 1) {
                return ERR_STACK_UNDERFLOW;
            }
            printf("%ld", bm->stack[bm->stackSize - 1]);
            bm->stackSize -= 1;
            bm->ip += 1;
            break;

        default:
            return ERR_ILLEGAL_INST;
    }

    return ERR_OK;
}

void bmDumpStack(FILE *stream, const Bm *bm) {
    fprintf(stream, "Stack:\n");
    if (bm->stackSize > 0) {
        for (Word i = 0; i < bm->stackSize; i++) {
            fprintf(stream, "  %ld\n", bm->stack[i]);
        }
    } else {
        fprintf(stream, "  [empty]\n");
    }
}

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))

void bmLoadProgramFromMemory(Bm *bm, Inst *program, size_t programSize) {
    assert(programSize < BM_PROGRAM_CAPACITY);
    memcpy(bm->program, program, sizeof(program[0]) * programSize);
    bm->programSize = programSize;
}

void bmLoadProgramFromFile(Bm *bm, const char *filePath) {
    FILE *f = fopen(filePath, "rb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }
    
    long m = ftell(f);
    if (m < 0) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }

    assert(m % sizeof(bm->program[0]) == 0);
    assert((size_t)m <= BM_PROGRAM_CAPACITY * sizeof(bm->program[0]));

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }

    bm->programSize = fread(bm->program, sizeof(bm->program[0]), m / sizeof(bm->program[0]), f);
    
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }

    fclose(f);
}

void bmSaveProgramToFile(Inst *program, size_t programSize, const char *filePath) {
    FILE *f = fopen(filePath, "wb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }

    fwrite(program, sizeof(program[0]), programSize, f);

    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could no write to file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }

    fclose(f);
}

Bm bm = {0};
char *sourceCode =
    "push 0\n"
    "push 1\n"
    "dup 1\n"
    "dup 1\n"
    "plus\n"
    "jmp 2\n";

char *trimLeft(char *str, size_t strSize) {
    for (size_t i = 0; i < strSize; i++) {
        if (!isspace(str[i])) {
            return str + i;
        }
    }

    return str + strSize;
}

Inst bmTranslateLine(char *line, size_t lineSize) {
    char *end = trimLeft(line, lineSize);
    lineSize -= end - line;
    line = end;

    if (lineSize == 0) {
        fprintf(stderr, "Could not translate empty line to instruction\n");
        exit(1);
    }

    return (Inst) {0};
}

size_t bmTranslateSource(char *source, size_t sourceSize, Inst *program, size_t programCapacity) {
    while (sourceSize > 0) {
        char *end = memchr(source, '\n', sourceSize);
        size_t n = end != NULL ? (size_t) (end - source) : sourceSize;

        printf("#%.*s#\n", (int) n, source);

        source = end;
        sourceSize -= n;

        if (source != NULL) {
            source += 1;
            sourceSize -= 1;
        }
    }

    return 0;
}

int main() {
    bm.programSize = bmTranslateSource(sourceCode, strlen(sourceCode), bm.program, BM_PROGRAM_CAPACITY);

    return 0;
}

int main2() {
    // bmLoadProgramFromMemory(&bm, program, ARRAY_SIZE(program));
    bmLoadProgramFromFile(&bm, "fib.bm");
    bmDumpStack(stdout, &bm);

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