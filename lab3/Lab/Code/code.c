#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "code.h"
#include "ast.h"

int temp_cnt = 0;
int label_cnt = 0;

Label new_temp() {
    Label label;
    label.kind = LABEL_VARIABLE;
    label.name = malloc(20);
    memset(label.name, 0, 20);
    sprintf(label.name, "t%d", temp_cnt);
    temp_cnt++;
    return label;
}

Label new_label() {
    Label label;
    label.kind = LABEL_LABEL;
    label.name = malloc(20);
    memset(label.name, 0, 20);
    sprintf(label.name, "l%d", label_cnt);
    label_cnt++;
    return label;
}

InterCode new_assign(Label x, Label y) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_ASSIGN;
    code->code.assign.x = x;
    code->code.assign.y = y;
    return code;
}

InterCode new_dereference(int op, Label x, Label y) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_DEREFERENCE;
    code->code.dereference.op = op;
    code->code.dereference.x = x;
    code->code.dereference.y = y;
    return code;
}

InterCode new_reference(Label x, Label y) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_REFERENCE;
    code->code.reference.x = x;
    code->code.reference.y = y;
    return code;
}

InterCode new_binop(int op, Label x, Label y, Label z) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_BINOP;
    code->code.binop.op = op;
    code->code.binop.x = x;
    code->code.binop.y = y;
    code->code.binop.z = z;
    return code;
}

InterCode new_labelop(int op, Label x) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_LABELOP;
    code->code.labelop.op = op;
    code->code.labelop.x = x;
    return code;
}

InterCode new_call(Label x, Label f) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_CALL;
    code->code.call.x = x;
    code->code.call.f = f;
    return code;
}

InterCode new_condjmp(int relop, Label x, Label y, Label z) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_CONDJMP;
    code->code.condjmp.relop = relop;
    code->code.condjmp.x = x;
    code->code.condjmp.y = y;
    code->code.condjmp.z = z;
    return code;
}

InterCode new_dec(Label x, int size) {
    InterCode code = malloc(sizeof(struct InterCode_));
    code->next = code->prev = NULL;
    code->code.kind = CODE_DEC;
    code->code.dec.x = x;
    code->code.dec.size = size;
    return code;
}

void contact(InterCode *code1, InterCode *code2) {
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

static void print_label(Label label, FILE *file) {
    if (label.kind == LABEL_CONSTANT) {
        fprintf(file, "#%d", label.value);
    } else {
        fprintf(file, "%s", label.name);
    }
}

void print_ir(InterCode code, const char *filepath) {
    FILE *file = fopen(filepath, "w");
    InterCode now = code;
    while (now != NULL) {
        Code line = now->code;
        switch (line.kind) {
            case CODE_ASSIGN:
                print_label(line.assign.x, file);
                fprintf(file, " := ");
                print_label(line.assign.y, file);
                fprintf(file, "\n");
                break;
            case CODE_DEREFERENCE:
                if (line.dereference.op == DE_LEFT) {
                    fprintf(file, "*");
                }
                print_label(line.dereference.x, file);
                fprintf(file, " := ");
                if (line.dereference.op == DE_RIGHT) {
                    fprintf(file, "*");
                }
                print_label(line.dereference.y, file);
                fprintf(file, "\n");
                break;
            case CODE_REFERENCE:
                print_label(line.reference.x, file);
                fprintf(file, " := &");
                print_label(line.reference.y, file);
                fprintf(file, "\n");
                break;
            case CODE_BINOP:
                print_label(line.binop.x, file);
                fprintf(file, " := ");
                print_label(line.binop.y, file);
                if (line.binop.op == BINOP_PLUS) {
                    fprintf(file, " + ");
                } else if (line.binop.op == BINOP_MINUS) {
                    fprintf(file, " - ");
                } else if (line.binop.op == BINOP_STAR) {
                    fprintf(file, " * ");
                } else if (line.binop.op == BINOP_DIV) {
                    fprintf(file, " / ");
                }
                print_label(line.binop.z, file);
                fprintf(file, "\n");
                break;
            case CODE_LABELOP:
                if (line.labelop.op == LABELOP_DEFLABEL) {
                    fprintf(file, "LABEL ");
                } else if (line.labelop.op == LABELOP_FUNCTION) {
                    fprintf(file, "FUNCTION ");
                } else if (line.labelop.op == LABELOP_GOTO) {
                    fprintf(file, "GOTO ");
                } else if (line.labelop.op == LABELOP_RETURN) {
                    fprintf(file, "RETURN ");
                } else if (line.labelop.op == LABELOP_ARG) {
                    fprintf(file, "ARG ");
                } else if (line.labelop.op == LABELOP_PARAM) {
                    fprintf(file, "PARAM ");
                } else if (line.labelop.op == LABELOP_READ) {
                    fprintf(file, "READ ");
                } else if (line.labelop.op == LABELOP_WRITE) {
                    fprintf(file, "WRITE ");
                }
                print_label(line.labelop.x, file);
                if (line.labelop.op == LABELOP_DEFLABEL || line.labelop.op == LABELOP_FUNCTION) {
                    fprintf(file, " :");
                }
                fprintf(file, "\n");
                break;
            case CODE_CALL:
                print_label(line.call.x, file);
                fprintf(file, " := CALL ");
                print_label(line.call.f, file);
                fprintf(file, "\n");
                break;
            case CODE_CONDJMP:
                fprintf(file, "IF ");
                print_label(line.condjmp.x, file);
                if (line.condjmp.relop == RELOP_GT) {
                    fprintf(file, " > ");
                } else if (line.condjmp.relop == RELOP_LS) {
                    fprintf(file, " < ");
                } else if (line.condjmp.relop == RELOP_GEQ) {
                    fprintf(file, " >= ");
                } else if (line.condjmp.relop == RELOP_LEQ) {
                    fprintf(file, " <= ");
                } else if (line.condjmp.relop == RELOP_EQ) {
                    fprintf(file, " == ");
                } else if (line.condjmp.relop == RELOP_NEQ) {
                    fprintf(file, " != ");
                }
                print_label(line.condjmp.y, file);
                fprintf(file, " GOTO ");
                print_label(line.condjmp.z, file);
                fprintf(file, "\n");
                break;
            case CODE_DEC:
                fprintf(file, "DEC ");
                print_label(line.dec.x, file);
                fprintf(file, " %d\n", line.dec.size);
                break;
            default:
                assert(0);
        }
        now = now->next;
    }
    fclose(file);
}
