#include <stdlib.h>
#include <string.h>
#include "cfg.h"
#include "ir.h"

extern InterCode IR;

int label_to_line[MAX_IR_LINE];
CFGNode line_to_node[MAX_IR_LINE];
CFGNode nodes[MAX_IR_LINE];
int node_count = 0;

static CFGNode new_node(InterCode begin, InterCode end) {
    CFGNode node = malloc(sizeof(struct CFGNode_));
    nodes[++node_count] = node;
    node->id = node_count;
    node->begin = begin;
    node->end = end;
    node->succ = NULL;
    node->pred = NULL;
    for (int i = begin->line; i <= end->line; i++) {
        line_to_node[i] = node;
    }
    return node;
}

static void insert_node_list(CFGNodeList *list, CFGNode node) {
    CFGNodeList new_elem = malloc(sizeof(struct CFGNodeList_));
    new_elem->node = node;
    new_elem->next = *list;
    *list = new_elem;
}

static void add_edge(CFGNode from, CFGNode to) {
    if (from == NULL || to == NULL) {
        return;
    }
    insert_node_list(&from->succ, to);
    insert_node_list(&to->pred, from);
}

void construct_cfg() {
    memset(label_to_line, 0, sizeof(label_to_line));
    memset(line_to_node, 0, sizeof(line_to_node));
    InterCode now = IR, pre = NULL;
    InterCode begin = IR;
    line_to_node[0] = NULL;
    while (now != NULL) {
        Code code = now->code;
        if (code.kind == CODE_LABELOP && code.labelop.op == LABELOP_DEFLABEL) {
            label_to_line[code.x.id] = now->line;
        }
        if (now != IR) {
            if (code.kind == CODE_LABELOP && code.labelop.op == LABELOP_DEFLABEL){
                CFGNode node = new_node(begin, pre);
                begin = now;
            }
            if (code.kind == CODE_LABELOP && code.labelop.op == LABELOP_FUNCTION) {
                CFGNode node = new_node(begin, pre);
                begin = now;
            }
            if (pre != NULL) {
                Code pre_code = pre->code;
                if ((pre_code.kind == CODE_LABELOP &&
                    (pre_code.labelop.op == LABELOP_GOTO || pre_code.labelop.op == LABELOP_RETURN)) ||
                    pre_code.kind == CODE_CONDJMP) {
                    CFGNode node = new_node(begin, pre);
                    begin = now;
                }
            }
        }
        pre = now;
        now = now->next;
    }
    CFGNode node = new_node(begin, pre);
    for (int i = 1; i <= node_count; i++) {
        CFGNode curr = nodes[i];
        InterCode end_code = curr->end;
        if (end_code->code.kind == CODE_LABELOP && end_code->code.labelop.op == LABELOP_GOTO) {
            int target_line = label_to_line[end_code->code.x.id];
            CFGNode target_node = line_to_node[target_line];
            add_edge(curr, target_node);
        } else if (end_code->code.kind == CODE_LABELOP && end_code->code.labelop.op == LABELOP_RETURN) {
            continue;
        } else if (end_code->code.kind == CODE_CONDJMP) {
            int target_line1 = label_to_line[end_code->code.condjmp.z.id];
            CFGNode target_node1 = line_to_node[target_line1];
            add_edge(curr, target_node1);
            if (curr->end->next != NULL) {
                CFGNode target_node2 = line_to_node[curr->end->next->line];
                add_edge(curr, target_node2);
            }
        } else {
            if (curr->end->next != NULL) {
                CFGNode target_node = line_to_node[curr->end->next->line];
                add_edge(curr, target_node);
            }
        }
    }
}