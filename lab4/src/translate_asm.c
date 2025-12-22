#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "ast.h"
#include "ir.h"
#include "type.h"
#include "asm.h"

#define INF 0x3f3f3f3f

extern InterCode IR;

ListHead ASM;

struct offset_table {
    ListHead list;
    char *name;
    int offset;
};

ListHead offset_table_head;

static ListHead *translate_assign(InterCode ir);
static ListHead *translate_binop(InterCode ir);
static ListHead *translate_labelop(InterCode ir);
static ListHead *translate_call(InterCode ir);
static ListHead *translate_condjmp(InterCode ir);
static ListHead *translate_dereference(InterCode ir);
static ListHead *translate_reference(InterCode ir);
static ListHead *translate_function(InterCode ir);
static ListHead *translate_dec(InterCode ir);
static void insert_offset(char *name, int offset);
static int lookup_offset(char *name);

static void insert_offset(char *name, int offset) {
    struct offset_table *new_entry = (struct offset_table*)malloc(sizeof(struct offset_table));
    new_entry->name = name;
    new_entry->offset = offset;
    list_add_tail(&offset_table_head, &new_entry->list);
}

static int lookup_offset(char *name) {
    struct offset_table *pos;
    list_for_each_entry(pos, &offset_table_head, struct offset_table, list) {
        if (strcmp(pos->name, name) == 0) {
            return pos->offset;
        }
    }
    return INF;
}

void translate_asm() {
    init_list_head(&ASM);
    InterCode nowIR = IR;
    while (nowIR != NULL) {
        if (nowIR->code.kind == CODE_LABELOP && nowIR->code.labelop.op == LABELOP_FUNCTION) {
            ListHead *code = translate_function(nowIR);
            list_contact(&ASM, code);
        }
        nowIR = nowIR->next;
    }
}

