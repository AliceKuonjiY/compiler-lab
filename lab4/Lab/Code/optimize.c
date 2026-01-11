#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "cfg.h"
#include "ir.h"

extern int temp_cnt;
extern int label_cnt;
extern InterCode IR;
extern CFGNode line_to_node[MAX_IR_LINE];
extern CFGNode nodes[MAX_IR_LINE];
extern int node_count;

static int label_usage[MAX_IR_LINE];
static int visit_node[MAX_IR_LINE];

static void delete_code(InterCode code) {
    if (code->prev != NULL) {
        code->prev->next = code->next;
    } else {
        IR = code->next;
    }
    if (code->next != NULL) {
        code->next->prev = code->prev;
    }
    free(code);
}

static bool equal_label(IRLabel a, IRLabel b) {
    if (a.kind != b.kind) {
        return false;
    }
    if (a.kind == LABEL_CONSTANT) {
        return a.value == b.value;
    } else {
        return strcmp(a.name, b.name) == 0;
    }
}

static bool is_constant(IRLabel label, int value) {
    return label.kind == LABEL_CONSTANT && label.value == value;
}

static bool is_constant_assign(InterCode code, int value) {
    if (code->code.kind == CODE_ASSIGN && code->code.assign.y.kind == LABEL_CONSTANT) {
        return code->code.assign.y.value == value;
    }
    return false;
}

static bool is_binop(InterCode code, int op) {
    if (code->code.kind == CODE_BINOP && code->code.binop.op == op) {
        return true;
    }
    return false;
}

static void count_label_usage(InterCode code) {
    Code c = code->code;
    switch (c.kind) {
        case CODE_ASSIGN:
            if (c.assign.y.kind == LABEL_VARIABLE && c.assign.y.id != -1) {
                label_usage[c.assign.y.id]++;
            }
            break;
        case CODE_BINOP:
            if (c.binop.y.kind == LABEL_VARIABLE && c.binop.y.id != -1) {
                label_usage[c.binop.y.id]++;
            }
            if (c.binop.z.kind == LABEL_VARIABLE && c.binop.z.id != -1) {
                label_usage[c.binop.z.id]++;
            }
            break;
        case CODE_CONDJMP:
            if (c.x.kind == LABEL_VARIABLE && c.x.id != -1) {
                label_usage[c.x.id]++;
            }
            if (c.condjmp.y.kind == LABEL_VARIABLE && c.condjmp.y.id != -1) {
                label_usage[c.condjmp.y.id]++;
            }
            break;
        case CODE_DEREFERENCE:
            if (c.dereference.y.kind == LABEL_VARIABLE && c.dereference.y.id != -1) {
                label_usage[c.dereference.y.id]++;
            }
            break;
        case CODE_REFERENCE:
            if (c.reference.y.kind == LABEL_VARIABLE && c.reference.y.id != -1) {
                label_usage[c.reference.y.id]++;
            }
            break;
        case CODE_LABELOP:
            if (c.x.kind == LABEL_VARIABLE && c.x.id != -1) {
                label_usage[c.x.id]++;
            }
            break;
        default:
            break;
    }
}

void optimize1() {
    InterCode now = IR, pre = NULL;
    while (now != NULL) {
        count_label_usage(now);
        if (pre != NULL) {
            Code code1 = pre->code;
            Code code2 = now->code;
            if (is_constant_assign(pre, 0) &&
                is_binop(now, BINOP_MINUS) &&
                is_constant(now->code.binop.y, 0) &&
                equal_label(code1.x, code2.binop.z)) {
                // x := y
                // t := 0 - x
                // =>
                // t := -y
                pre->code.assign.y.value = 0 - code1.assign.y.value;
                InterCode to_free = now;
                now = now->next;
                delete_code(to_free);
                continue;
            }
        }
        pre = now;
        now = now->next;
    }
    now = IR;
    while (now != NULL) {
        Code code = now->code;
        if (code.kind == CODE_ASSIGN || code.kind == CODE_BINOP ||
            code.kind == CODE_DEREFERENCE || code.kind == CODE_REFERENCE) {
            int id = code.x.id;
            if (code.x.kind == LABEL_VARIABLE && id != -1 && label_usage[id] == 0) {
                InterCode to_free = now;
                now = now->next;
                delete_code(to_free);
                continue;
            }
        }
        now = now->next;
    }
}

static void dfs_cfg(CFGNode node) {
    if (visit_node[node->id]) {
        return;
    }
    visit_node[node->id] = 1;
    CFGNodeList succ = node->succ;
    while (succ != NULL) {
        dfs_cfg(succ->node);
        succ = succ->next;
    }
}

void optimize2() {
    construct_cfg();
    memset(visit_node, 0, sizeof(visit_node));
    for (int i = 1; i <= node_count; i++) {
        if (nodes[i]->begin->code.kind == CODE_LABELOP &&
            nodes[i]->begin->code.labelop.op == LABELOP_FUNCTION) {
            dfs_cfg(nodes[i]);
        }
        if (nodes[i]->begin == IR) {
            dfs_cfg(nodes[i]);
        }
    }
    InterCode now = IR;
    while (now != NULL) {
        CFGNode node = line_to_node[now->line];
        if (!visit_node[node->id]) {
            InterCode to_free = now;
            now = now->next;
            delete_code(to_free);
        } else {
            now = now->next;
        }
    }
}

void optimize() {
    optimize1();
    optimize2();
}