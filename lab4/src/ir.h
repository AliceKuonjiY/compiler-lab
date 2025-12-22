#ifndef CODE_H
#define CODE_H

#include <stdint.h>
#include "list.h"

#define MAX_IR_LINE 100000

typedef struct IRLabel IRLabel;
typedef struct Code Code;
typedef struct LabelList_ * LabelList;

enum IRLabelKind { LABEL_VARIABLE, LABEL_CONSTANT, LABEL_FUNCTION, LABEL_LABEL };
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
enum IRLabelOp {
    LABELOP_DEFLABEL,
    LABELOP_FUNCTION,
    LABELOP_GOTO,
    LABELOP_RETURN,
    LABELOP_ARG,
    LABELOP_PARAM,
    LABELOP_READ,
    LABELOP_WRITE
};

struct IRLabel {
    enum IRLabelKind kind;
    union {
        char *name;
        uint32_t value;
    };
    int id;
};

struct LabelList_ {
    IRLabel label;
    LabelList next;
};

struct Code {
    ListHead list;
    int line;
    enum CodeKind kind;
    IRLabel x;
    union {
        struct {
            IRLabel y;
        } assign;
        struct {
            enum DeOp op;
            IRLabel y;
        } dereference;
        struct {
            IRLabel y;
        } reference;
        struct {
            enum BinOp op;
            IRLabel y, z;
        } binop;
        struct {
            enum IRLabelOp op;
        } labelop;
        struct {
            IRLabel f;
        } call;
        struct {
            int relop;
            IRLabel y, z;
        } condjmp;
        struct {
            int size;
        } dec;
    };
};

IRLabel new_temp();
IRLabel new_label();
ListHead *new_assign(IRLabel x, IRLabel y);
ListHead *new_dereference(int op, IRLabel x, IRLabel y);
ListHead *new_reference(IRLabel x, IRLabel y);
ListHead *new_binop(int op, IRLabel x, IRLabel y, IRLabel z);
ListHead *new_labelop(int op, IRLabel x);
ListHead *new_call(IRLabel x, IRLabel f);
ListHead *new_condjmp(int relop, IRLabel x, IRLabel y, IRLabel z);
ListHead *new_dec(IRLabel x, int size);
void print_ir(InterCode code);
void contactIR(InterCode *code1, InterCode *code2);

#endif //CODE_H