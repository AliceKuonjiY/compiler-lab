#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "asm.h"
#include "list.h"

TargetCode *new_target_code(int op, int rd, int rs, int rt, uint32_t off, uint32_t imm, char *label) {
    TargetCode *code = (TargetCode *)malloc(sizeof(struct TargetCode));
    init_list_head(&code->list);
    code->op = op;
    code->rd = rd;
    code->rs = rs;
    code->rt = rt;
    code->off = off;
    code->imm = imm;
    code->label = label;
    return code;
}

void print_asm(ListHead *code, const char *output_path) {
    FILE *fp = fopen(output_path, "w");
    if (fp == NULL) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, ".data\n");
    fprintf(fp, "_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(fp, "_ret: .asciiz \"\\n\"\n\n");
    fprintf(fp, ".globl main\n\n");
    fprintf(fp, ".text\n");
    // fprintf(fp, "read:\n");
    // fprintf(fp, "  li $v0, 4\n");
    // fprintf(fp, "  la $a0, _prompt\n");
    // fprintf(fp, "  syscall\n");
    // fprintf(fp, "  li $v0, 5\n");
    // fprintf(fp, "  syscall\n");
    // fprintf(fp, "  jr $ra\n\n");
    // fprintf(fp, "write:\n");
    // fprintf(fp, "  li $v0, 1\n");
    // fprintf(fp, "  syscall\n");
    // fprintf(fp, "  li $v0, 4\n");
    // fprintf(fp, "  la $a0, _ret\n");
    // fprintf(fp, "  syscall\n");
    // fprintf(fp, "  move $v0, $0\n");
    // fprintf(fp, "  jr $ra\n\n");
    TargetCode *pos;
    list_for_each_entry(pos, code, TargetCode, list) {
        switch (pos->op) {
            case OP_ADD:
                fprintf(fp, "  add $%d, $%d, $%d\n", pos->rd, pos->rs, pos->rt);
                break;
            case OP_SUB:
                fprintf(fp, "  sub $%d, $%d, $%d\n", pos->rd, pos->rs, pos->rt);
                break;
            case OP_MUL:
                fprintf(fp, "  mul $%d, $%d, $%d\n", pos->rd, pos->rs, pos->rt);
                break;
            case OP_DIV:
                fprintf(fp, "  div $%d, $%d\n", pos->rs, pos->rt);
                break;
            case OP_MFLO:
                fprintf(fp, "  mflo $%d\n", pos->rd);
                break;
            case OP_ADDI:
                fprintf(fp, "  addi $%d, $%d, %d\n", pos->rd, pos->rs, pos->imm);
                break;
            case OP_LI:
                fprintf(fp, "  li $%d, %d\n", pos->rd, pos->imm);
                break;
            case OP_MOVE:
                fprintf(fp, "  move $%d, $%d\n", pos->rd, pos->rs);
                break;
            case OP_LW:
                fprintf(fp, "  lw $%d, %d($%d)\n", pos->rd, pos->off, pos->rs);
                break;
            case OP_SW:
                fprintf(fp, "  sw $%d, %d($%d)\n", pos->rs, pos->off, pos->rd);
                break;
            case OP_LA:
                fprintf(fp, "  la $%d, %s\n", pos->rd, pos->label);
                break;
            case OP_J:
                fprintf(fp, "  j %s\n", pos->label);
                break;
            case OP_JAL:
                fprintf(fp, "  jal %s\n", pos->label);
                break;
            case OP_JR:
                fprintf(fp, "  jr $%d\n", pos->rd);
                break;
            case OP_BEQ:
                fprintf(fp, "  beq $%d, $%d, %s\n", pos->rs, pos->rt, pos->label);
                break;
            case OP_BNE:
                fprintf(fp, "  bne $%d, $%d, %s\n", pos->rs, pos->rt, pos->label);
                break;
            case OP_BLT:
                fprintf(fp, "  blt $%d, $%d, %s\n", pos->rs, pos->rt, pos->label);
                break;
            case OP_BGT:
                fprintf(fp, "  bgt $%d, $%d, %s\n", pos->rs, pos->rt, pos->label);
                break;
            case OP_BLE:
                fprintf(fp, "  ble $%d, $%d, %s\n", pos->rs, pos->rt, pos->label);
                break;
            case OP_BGE:
                fprintf(fp, "  bge $%d, $%d, %s\n", pos->rs, pos->rt, pos->label);
                break;
            case OP_SYSCALL:
                fprintf(fp, "  syscall\n");
                break;
            case OP_LABEL:
                fprintf(fp, "%s:\n", pos->label);
                break;
            case OP_NOP:
                fprintf(fp, "\n");
                break;
            default:
                fprintf(stderr, "Unknown OpCode: %d\n", pos->op);
                break;
        }
    }
    fclose(fp);
}