static ListHead *translate_function(InterCode ir) {
    init_list_head(&offset_table_head);
    ListHead *function_code = new_list_head();
    InterCode nowIR = ir->next;
    int size = 24, param_cnt = 0;
    while (nowIR != NULL && !(nowIR->code.kind == CODE_LABELOP && nowIR->code.labelop.op == LABELOP_FUNCTION)) {
        if (nowIR->code.kind == CODE_ASSIGN ||
            nowIR->code.kind == CODE_BINOP ||
            nowIR->code.kind == CODE_CALL ||
            nowIR->code.kind == CODE_DEREFERENCE ||
            nowIR->code.kind == CODE_REFERENCE) {
            if (lookup_offset(nowIR->code.x.name) == INF) {
                size += 4;
                insert_offset(nowIR->code.x.name, -size);
            }
        } else if (nowIR->code.kind == CODE_LABELOP) {
            if (nowIR->code.labelop.op == LABELOP_PARAM) {
                param_cnt++;
                if (param_cnt <= 4) {
                    insert_offset(nowIR->code.x.name, -8 - param_cnt * 4);
                } else {
                    if (lookup_offset(nowIR->code.x.name) == INF) {
                        size += 4;
                        insert_offset(nowIR->code.x.name, -size);
                    }
                }
            }
            if (nowIR->code.labelop.op == LABELOP_READ) {
                size += 4;
                insert_offset(nowIR->code.x.name, -size);
            }
        } else if (nowIR->code.kind == CODE_DEC) {
            size += nowIR->code.dec.size;
            size += 4;
            insert_offset(nowIR->code.x.name, -size);
        }
        nowIR = nowIR->next;
    }

    // 函数入口
    TargetCode *func_code = new_target_code(OP_LABEL, 0, 0, 0, 0, 0, ir->code.x.name);
    list_add_tail(function_code, &func_code->list);

    // 分配栈空间
    TargetCode *addi_sp = new_target_code(OP_ADDI, SP, SP, 0, 0, -size, 0);
    list_add_tail(function_code, &addi_sp->list);

    // 保存返回地址和旧帧指针
    TargetCode *sw_ra = new_target_code(OP_SW, SP, RA, 0, size - 4, 0, 0);
    list_add_tail(function_code, &sw_ra->list);
    TargetCode *sw_fp = new_target_code(OP_SW, SP, FP, 0, size - 8, 0, 0);
    list_add_tail(function_code, &sw_fp->list);

    // 设置新帧指针
    TargetCode *addi_fp = new_target_code(OP_ADDI, FP, SP, 0, 0, size, 0);
    list_add_tail(function_code, &addi_fp->list);

    // 保存函数参数到栈
    for (int i = A0; i <= A3; i++) {
        TargetCode *sw_ai = new_target_code(OP_SW, FP, i, 0, -8 - (i - A0 + 1) * 4, 0, 0);
        list_add_tail(function_code, &sw_ai->list);
    }
    if (param_cnt > 4) {
        int arg_num = 0;
        nowIR = ir->next;
        while (nowIR != NULL && !(nowIR->code.kind == CODE_LABELOP && nowIR->code.labelop.op == LABELOP_FUNCTION)) {
            if (nowIR->code.kind == CODE_LABELOP && nowIR->code.labelop.op == LABELOP_PARAM) {
                arg_num++;
                if (arg_num > 4) {
                    TargetCode *lw_t0 = new_target_code(OP_LW, T0, FP, 0, (arg_num - 5) * 4, 0, 0);
                    TargetCode *sw_t0 = new_target_code(OP_SW, FP, T0, 0, -8 - arg_num * 4, 0, 0);
                    list_add_tail(function_code, &lw_t0->list);
                    list_add_tail(function_code, &sw_t0->list);
                }
            }
            nowIR = nowIR->next;
        }
    }

    // 翻译函数体
    nowIR = ir->next;
    while (nowIR != NULL && !(nowIR->code.kind == CODE_LABELOP && nowIR->code.labelop.op == LABELOP_FUNCTION)) {
        if (nowIR->code.kind == CODE_ASSIGN) {
            ListHead *code = translate_assign(nowIR);
            list_contact(function_code, code);
        } else if (nowIR->code.kind == CODE_BINOP) {
            ListHead *code = translate_binop(nowIR);
            list_contact(function_code, code);
        } else if (nowIR->code.kind == CODE_LABELOP) {
            ListHead *code = translate_labelop(nowIR);
            list_contact(function_code, code);
        } else if (nowIR->code.kind == CODE_CALL) {
            ListHead *code = translate_call(nowIR);
            list_contact(function_code, code);
        } else if (nowIR->code.kind == CODE_CONDJMP) {
            ListHead *code = translate_condjmp(nowIR);
            list_contact(function_code, code);
        } else if (nowIR->code.kind == CODE_DEREFERENCE) {
            ListHead *code = translate_dereference(nowIR);
            list_contact(function_code, code);
        } else if (nowIR->code.kind == CODE_REFERENCE) {
            ListHead *code = translate_reference(nowIR);
            list_contact(function_code, code);
        } else if (nowIR->code.kind == CODE_DEC) {
            ListHead *code = translate_dec(nowIR);
            list_contact(function_code, code);
        }
        nowIR = nowIR->next;
    }
    TargetCode *nop = new_target_code(OP_NOP, 0, 0, 0, 0, 0, 0);
    list_add_tail(function_code, &nop->list);
    return function_code;
}

static ListHead *translate_assign(InterCode ir) {
    ListHead *assign_code = new_list_head();
    if (ir->code.assign.y.kind == LABEL_CONSTANT) {
        TargetCode *li = new_target_code(OP_LI, T0, 0, 0, 0, ir->code.assign.y.value, 0);
        list_add_tail(assign_code, &li->list);
    } else if (ir->code.assign.y.kind == LABEL_VARIABLE) {
        TargetCode *lw_t0 = new_target_code(OP_LW, T0, FP, 0, lookup_offset(ir->code.assign.y.name), 0, 0);
        list_add_tail(assign_code, &lw_t0->list);
    }
    TargetCode *sw_t0 = new_target_code(OP_SW, FP, T0, 0, lookup_offset(ir->code.x.name), 0, 0);
    list_add_tail(assign_code, &sw_t0->list);
    return assign_code;
}

