typedef struct Label Label;
typedef struct Code Code;
typedef struct InterCode_* InterCode;
typedef struct LabelList_* LabelList;

enum LabelKind { LABEL_VARIABLE, LABEL_CONSTANT, LABEL_FUNCTION, LABEL_LABEL };
enum CodeKind {
    CODE_ASSIGN,
    CODE_BINOP,
    CODE_LABELOP,
    CODE_CALL,
    CODE_CONDJMP,
    CODE_DEREFERENCE,
    CODE_DEC,
    CODE_REFERENCE
};
enum DeOp { DE_LEFT, DE_RIGHT };
enum BinOp { BINOP_PLUS, BINOP_MINUS, BINOP_STAR, BINOP_DIV };
enum LabelOp {
    LABELOP_DEFLABEL,
    LABELOP_FUNCTION,
    LABELOP_GOTO,
    LABELOP_RETURN,
    LABELOP_ARG,
    LABELOP_PARAM,
    LABELOP_READ,
    LABELOP_WRITE
};

struct Label {
    enum LabelKind kind;
    union {
        char *name;
        int value;
    };
};

struct LabelList_ {
    Label label;
    LabelList next;
};

struct Code {
    enum CodeKind kind;
    union {
        struct {
            Label x, y;
        } assign;
        struct {
            enum DeOp op;
            Label x, y;
        } dereference;
        struct {
            Label x, y;
        } reference;
        struct {
            enum BinOp op;
            Label x, y, z;
        } binop;
        struct {
            enum LabelOp op;
            Label x;
        } labelop;
        struct {
            Label x, f;
        } call;
        struct {
            int relop;
            Label x, y, z;
        } condjmp;
        struct {
            Label x;
            int size;
        } dec;
    };
};

struct InterCode_ {
    Code code;
    InterCode prev, next;
};

Label new_temp();
Label new_label();
InterCode new_assign(Label x, Label y);
InterCode new_dereference(int op, Label x, Label y);
InterCode new_reference(Label x, Label y);
InterCode new_binop(int op, Label x, Label y, Label z);
InterCode new_labelop(int op, Label x);
InterCode new_call(Label x, Label f);
InterCode new_condjmp(int relop, Label x, Label y, Label z);
InterCode new_dec(Label x, int size);
void print_ir(InterCode code, const char *output_path);
void contact(InterCode *code1, InterCode *code2);
