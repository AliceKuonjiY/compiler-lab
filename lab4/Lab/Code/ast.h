#include <stdint.h>

typedef struct ASTNode_* ASTNode;

enum NodeType { NTML, NVL, VL };
enum RelopType { RELOP_LT, RELOP_GT, RELOP_LE, RELOP_GE, RELOP_EQ, RELOP_NEQ };
enum TypeType { TOKEN_INT, TOKEN_FLOAT };

struct ASTNode_ {
    char *nodeName;
    enum NodeType nodeType;
    int lineNum;
    union {
        uint32_t valInt;
        float valFloat;
        char *valStr;
    };
    ASTNode firstChild;
    ASTNode next;
};

ASTNode constructNode(char *name, enum NodeType type, int line);
void construct(ASTNode parent, int child_count, ...);
void print_ast(ASTNode root, int space);
ASTNode getChild(ASTNode parent, int index);