static ListHead *translate_binop(InterCode ir) {
    ListHead *binop_code = new_list_head();
    if (ir->code.binop.y.kind == LABEL_CONSTANT) {
        TargetCode *li = new_target_code(OP_LI, T0, 0, 0, 0, ir->code.binop.y.value, 0);
        list_add_tail(binop_code, &li->list);
    } else if (ir->code.binop.y.kind == LABEL_VARIABLE) {
        TargetCode *lw_t0 = new_target_code(OP_LW, T0, FP, 0, lookup_offset(ir->code.binop.y.name), 0, 0);
        list_add_tail(binop_code, &lw_t0->list);
    }
    if (ir->code.binop.z.kind == LABEL_CONSTANT) {
        TargetCode *li = new_target_code(OP_LI, T1, 0, 0, 0, ir->code.binop.z.value, 0);
        list_add_tail(binop_code, &li->list);
    } else if (ir->code.binop.z.kind == LABEL_VARIABLE) {
        TargetCode *lw_t1 = new_target_code(OP_LW, T1, FP, 0, lookup_offset(ir->code.binop.z.name), 0, 0);
        list_add_tail(binop_code, &lw_t1->list);
    }
    if (ir->code.binop.op == BINOP_PLUS) {
        TargetCode *add = new_target_code(OP_ADD, T2, T0, T1, 0, 0, 0);
        list_add_tail(binop_code, &add->list);
    } else if (ir->code.binop.op == BINOP_MINUS) {
        TargetCode *sub = new_target_code(OP_SUB, T2, T0, T1, 0, 0, 0);
        list_add_tail(binop_code, &sub->list);
    } else if (ir->code.binop.op == BINOP_STAR) {
        TargetCode *mul = new_target_code(OP_MUL, T2, T0, T1, 0, 0, 0);
        list_add_tail(binop_code, &mul->list);
    } else if (ir->code.binop.op == BINOP_DIV) {
        TargetCode *div = new_target_code(OP_DIV, 0, T0, T1, 0, 0, 0);
        TargetCode *mflo = new_target_code(OP_MFLO, T2, 0, 0, 0, 0, 0);
        list_add_tail(binop_code, &div->list);
        list_add_tail(binop_code, &mflo->list);
    }
    TargetCode *sw_t2 = new_target_code(OP_SW, FP, T2, 0, lookup_offset(ir->code.x.name), 0, 0);
    list_add_tail(binop_code, &sw_t2->list);
    return binop_code;
}

