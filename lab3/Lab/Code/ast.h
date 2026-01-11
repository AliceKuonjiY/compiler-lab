#include <stdint.h>

typedef struct Node_* Node;

enum NodeType { NTML, NVL, VL };
enum RelopType { RELOP_LS, RELOP_GT, RELOP_LEQ, RELOP_GEQ, RELOP_EQ, RELOP_NEQ };
enum TypeType { TOKEN_INT, TOKEN_FLOAT };

struct Node_ {
    char *nodeName;
    enum NodeType nodeType;
    int lineNum;
    union {
        uint32_t valInt;
        float valFloat;
        char *valStr;
    };
    Node firstChild;
    Node next;
};

Node constructNode(char *name, enum NodeType type, int line);
void construct(Node parent, int child_count, ...);
void printTree(Node root, int space);
Node getChild(Node parent, int index);
