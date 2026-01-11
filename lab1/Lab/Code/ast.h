#include <stdint.h>

enum NodeType { NTML, NVL, VL };
enum RelopType { RELOP_LS, RELOP_GT, RELOP_LEQ, RELOP_GEQ, RELOP_EQ, RELOP_NEQ };
enum TypeType { TYPE_INT, TYPE_FLOAT };

struct Node {
    char *nodeName;
    enum NodeType nodeType;
    int lineNum;
    union {
        uint32_t valInt;
        float valFloat;
        char *valStr;
        enum RelopType valRelop;
        enum TypeType valType;
    };
    struct Node *firstChild;
    struct Node *next;
};

typedef struct Node* NodeP;

NodeP constructNode(char *name, enum NodeType type, int line);
void construct(NodeP parent, int child_count, ...);
void printTree(NodeP root, int space);
