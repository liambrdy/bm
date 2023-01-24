#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#define BM_STACK_CAPACITY 1024

typedef int64_t Word;

typedef enum {
    ERR_OK = 0,
    ERR_STACK_OVERFLOW,
    ERR_STACK_UNDERFLOW,
    ERR_ILLEGAL_INST,
    ERR_DIV_BY_ZERO,
} Err;

const char *errAsCStr(Err trap) {
    switch (trap) {
        case ERR_OK: return "ERR_OK";
        case ERR_STACK_OVERFLOW: return "ERR_STACK_OVERFLOW";
        case ERR_STACK_UNDERFLOW: return "ERR_STACK_UNDERFLOW";
        case ERR_ILLEGAL_INST: return "ERR_ILLEGAL_INST";
        case ERR_DIV_BY_ZERO: return "ERR_DIV_BY_ZERO";
        default: assert(0 && "errAsCStr: Unreachable");
    }
}

typedef struct {
    Word stack[BM_STACK_CAPACITY];
    size_t stackSize;
    Word ip;
    int halt;
} Bm;

typedef enum {
    INST_PUSH,
    INST_PLUS,
    INST_MINUS,
    INST_MULT,
    INST_DIV,
    INST_JMP,
    INST_HALT,
} InstType;

const char *instTypeAsCStr(InstType type) {
    switch (type) {
        case INST_PUSH: return "INST_PUSH";
        case INST_PLUS: return "INST_PLUS";
        case INST_MINUS: return "INST_MINUS";
        case INST_MULT: return "INST_MULT";
        case INST_DIV: return "INST_DIV";
        case INST_JMP: return "INST_JMP";
        case INST_HALT: return "INST_HALT";
        default: assert(0 && "instTypeAsCStr: Unreachable");
    }
}

typedef struct {
    InstType type;
    Word operand;
} Inst;

#define MAKE_INST_PUSH(value) {.type = INST_PUSH, .operand = (value)}
#define MAKE_INST_PLUS {.type = INST_PLUS}
#define MAKE_INST_MINUS {.type = INST_MINUS}
#define MAKE_INST_MULT {.type = INST_MULT}
#define MAKE_INST_DIV {.type = INST_DIV}
#define MAKE_INST_JMP(addr) {.type = INST_JMP, .operand = (addr)}
#define MAKE_INST_HALT {.type = INST_HALT}

Err bmExecuteInst(Bm *bm, Inst inst) {
    switch (inst.type) {
        case INST_PUSH:
            if (bm->stackSize >= BM_STACK_CAPACITY) {
                return ERR_STACK_OVERFLOW;
            }
            bm->stack[bm->stackSize++] = inst.operand;
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
            if (bm)

        default:
            return ERR_ILLEGAL_INST;
    }

    return ERR_OK;
}

void bmDump(FILE *stream, const Bm *bm) {
    fprintf(stream, "Stack:\n");
    if (bm->stackSize > 0) {
        for (size_t i = 0; i < bm->stackSize; i++) {
            fprintf(stream, "  %ld\n", bm->stack[i]);
        }
    } else {
        fprintf(stream, "  [empty]\n");
    }
}

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))

Bm bm = {0};
Inst program[] = {
    MAKE_INST_PUSH(12),
    MAKE_INST_PUSH(21),
    MAKE_INST_PLUS,
};

int main() {
    bmDump(stdout, &bm);

    while (!bm.halt) {
        printf("%s\n", instTypeAsCStr(program[bm.ip].type));
        Err trap = bmExecuteInst(&bm, program[bm.ip]);
        if (trap != ERR_OK) {
            fprintf(stderr, "Error: %s\n", errAsCStr(trap));
            exit(1);
        }
    }

    return 0;
}