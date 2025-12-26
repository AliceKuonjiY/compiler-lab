#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ir.h"
#include "ast.h"

int temp_cnt = 0;
int label_cnt = 0;

IRLabel new_temp() {
    IRLabel label;
    label.kind = LABEL_VARIABLE;
    label.name = malloc(10);
    label.id = temp_cnt;
    sprintf(label.name, "t%d", temp_cnt);
    temp_cnt++;
    return label;
}

IRLabel new_label() {
    IRLabel label;
    label.kind = LABEL_LABEL;
    label.name = malloc(10);
    label.id = label_cnt;
    sprintf(label.name, "l%d", label_cnt);
    label_cnt++;
    return label;
}

InterCode new_assign(IRLabel x, IRLabel y) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_ASSIGN;
    code->code.x = x;
    code->code.assign.y = y;
    return code;
}

InterCode new_dereference(int op, IRLabel x, IRLabel y) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_DEREFERENCE;
    code->code.dereference.op = op;
    code->code.x = x;
    code->code.dereference.y = y;
    return code;
}

InterCode new_reference(IRLabel x, IRLabel y) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_REFERENCE;
    code->code.x = x;
    code->code.reference.y = y;
    return code;
}

InterCode new_binop(int op, IRLabel x, IRLabel y, IRLabel z) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_BINOP;
    code->code.binop.op = op;
    code->code.x = x;
    code->code.binop.y = y;
    code->code.binop.z = z;
    return code;
}

InterCode new_labelop(int op, IRLabel x) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_LABELOP;
    code->code.labelop.op = op;
    code->code.x = x;
    return code;
}

InterCode new_call(IRLabel x, IRLabel f) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_CALL;
    code->code.x = x;
    code->code.call.f = f;
    return code;
}

InterCode new_condjmp(int relop, IRLabel x, IRLabel y, IRLabel z) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_CONDJMP;
    code->code.condjmp.relop = relop;
    code->code.x = x;
    code->code.condjmp.y = y;
    code->code.condjmp.z = z;
    return code;
}

InterCode new_dec(IRLabel x, int size) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_DEC;
    code->code.x = x;
    code->code.dec.size = size;
    return code;
}

void contactIR(InterCode *code1, InterCode *code2) {
    if (*code1 == NULL) {
        *code1 = *code2;
        return;
    } else if (*code2 == NULL) {
        return;
    }
    InterCode code1_tail = *code1;
    while (code1_tail->next != NULL) {
        code1_tail = code1_tail->next;
    }
    code1_tail->next = *code2;
    (*code2)->prev = code1_tail;
}

static void print_label(IRLabel label) {
    if (label.kind == LABEL_CONSTANT) {
        printf("#%d", label.value);
    } else {
        printf("%s", label.name);
    }
}

void print_ir(InterCode code) {
    InterCode now = code;
    while (now != NULL) {
        Code line = now->code;
        switch (line.kind) {
            case CODE_ASSIGN:
                print_label(line.x);
                printf(" := ");
                print_label(line.assign.y);
                printf("\n");
                break;
            case CODE_DEREFERENCE:
                if (line.dereference.op == DE_LEFT) {
                    printf("*");
                }
                print_label(line.x);
                printf(" := ");
                if (line.dereference.op == DE_RIGHT) {
                    printf("*");
                }
                print_label(line.dereference.y);
                printf("\n");
                break;
            case CODE_REFERENCE:
                print_label(line.x);
                printf(" := &");
                print_label(line.reference.y);
                printf("\n");
                break;
            case CODE_BINOP:
                print_label(line.x);
                printf(" := ");
                print_label(line.binop.y);
                if (line.binop.op == BINOP_PLUS) {
                    printf(" + ");
                } else if (line.binop.op == BINOP_MINUS) {
                    printf(" - ");
                } else if (line.binop.op == BINOP_STAR) {
                    printf(" * ");
                } else if (line.binop.op == BINOP_DIV) {
                    printf(" / ");
                }
                print_label(line.binop.z);
                printf("\n");
                break;
            case CODE_LABELOP:
                if (line.labelop.op == LABELOP_DEFLABEL) {
                    printf("LABEL ");
                } else if (line.labelop.op == LABELOP_FUNCTION) {
                    printf("FUNCTION ");
                } else if (line.labelop.op == LABELOP_GOTO) {
                    printf("GOTO ");
                } else if (line.labelop.op == LABELOP_RETURN) {
                    printf("RETURN ");
                } else if (line.labelop.op == LABELOP_ARG) {
                    printf("ARG ");
                } else if (line.labelop.op == LABELOP_PARAM) {
                    printf("PARAM ");
                } else if (line.labelop.op == LABELOP_READ) {
                    printf("READ ");
                } else if (line.labelop.op == LABELOP_WRITE) {
                    printf("WRITE ");
                }
                print_label(line.x);
                if (line.labelop.op == LABELOP_DEFLABEL || line.labelop.op == LABELOP_FUNCTION) {
                    printf(" :");
                }
                printf("\n");
                break;
            case CODE_CALL:
                print_label(line.x);
                printf(" := CALL ");
                print_label(line.call.f);
                printf("\n");
                break;
            case CODE_CONDJMP:
                printf("IF ");
                print_label(line.x);
                if (line.condjmp.relop == RELOP_GT) {
                    printf(" > ");
                } else if (line.condjmp.relop == RELOP_LT) {
                    printf(" < ");
                } else if (line.condjmp.relop == RELOP_GE) {
                    printf(" >= ");
                } else if (line.condjmp.relop == RELOP_LE) {
                    printf(" <= ");
                } else if (line.condjmp.relop == RELOP_EQ) {
                    printf(" == ");
                } else if (line.condjmp.relop == RELOP_NEQ) {
                    printf(" != ");
                }
                print_label(line.condjmp.y);
                printf(" GOTO ");
                print_label(line.condjmp.z);
                printf("\n");
                break;
            case CODE_DEC:
                printf("DEC ");
                print_label(line.x);
                printf(" %d\n", line.dec.size);
                break;
            default:
                assert(0);
        }
        now = now->next;
    }
}
