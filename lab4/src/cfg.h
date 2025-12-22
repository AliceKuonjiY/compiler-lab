#include "ir.h"

typedef struct CFGNode_* CFGNode;
typedef struct CFGNodeList_* CFGNodeList;

struct CFGNode_ {
    int id;
    InterCode begin, end;
    CFGNodeList succ, pred;
};

struct CFGNodeList_ {
    CFGNode node;
    CFGNodeList next;
};

void construct_cfg();