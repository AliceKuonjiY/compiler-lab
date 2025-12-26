#include <stdint.h>
#include "list.h"

typedef struct TargetCode TargetCode;

enum OpCode {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MFLO,
    OP_ADDI, OP_LI,
    OP_MOVE, OP_LW, OP_SW, OP_LA,
    OP_J, OP_JAL, OP_JR,
    OP_BEQ, OP_BNE,
    OP_BLT, OP_BGT, OP_BLE, OP_BGE,
    OP_SYSCALL,
    OP_LABEL,
    OP_NOP
};

enum Register {
    ZERO,
    AT,
    V0, V1,
    A0, A1, A2, A3,
    T0, T1, T2, T3, T4, T5, T6, T7,
    S0, S1, S2, S3, S4, S5, S6, S7,
    T8, T9,
    K0, K1,
    GP,
    SP,
    FP,
    RA
};

struct TargetCode {
    ListHead list;
    enum OpCode op;
    enum Register rd, rs, rt;
    uint32_t off, imm;
    char *label;
};

TargetCode* new_target_code(int op, int rd, int rs, int rt, uint32_t off, uint32_t imm, char *label);
void print_asm(ListHead *code, const char *output_path);