static ListHead *translate_labelop(InterCode ir) {
    ListHead *labelop_code = new_list_head();
    if (ir->code.labelop.op == LABELOP_DEFLABEL) {
        TargetCode *label = new_target_code(OP_LABEL, 0, 0, 0, 0, 0, ir->code.x.name);
        list_add_tail(labelop_code, &label->list);
    } else if (ir->code.labelop.op == LABELOP_GOTO) {
        TargetCode *j = new_target_code(OP_J, 0, 0, 0, 0, 0, ir->code.x.name);
        list_add_tail(labelop_code, &j->list);
    } else if (ir->code.labelop.op == LABELOP_RETURN) {
        TargetCode *lw_v0 = new_target_code(OP_LW, V0, FP, 0, lookup_offset(ir->code.x.name), 0, 0);
        list_add_tail(labelop_code, &lw_v0->list);
        TargetCode *lw_ra = new_target_code(OP_LW, RA, FP, 0, -4, 0, 0);
        list_add_tail(labelop_code, &lw_ra->list);
        TargetCode *move = new_target_code(OP_MOVE, SP, FP, 0, 0, 0, 0);
        list_add_tail(labelop_code, &move->list);
        TargetCode *lw_fp = new_target_code(OP_LW, FP, FP, 0, -8, 0, 0);
        list_add_tail(labelop_code, &lw_fp->list);
        TargetCode *jr = new_target_code(OP_JR, RA, 0, 0, 0, 0, 0);
        list_add_tail(labelop_code, &jr->list);
    } else if (ir->code.labelop.op == LABELOP_READ) {
        TargetCode *la = new_target_code(OP_LA, A0, 0, 0, 0, 0, "_prompt");
        TargetCode *li = new_target_code(OP_LI, V0, 0, 0, 0, 4, 0);
        TargetCode *syscall = new_target_code(OP_SYSCALL, 0, 0, 0, 0, 0, 0);
        list_add_tail(labelop_code, &la->list);
        list_add_tail(labelop_code, &li->list);
        list_add_tail(labelop_code, &syscall->list);
        li = new_target_code(OP_LI, V0, 0, 0, 0, 5, 0);
        syscall = new_target_code(OP_SYSCALL, 0, 0, 0, 0, 0, 0);
        TargetCode *sw_v0 = new_target_code(OP_SW, FP, V0, 0, lookup_offset(ir->code.x.name), 0, 0);
        list_add_tail(labelop_code, &li->list);
        list_add_tail(labelop_code, &syscall->list);
        list_add_tail(labelop_code, &sw_v0->list);
    } else if (ir->code.labelop.op == LABELOP_WRITE) {
        TargetCode *lw_a0 = new_target_code(OP_LW, A0, FP, 0, lookup_offset(ir->code.x.name), 0, 0);
        TargetCode *li = new_target_code(OP_LI, V0, 0, 0, 0, 1, 0);
        TargetCode *syscall = new_target_code(OP_SYSCALL, 0, 0, 0, 0, 0, 0);
        list_add_tail(labelop_code, &lw_a0->list);
        list_add_tail(labelop_code, &li->list);
        list_add_tail(labelop_code, &syscall->list);
        TargetCode *la = new_target_code(OP_LA, A0, 0, 0, 0, 0, "_ret");
        li = new_target_code(OP_LI, V0, 0, 0, 0, 4, 0);
        syscall = new_target_code(OP_SYSCALL, 0, 0, 0, 0, 0, 0);
        list_add_tail(labelop_code, &la->list);
        list_add_tail(labelop_code, &li->list);
        list_add_tail(labelop_code, &syscall->list);
    }
    return labelop_code;
}

static ListHead *translate_dereference(InterCode ir) {
    ListHead *deref_code = new_list_head();
    if (ir->code.dereference.op == DE_RIGHT) {
        TargetCode *lw_t0 = new_target_code(OP_LW, T0, FP, 0, lookup_offset(ir->code.dereference.y.name), 0, 0);
        TargetCode *lw_t1 = new_target_code(OP_LW, T1, T0, 0, 0, 0, 0);
        TargetCode *sw_t1 = new_target_code(OP_SW, FP, T1, 0, lookup_offset(ir->code.x.name), 0, 0);
        list_add_tail(deref_code, &lw_t0->list);
        list_add_tail(deref_code, &lw_t1->list);
        list_add_tail(deref_code, &sw_t1->list);
    } else if (ir->code.dereference.op == DE_LEFT) {
        TargetCode *lw_t0 = new_target_code(OP_LW, T0, FP, 0, lookup_offset(ir->code.x.name), 0, 0);
        TargetCode *lw_t1 = new_target_code(OP_LW, T1, FP, 0, lookup_offset(ir->code.dereference.y.name), 0, 0);
        TargetCode *sw_t1 = new_target_code(OP_SW, T0, T1, 0, 0, 0, 0);
        list_add_tail(deref_code, &lw_t0->list);
        list_add_tail(deref_code, &lw_t1->list);
        list_add_tail(deref_code, &sw_t1->list);
    }
    return deref_code;
}

