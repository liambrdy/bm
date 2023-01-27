#ifndef BM_H
#define BM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))
#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024

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

const char *errAsCStr(Err trap);

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

const char *instTypeAsCStr(InstType type);

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

Err bmExecuteInst(Bm *bm);
Err bmExecuteProgram(Bm *bm, int limit);
void bmDumpStack(FILE *stream, const Bm *bm);

void bmLoadProgramFromMemory(Bm *bm, Inst *program, size_t programSize);
void bmLoadProgramFromFile(Bm *bm, const char *filePath);
void bmSaveProgramToFile(Bm *bm, const char *filePath);

typedef struct {
    size_t count;
    const char *data;
} StringView;

StringView cStrAsSV(const char *cstr);
StringView svTrimLeft(StringView sv);
StringView svTrimRight(StringView sv);
StringView svTrim(StringView sv);
StringView svChopByDelim(StringView *sv, char delim);
int svEq(StringView a, StringView b);
int svToInt(StringView sv);

Inst bmTranslateLine(StringView line);
size_t bmTranslateSource(StringView source, Inst *program, size_t programCapacity);

StringView slurpFile(const char *filePath);

#endif

#ifdef BM_IMPLEMENTATION

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

Err bmExecuteProgram(Bm *bm, int limit) {
    while (limit != 0 && !bm->halt) {
        Err err = bmExecuteInst(bm);
        if (err != ERR_OK) {
            return err;
        }

        if (limit > 0) {
            --limit;
        }
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

void bmSaveProgramToFile(Bm *bm, const char *filePath) {
    FILE *f = fopen(filePath, "wb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not open file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }

    fwrite(bm->program, sizeof(bm->program[0]), bm->programSize, f);

    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could no write to file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }

    fclose(f);
}

StringView cStrAsSV(const char *cstr) {
    return (StringView) {
        .count = strlen(cstr),
        .data = cstr
    };
}

StringView svTrimLeft(StringView sv) {
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[i])) {
        i += 1;
    }

    return (StringView) {
        .count = sv.count - i,
        .data = sv.data + i
    };
}

StringView svTrimRight(StringView sv) {
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
        i += 1;
    }

    return (StringView) {
        .count = sv.count - i,
        .data = sv.data
    };
}

StringView svTrim(StringView sv) {
    return svTrimRight(svTrimLeft(sv));
}

StringView svChopByDelim(StringView *sv, char delim) {
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim) {
        i += 1;
    }

    StringView result = {
        .count = i,
        .data = sv->data
    };

    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data += i + 1;
    } else {
        sv->count -= i;
        sv->data += i;
    }

    return result;
}

int svEq(StringView a, StringView b) {
    if (a.count != b.count) {
        return 0;
    } else {
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

int svToInt(StringView sv) {
    int result = 0;

    for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); i++) {
        result = result * 10 + sv.data[i] - '0';
    }

    return result;
}

Inst bmTranslateLine(StringView line) {
    line = svTrimLeft(line);
    StringView instName = svChopByDelim(&line, ' ');
    StringView operand = svTrim(svChopByDelim(&line, '#'));

    if (svEq(instName, cStrAsSV("push"))) {
        line = svTrimLeft(line);
        return (Inst) { .type = INST_PUSH, .operand = svToInt(operand) };
    } else if (svEq(instName, cStrAsSV("dup"))) {
        line = svTrimLeft(line);
        return (Inst) { .type = INST_DUP, .operand = svToInt(operand) };
    } else if (svEq(instName, cStrAsSV("plus"))) {
        return (Inst) { .type = INST_PLUS };
    } else if (svEq(instName, cStrAsSV("jmp"))) {
        line = svTrimLeft(line);
        return (Inst) { .type = INST_JMP, .operand = svToInt(operand) };
    } else {
        fprintf(stderr, "ERROR unknown instruction `%.*s`\n", (int) instName.count, instName.data);
        exit(1);
    }
}

size_t bmTranslateSource(StringView source, Inst *program, size_t programCapacity) {
    size_t programSize = 0;
    while (source.count > 0) {
        assert(programSize < programCapacity);
        StringView line = svTrim(svChopByDelim(&source, '\n'));
        if (line.count > 0 && *line.data != '#') {
            program[programSize++] = bmTranslateLine(line);
        }
    }

    return programSize;
}

StringView slurpFile(const char *filePath) {
    FILE *f = fopen(filePath, "r");
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

    char *buffer = malloc(m);
    if (buffer == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for file: %s\n", strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }

    size_t n = fread(buffer, 1, m, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n", filePath, strerror(errno));
        exit(1);
    }

    fclose(f);

    return (StringView) {
        .count = n,
        .data = buffer
    };
}

#endif