static ListHead *translate_reference(InterCode ir) {
    ListHead *ref_code = new_list_head();
    TargetCode *addi_t0 = new_target_code(OP_ADDI, T0, FP, 0, 0, lookup_offset(ir->code.reference.y.name), 0);
    TargetCode *sw_t0 = new_target_code(OP_SW, FP, T0, 0, lookup_offset(ir->code.x.name), 0, 0);
    list_add_tail(ref_code, &addi_t0->list);
    list_add_tail(ref_code, &sw_t0->list);
    return ref_code;
}

static ListHead *translate_condjmp(InterCode ir) {
    ListHead *condjmp_code = new_list_head();
    if (ir->code.x.kind == LABEL_CONSTANT) {
        TargetCode *li = new_target_code(OP_LI, T0, 0, 0, 0, ir->code.x.value, 0);
        list_add_tail(condjmp_code, &li->list);
    } else if (ir->code.x.kind == LABEL_VARIABLE) {
        TargetCode *lw_t0 = new_target_code(OP_LW, T0, FP, 0, lookup_offset(ir->code.x.name), 0, 0);
        list_add_tail(condjmp_code, &lw_t0->list);
    }
    if (ir->code.condjmp.y.kind == LABEL_CONSTANT) {
        TargetCode *li = new_target_code(OP_LI, T1, 0, 0, 0, ir->code.condjmp.y.value, 0);
        list_add_tail(condjmp_code, &li->list);
    } else if (ir->code.condjmp.y.kind == LABEL_VARIABLE) {
        TargetCode *lw_t1 = new_target_code(OP_LW, T1, FP, 0, lookup_offset(ir->code.condjmp.y.name), 0, 0);
        list_add_tail(condjmp_code, &lw_t1->list);
    }
    if (ir->code.condjmp.relop == RELOP_EQ) {
        TargetCode *beq = new_target_code(OP_BEQ, 0, T0, T1, 0, 0, ir->code.condjmp.z.name);
        list_add_tail(condjmp_code, &beq->list);
    } else if (ir->code.condjmp.relop == RELOP_NEQ) {
        TargetCode *bne = new_target_code(OP_BNE, 0, T0, T1, 0, 0, ir->code.condjmp.z.name);
        list_add_tail(condjmp_code, &bne->list);
    } else if (ir->code.condjmp.relop == RELOP_LT) {
        TargetCode *blt = new_target_code(OP_BLT, 0, T0, T1, 0, 0, ir->code.condjmp.z.name);
        list_add_tail(condjmp_code, &blt->list);
    } else if (ir->code.condjmp.relop == RELOP_GT) {
        TargetCode *bgt = new_target_code(OP_BGT, 0, T0, T1, 0, 0, ir->code.condjmp.z.name);
        list_add_tail(condjmp_code, &bgt->list);
    } else if (ir->code.condjmp.relop == RELOP_LE) {
        TargetCode *ble = new_target_code(OP_BLE, 0, T0, T1, 0, 0, ir->code.condjmp.z.name);
        list_add_tail(condjmp_code, &ble->list);
    } else if (ir->code.condjmp.relop == RELOP_GE) {
        TargetCode *bge = new_target_code(OP_BGE, 0, T0, T1, 0, 0, ir->code.condjmp.z.name);
        list_add_tail(condjmp_code, &bge->list);
    }
    return condjmp_code;
}

static ListHead *translate_call(InterCode ir) {
    ListHead *call_code = new_list_head();
    
    // 保存调用者保存寄存器
    TargetCode *addi_sp = new_target_code(OP_ADDI, SP, SP, 0, 0, -12, 0);
    list_add_tail(call_code, &addi_sp->list);
    for (int i = T0; i <= T2; i++) {
        TargetCode *sw_ti = new_target_code(OP_SW, SP, i, 0, (i - T0) * 4, 0, 0);
        list_add_tail(call_code, &sw_ti->list);
    }

    // 传递参数
    InterCode nowIR = ir->prev;
    int arg_num = 0;
    while (nowIR != NULL && nowIR->code.kind == CODE_LABELOP && nowIR->code.labelop.op == LABELOP_ARG) {
        arg_num++;
        nowIR = nowIR->prev;
    }
    if (arg_num > 4) {
        addi_sp = new_target_code(OP_ADDI, SP, SP, 0, 0, -4 * (arg_num - 4), 0);
        list_add_tail(call_code, &addi_sp->list);
    }
    nowIR = ir->prev;
    arg_num = 0;
    while (nowIR != NULL && nowIR->code.kind == CODE_LABELOP && nowIR->code.labelop.op == LABELOP_ARG) {
        if (arg_num < 4) {
            if (nowIR->code.x.kind == LABEL_CONSTANT) {
                TargetCode *li = new_target_code(OP_LI, A0 + arg_num, 0, 0, 0, nowIR->code.x.value, 0);
                list_add_tail(call_code, &li->list);
            } else if (nowIR->code.x.kind == LABEL_VARIABLE) {
                TargetCode *lw_ai = new_target_code(OP_LW, A0 + arg_num, FP, 0, lookup_offset(nowIR->code.x.name), 0, 0);
                list_add_tail(call_code, &lw_ai->list);
            }
        } else {
            if (nowIR->code.x.kind == LABEL_CONSTANT) {
                TargetCode *li = new_target_code(OP_LI, T0, 0, 0, 0, nowIR->code.x.value, 0);
                TargetCode *sw_t0 = new_target_code(OP_SW, SP, T0, 0, (arg_num - 4) * 4, 0, 0);
                list_add_tail(call_code, &li->list);
                list_add_tail(call_code, &sw_t0->list);
            } else if (nowIR->code.x.kind == LABEL_VARIABLE) {
                TargetCode *lw_t0 = new_target_code(OP_LW, T0, FP, 0, lookup_offset(nowIR->code.x.name), 0, 0);
                TargetCode *sw_t0 = new_target_code(OP_SW, SP, T0, 0, (arg_num - 4) * 4, 0, 0);
                list_add_tail(call_code, &lw_t0->list);
                list_add_tail(call_code, &sw_t0->list);
            }
        }
        arg_num++;
        nowIR = nowIR->prev;
    }

    // 调用函数
    TargetCode *jal = new_target_code(OP_JAL, 0, 0, 0, 0, 0, ir->code.call.f.name);
    list_add_tail(call_code, &jal->list);

    // 恢复栈空间
    for (int i = T0; i <= T2; i++) {
        TargetCode *lw_ti = new_target_code(OP_LW, i, SP, 0, (i - T0) * 4, 0, 0);
        list_add_tail(call_code, &lw_ti->list);
    }
    addi_sp = new_target_code(OP_ADDI, SP, SP, 0, 0, 12, 0);
    list_add_tail(call_code, &addi_sp->list);
    TargetCode *sw_v0 = new_target_code(OP_SW, FP, V0, 0, lookup_offset(ir->code.x.name), 0, 0);
    list_add_tail(call_code, &sw_v0->list);
    return call_code;
}

static ListHead *translate_dec(InterCode ir) {
    ListHead *dec_code = new_list_head();
    TargetCode *addi = new_target_code(OP_ADDI, T0, FP, 0, 0, lookup_offset(ir->code.x.name) + 4, 0);
    TargetCode *sw = new_target_code(OP_SW, FP, T0, 0, lookup_offset(ir->code.x.name), 0, 0);
    list_add_tail(dec_code, &addi->list);
    list_add_tail(dec_code, &sw->list);
    return dec_code;
}