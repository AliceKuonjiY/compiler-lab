#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "ast.h"
#include "ir.h"
#include "type.h"
#include "list.h"

static ListHead *ExtDefList(ASTNode node);
static ListHead *ExtDef(ASTNode node);
static ListHead *ExtDecList(ASTNode node, Type type);
static ListHead *FunDec(ASTNode node);
static ListHead *CompSt(ASTNode node);
static ListHead *StmtList(ASTNode node);
static ListHead *Stmt(ASTNode node);
static ListHead *VarList(ASTNode node);
static ListHead *ParamDec(ASTNode node);
static Type StructSpecifier(ASTNode node);
static Type Specifier(ASTNode node);
static ListHead *DefList(ASTNode node, TypeList *head);
static ListHead *Def(ASTNode node, TypeList *head);
static ListHead *DecList(ASTNode node, Type type, TypeList *head);
static ListHead *Dec(ASTNode node, Type type, TypeList *head);
static IRLabel VarDec(ASTNode node, Type *type);
static ListHead *Exp(ASTNode node, IRLabel place, int assign);
static ListHead *Args(ASTNode node, LabelList *head);
static ListHead *Cond(ASTNode node, IRLabel label_true, IRLabel label_false);
static Type get_type(ASTNode node);

ListHead IR;

static TypeList symbol_list = NULL;

void translate_ir(ASTNode root) {
    srand((unsigned)time(NULL));
    init_list_head(&IR);
    ListHead *code = ExtDefList(getChild(root, 0));
    list_contact(&IR, code);
    InterCode now = IR;
    int line = 1;
    while (now != NULL) {
        now->line = line;
        now = now->next;
        line++;
    }
}

static ListHead *ExtDefList(ASTNode node) {
    ListHead *extDefList = new_list_head();
    if (node == NULL) {
        return extDefList;
    }
    ListHead *code1 = ExtDef(getChild(node, 0));
    ListHead *code2 = ExtDefList(getChild(node, 1));
    list_contact(extDefList, code1);
    list_contact(extDefList, code2);
    return extDefList;
}

static ListHead *ExtDef(ASTNode node) {
    ListHead *extDef = new_list_head();
    Type type = Specifier(getChild(node, 0));
    ASTNode second = getChild(node, 1);
    if (strcmp(second->nodeName, "ExtDecList") == 0) { // 变量声明
        return ExtDecList(second, type);
    } else if (strcmp(second->nodeName, "FunDec") == 0) { // 函数定义
        ListHead *code1 = FunDec(second);
        ListHead *code2 = CompSt(getChild(node, 2));
        list_contact(extDef, code1);
        list_contact(extDef, code2);
        return extDef;
    } else { // 仅类型声明
        return extDef;
    }
}

static ListHead *FunDec(ASTNode node) {
    ListHead *code1 = new_labelop(LABELOP_FUNCTION, (IRLabel){ LABEL_FUNCTION, .name = getChild(node, 0)->valStr, -1 });
    if (strcmp(getChild(node, 2)->nodeName, "VarList") == 0) { // 有参数
        ListHead *code2 = VarList(getChild(node, 2));
        list_contact(code1, code2);
    }
    return code1;
}

static InterCode CompSt(ASTNode node) {
    ASTNode defList = NULL, stmtList = NULL;
    if (strcmp(getChild(node, 1)->nodeName, "DefList") == 0) {
        defList = getChild(node, 1);
        if (strcmp(getChild(node, 2)->nodeName, "StmtList") == 0) {
            stmtList = getChild(node, 2);
        }
    } else if (strcmp(getChild(node, 1)->nodeName, "StmtList") == 0) {
        stmtList = getChild(node, 1);
    }
    TypeList head = NULL;
    InterCode code1 = DefList(defList, &head);
    InterCode code2 = StmtList(stmtList);
    contactIR(&code1, &code2);
    return code1;
}

static InterCode StmtList(ASTNode node) {
    if (node == NULL) {
        return NULL;
    }
    InterCode code1 = Stmt(getChild(node, 0));
    InterCode code2 = StmtList(getChild(node, 1));
    contactIR(&code1, &code2);
    return code1;
}

static InterCode Stmt(ASTNode node) {
    ASTNode first = getChild(node, 0);
    if (strcmp(first->nodeName, "Exp") == 0) { // Exp
        IRLabel t1 = new_temp();
        return Exp(first, t1, BOTH);
    } else if (strcmp(first->nodeName, "CompSt") == 0) { // CompSt
        return CompSt(first);
    } else if (strcmp(first->nodeName, "RETURN") == 0) { // return
        IRLabel t1 = new_temp();
        InterCode code1 = Exp(getChild(node, 1), t1, RIGHT);
        InterCode code2 = new_labelop(LABELOP_RETURN, t1);
        contactIR(&code1, &code2);
        return code1;
    } else if (strcmp(first->nodeName, "IF") == 0) { // if
        IRLabel l1 = new_label();
        IRLabel l2 = new_label();
        InterCode code1 = Cond(getChild(node, 2), l1, l2);
        InterCode code2 = new_labelop(LABELOP_DEFLABEL, l1);
        InterCode code3 = Stmt(getChild(node, 4));
        InterCode code4 = new_labelop(LABELOP_DEFLABEL, l2);
        if (getChild(node, 5) != NULL) {
            IRLabel l3 = new_label();
            InterCode code5 = new_labelop(LABELOP_GOTO, l3);
            InterCode code6 = Stmt(getChild(node, 6));
            InterCode code7 = new_labelop(LABELOP_DEFLABEL, l3);
            contactIR(&code3, &code5);
            contactIR(&code6, &code7);
            contactIR(&code4, &code6);
        }
        contactIR(&code3, &code4);
        contactIR(&code2, &code3);
        contactIR(&code1, &code2);
        return code1;
    } else { // while
        IRLabel l1 = new_label();
        IRLabel l2 = new_label();
        IRLabel l3 = new_label();
        InterCode code1 = new_labelop(LABELOP_DEFLABEL, l1);
        InterCode code2 = Cond(getChild(node, 2), l2, l3);
        InterCode code3 = new_labelop(LABELOP_DEFLABEL, l2);
        InterCode code4 = Stmt(getChild(node, 4));
        InterCode code5 = new_labelop(LABELOP_GOTO, l1);
        InterCode code6 = new_labelop(LABELOP_DEFLABEL, l3);
        contactIR(&code5, &code6);
        contactIR(&code4, &code5);
        contactIR(&code3, &code4);
        contactIR(&code2, &code3);
        contactIR(&code1, &code2);
        return code1;
    }
}

static InterCode VarList(ASTNode node) {
    InterCode code1 = ParamDec(getChild(node, 0));
    if (getChild(node, 1) != NULL) {
        InterCode code2 = VarList(getChild(node, 2));
        contactIR(&code1, &code2);
    }
    return code1;
}

static InterCode ParamDec(ASTNode node) {
    Type type = Specifier(getChild(node, 0));
    IRLabel param = VarDec(getChild(node, 1), &type);
    if (type->kind == TYPE_STRUCTURE) {
        Type addressType = malloc(sizeof(struct Type_));
        addressType->kind = TYPE_ADDRESS;
        addressType->info.address.type = type;
        addressType->assign = RIGHT;
        type = addressType;
    }
    if (type->kind == TYPE_ARRAY) {
        printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.");
        exit(1);
    }
    insert_list(&symbol_list, param.name, type);
    InterCode code = new_labelop(LABELOP_PARAM, param);
    return code;
}

static InterCode ExtDecList(ASTNode node, Type type) {
    IRLabel var = VarDec(getChild(node, 0), &type);
    insert_list(&symbol_list, var.name, type);
    InterCode code1 = NULL;
    if (type->kind == TYPE_ARRAY || type->kind == TYPE_STRUCTURE) {
        IRLabel t1 = new_temp();
        code1 = new_dec(t1, calc_size(type));
        InterCode code2 = new_reference(var, t1);
        contactIR(&code1, &code2);
    }
    if (getChild(node, 1) != NULL) {
        InterCode code2 = ExtDecList(getChild(node, 2), type);
        contactIR(&code1, &code2);
    }
    return code1;
}

static Type Specifier(ASTNode node) {
    ASTNode child = getChild(node, 0);
    if (strcmp(child->nodeName, "TYPE") == 0) { // 基本类型
        Type type = malloc(sizeof(struct Type_));
        type->kind = TYPE_BASIC;
        if (child->valInt == TOKEN_INT) {
            type->info.basic = BASIC_INT;
        } else {
            type->info.basic = BASIC_FLOAT;
        }
        return type;
    } else { // 结构体类型
        return StructSpecifier(child);
    }
}

static Type StructSpecifier(ASTNode node) {
    Type type = malloc(sizeof(struct Type_));
    type->kind = TYPE_STRUCTURE;
    type->assign = BOTH;
    ASTNode tag = getChild(node, 1);
    if (strcmp(tag->nodeName, "OptTag") == 0) { // 命名结构体定义
        ASTNode id = getChild(tag, 0);
        ASTNode defList = getChild(node, 3);
        type->info.structure.name = id->valStr;
        if (strcmp(defList->nodeName, "DefList") == 0) {
            DefList(defList, &type->info.structure.domain);
        }
        insert_list(&symbol_list, type->info.structure.name, type);
        return type;
    } else if (strcmp(tag->nodeName, "LC") == 0) { // 匿名结构体定义
        ASTNode defList = getChild(node, 2);
        type->info.structure.name = malloc(20 * sizeof(char));
        sprintf(type->info.structure.name, "anon%d", rand());
        if (strcmp(defList->nodeName, "DefList") == 0) {
            DefList(defList, &type->info.structure.domain);
        }
        insert_list(&symbol_list, type->info.structure.name, type);
        return type;
    } else if (strcmp(tag->nodeName, "Tag") == 0) { // 结构体引用
        ASTNode id = getChild(tag, 0);
        TypeList found = find_list(symbol_list, id->valStr);
        return found->type;
    } else {
        return NULL;
    }
}

static InterCode DefList(ASTNode node, TypeList *head) {
    if (node == NULL) {
        return NULL;
    }
    InterCode code1 = Def(getChild(node, 0), head);
    if (getChild(node, 1) != NULL) {
        InterCode code2 = DefList(getChild(node, 1), head);
        contactIR(&code1, &code2);
    }
    return code1;
}

static InterCode Def(ASTNode node, TypeList *head) {
    Type type = Specifier(getChild(node, 0));
    return DecList(getChild(node, 1), type, head);
}

static InterCode DecList(ASTNode node, Type type, TypeList *head) {
    InterCode code1 = Dec(getChild(node, 0), type, head);
    if (getChild(node, 1) != NULL) {
        InterCode code2 = DecList(getChild(node, 2), type, head);
        contactIR(&code1, &code2);
    }
    return code1;
}

static InterCode Dec(ASTNode node, Type type, TypeList *head) {
    IRLabel var = VarDec(getChild(node, 0), &type);
    insert_list(head, var.name, type);
    insert_list(&symbol_list, var.name, type);
    InterCode code1 = NULL;
    if (type->kind == TYPE_ARRAY || type->kind == TYPE_STRUCTURE) {
        IRLabel t1 = new_temp();
        code1 = new_dec(t1, calc_size(type));
        InterCode code2 = new_reference(var, t1);
        contactIR(&code1, &code2);
    } else {
        code1 = new_assign(var, (IRLabel){ LABEL_CONSTANT, .value = 0, -1 });
    }
    if (getChild(node, 1) != NULL) { // 变量初始化
        InterCode code2 = Exp(getChild(node, 2), var, RIGHT);
        contactIR(&code1, &code2);
    }
    return code1;
}

static IRLabel VarDec(ASTNode node, Type *type) {
    ASTNode first = getChild(node, 0);
    if (strcmp(first->nodeName, "ID") == 0) { // 变量声明
        return (IRLabel){ LABEL_VARIABLE, .name = first->valStr, -1 };
    } else { // 数组声明
        if ((*type)->kind == TYPE_ARRAY) {
            printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.");
            exit(1);
        }
        int size = getChild(node, 2)->valInt;
        Type arrayType = malloc(sizeof(struct Type_));
        arrayType->kind = TYPE_ARRAY;
        arrayType->info.array.size = size;
        arrayType->info.array.elem = *type;
        arrayType->assign = RIGHT;
        *type = arrayType;
        return VarDec(first, type);
    }
}

static InterCode Exp(ASTNode node, IRLabel place, int assign) {
    ASTNode first = getChild(node, 0);
    if (strcmp(first->nodeName, "ID") == 0) { // 变量或函数调用
        if (getChild(node, 1) == NULL) { // 变量
            InterCode code = NULL;
            if (assign == LEFT) {
                code = new_reference(place, (IRLabel){ LABEL_VARIABLE, .name = first->valStr, -1 });
            } else {
                code = new_assign(place, (IRLabel){ LABEL_VARIABLE, .name = first->valStr, -1 });
            }
            return code;
        } else { // 函数
            IRLabel func = (IRLabel){ LABEL_FUNCTION, .name = first->valStr, -1 };
            if (strcmp(getChild(node, 2)->nodeName, "Args") == 0) {
                LabelList args = NULL;
                InterCode code1 = Args(getChild(node, 2), &args);
                if (strcmp(func.name, "write") == 0) {
                    InterCode code2 = new_labelop(LABELOP_WRITE, args->label);
                    InterCode code3 = new_assign(place, (IRLabel){ LABEL_CONSTANT, .value = 0, -1 });
                    contactIR(&code2, &code3);
                    contactIR(&code1, &code2);
                    return code1;
                }
                LabelList arg = args;
                InterCode code2 = NULL;
                while (arg != NULL) {
                    InterCode code_temp = new_labelop(LABELOP_ARG, arg->label);
                    if (code2 == NULL) {
                        code2 = code_temp;
                    } else {
                        contactIR(&code2, &code_temp);
                    }
                    arg = arg->next;
                }
                InterCode code3 = new_call(place, func);
                contactIR(&code2, &code3);
                contactIR(&code1, &code2);
                return code1;
            } else {
                if (strcmp(func.name, "read") == 0) {
                    return new_labelop(LABELOP_READ, place);
                }
                return new_call(place, func);
            }
        }
    } else if (strcmp(first->nodeName, "INT") == 0) { // int 常量
        return new_assign(place, (IRLabel){ LABEL_CONSTANT, .value = first->valInt, -1 });
    } else if (strcmp(first->nodeName, "FLOAT") == 0) { // float 常量
        assert(0);
    } else if (strcmp(first->nodeName, "LP") == 0) { // (Exp)
        return Exp(getChild(node, 1), place, RIGHT);
    } else if (strcmp(first->nodeName, "MINUS") == 0) {
        IRLabel t1 = new_temp();
        InterCode code1 = Exp(getChild(node, 1), t1, RIGHT);
        InterCode code2 = new_binop(BINOP_MINUS, place, (IRLabel){ LABEL_CONSTANT, .value = 0, -1 }, t1);
        contactIR(&code1, &code2);
        return code1;
    } else if (strcmp(first->nodeName, "NOT") == 0) {
        IRLabel l1 = new_label();
        IRLabel l2 = new_label();
        InterCode code1 = new_assign(place, (IRLabel){ LABEL_CONSTANT, .value = 0, -1 });
        InterCode code2 = Cond(node, l1, l2);
        InterCode code3 = new_labelop(LABELOP_DEFLABEL, l1);
        InterCode code4 = new_assign(place, (IRLabel){ LABEL_CONSTANT, .value = 1, -1 });
        InterCode code5 = new_labelop(LABELOP_DEFLABEL, l2);
        contactIR(&code4, &code5);
        contactIR(&code3, &code4);
        contactIR(&code2, &code3);
        contactIR(&code1, &code2);
        return code1;
    } else if (strcmp(first->nodeName, "Exp") == 0) {
        ASTNode second = getChild(node, 1);
        if (strcmp(second->nodeName, "ASSIGNOP") == 0) {
            IRLabel t1 = new_temp();
            IRLabel t2 = new_temp();
            InterCode code1 = Exp(getChild(node, 0), t1, LEFT);
            InterCode code2 = Exp(getChild(node, 2), t2, RIGHT);
            InterCode code3 = new_dereference(DE_LEFT, t1, t2);
            InterCode code4 = new_dereference(DE_RIGHT, place, t1);
            contactIR(&code3, &code4);
            contactIR(&code2, &code3);
            contactIR(&code1, &code2);
            return code1;
        } else if (strcmp(second->nodeName, "AND") == 0 ||
                   strcmp(second->nodeName, "OR") == 0 ||
                   strcmp(second->nodeName, "RELOP") == 0) {
            IRLabel l1 = new_label();
            IRLabel l2 = new_label();
            InterCode code0 = new_assign(place, (IRLabel){ LABEL_CONSTANT, .value = 0, -1 });
            InterCode code1 = Cond(node, l1, l2);
            InterCode code2 = new_labelop(LABELOP_DEFLABEL, l1);
            InterCode code3 = new_assign(place, (IRLabel){ LABEL_CONSTANT, .value = 1, -1 });
            InterCode code4 = new_labelop(LABELOP_DEFLABEL, l2);
            contactIR(&code3, &code4);
            contactIR(&code2, &code3);
            contactIR(&code1, &code2);
            contactIR(&code0, &code1);
            return code0;
        } else if (strcmp(second->nodeName, "PLUS") == 0 ||
                   strcmp(second->nodeName, "MINUS") == 0 ||
                   strcmp(second->nodeName, "STAR") == 0 ||
                   strcmp(second->nodeName, "DIV") == 0) { // Exp op Exp
            IRLabel t1 = new_temp();
            IRLabel t2 = new_temp();
            InterCode code1 = Exp(getChild(node, 0), t1, RIGHT);
            InterCode code2 = Exp(getChild(node, 2), t2, RIGHT);
            InterCode code3 = new_binop(second->valInt, place, t1, t2);
            contactIR(&code2, &code3);
            contactIR(&code1, &code2);
            return code1;
        } else if (strcmp(second->nodeName, "LB") == 0) { // Exp[Exp]
            IRLabel t1 = new_temp();
            IRLabel t2 = new_temp();
            IRLabel t3 = new_temp();
            Type type = get_type(getChild(node, 0));
            if (type->info.array.elem->kind == TYPE_ARRAY) {
                printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.");
                exit(1);
            }
            int offset = calc_size(type->info.array.elem);
            InterCode code1 = Exp(getChild(node, 0), t1, RIGHT);
            InterCode code2 = Exp(getChild(node, 2), t2, RIGHT);
            InterCode code3 = new_binop(BINOP_STAR, t2, t2, (IRLabel){ LABEL_CONSTANT, .value = offset, -1 });
            InterCode code4 = new_binop(BINOP_PLUS, t3, t1, t2);
            InterCode code5 = NULL;
            if (assign == RIGHT) {
                code5 = new_dereference(DE_RIGHT, place, t3);
            } else {
                code5 = new_assign(place, t3);
            }
            contactIR(&code4, &code5);
            contactIR(&code3, &code4);
            contactIR(&code2, &code3);
            contactIR(&code1, &code2);
            return code1;
        } else if (strcmp(second->nodeName, "DOT") == 0) { // Exp.ID
            IRLabel t1 = new_temp();
            IRLabel t2 = new_temp();
            Type type = get_type(getChild(node, 0));
            int local_assign = LEFT;
            if (type->kind == TYPE_ADDRESS) {
                type = type->info.address.type;
                local_assign = RIGHT;
            }
            ASTNode id = getChild(node, 2);
            int offset = calc_offset(type->info.structure.domain, id->valStr);
            InterCode code1 = Exp(getChild(node, 0), t1, local_assign);
            InterCode code2 = new_binop(BINOP_PLUS, t2, t1, (IRLabel){ LABEL_CONSTANT, .value = offset, -1 });
            InterCode code3 = NULL;
            if (assign == RIGHT) {
                code3 = new_dereference(DE_RIGHT, place, t2);
            } else {
                code3 = new_assign(place, t2);
            }
            contactIR(&code2, &code3);
            contactIR(&code1, &code2);
            return code1;
        }
    }
    return NULL;
}

static InterCode Cond(ASTNode node, IRLabel label_true, IRLabel label_false) {
    ASTNode first = getChild(node, 0);
    ASTNode second = getChild(node, 1);
    if (second != NULL && strcmp(second->nodeName, "AND") == 0) {
        IRLabel l1 = new_label();
        InterCode code1 = Cond(getChild(node, 0), l1, label_false);
        InterCode code2 = new_labelop(LABELOP_DEFLABEL, l1);
        InterCode code3 = Cond(getChild(node, 2), label_true, label_false);
        contactIR(&code2, &code3);
        contactIR(&code1, &code2);
        return code1;
    } else if (second != NULL && strcmp(second->nodeName, "OR") == 0) {
        IRLabel l1 = new_label();
        InterCode code1 = Cond(getChild(node, 0), label_true, l1);
        InterCode code2 = new_labelop(LABELOP_DEFLABEL, l1);
        InterCode code3 = Cond(getChild(node, 2), label_true, label_false);
        contactIR(&code2, &code3);
        contactIR(&code1, &code2);
        return code1;
    } else if (second != NULL && strcmp(second->nodeName, "RELOP") == 0) {
        IRLabel t1 = new_temp();
        IRLabel t2 = new_temp();
        InterCode code1 = Exp(getChild(node, 0), t1, RIGHT);
        InterCode code2 = Exp(getChild(node, 2), t2, RIGHT);
        InterCode code3 = new_condjmp(second->valInt, t1, t2, label_true);
        InterCode code4 = new_labelop(LABELOP_GOTO, label_false);
        contactIR(&code3, &code4);
        contactIR(&code2, &code3);
        contactIR(&code1, &code2);
        return code1;
    } else if (strcmp(first->nodeName, "NOT") == 0) {
        return Cond(getChild(node, 1), label_false, label_true);
    } else {
        IRLabel t1 = new_temp();
        InterCode code1 = Exp(node, t1, RIGHT);
        InterCode code2 = new_condjmp(RELOP_NEQ, t1, (IRLabel){ LABEL_CONSTANT, .value = 0, -1 }, label_true);
        InterCode code3 = new_labelop(LABELOP_GOTO, label_false);
        contactIR(&code2, &code3);
        contactIR(&code1, &code2);
        return code1;
    }
}

static InterCode Args(ASTNode node, LabelList *head) {
    IRLabel t1 = new_temp();
    InterCode code1 = NULL; 
    Type type = get_type(getChild(node, 0));
    if (type->kind == TYPE_ARRAY || type->kind == TYPE_STRUCTURE) {
        code1 = Exp(getChild(node, 0), t1, LEFT);
    } else {
        code1 = Exp(getChild(node, 0), t1, RIGHT);
    }
    LabelList list = malloc(sizeof(struct LabelList_));
    list->label = t1;
    list->next = *head;
    *head = list;
    if (getChild(node, 1) != NULL) {
        InterCode code2 = Args(getChild(node, 2), head);
        contactIR(&code1, &code2);
    }
    return code1;
}

static Type get_type(ASTNode node) {
    ASTNode first = getChild(node, 0);
    if (strcmp(first->nodeName, "ID") == 0) { // 变量或函数调用
        TypeList found = find_list(symbol_list, first->valStr);
        if (getChild(node, 1) == NULL) { // 变量
            return found->type;
        } else { // 函数
            Type type = malloc(sizeof(struct Type_));
            type->kind = TYPE_BASIC;
            type->info.basic = BASIC_INT;
            type->assign = RIGHT;
            return type;
        }
    } else if (strcmp(first->nodeName, "INT") == 0) { // int 常量
        Type type = malloc(sizeof(struct Type_));
        type->kind = TYPE_BASIC;
        type->info.basic = BASIC_INT;
        type->assign = RIGHT;
        return type;
    } else if (strcmp(first->nodeName, "FLOAT") == 0) { // float 常量
        Type type = malloc(sizeof(struct Type_));
        type->kind = TYPE_BASIC;
        type->info.basic = BASIC_FLOAT;
        type->assign = RIGHT;
        return type;
    } else if (strcmp(first->nodeName, "LP") == 0) { // (Exp)
        Type expType = get_type(getChild(node, 1));
        if (expType == NULL) {
            return NULL;
        }
        Type type = malloc(sizeof(struct Type_));
        *type = *expType;
        type->assign = RIGHT;
        return type;
    } else if (strcmp(first->nodeName, "MINUS") == 0 ||
               strcmp(first->nodeName, "NOT") == 0) { // -Exp || !Exp
        Type expType = get_type(getChild(node, 1));
        if (expType == NULL) {
            return NULL;
        }
        Type type = malloc(sizeof(struct Type_));
        *type = *expType;
        type->assign = RIGHT;
        return type;
    } else if (strcmp(first->nodeName, "Exp") == 0) {
        ASTNode second = getChild(node, 1);
        if (strcmp(second->nodeName, "ASSIGNOP") == 0) {
            Type exp1 = get_type(getChild(node, 0));
            Type exp2 = get_type(getChild(node, 2));
            if (exp1 == NULL || exp2 == NULL) {
                return NULL;
            }
            Type type = malloc(sizeof(struct Type_));
            *type = *exp1;
            type->assign = RIGHT;
            return type;
        } else if (strcmp(second->nodeName, "AND") == 0 ||
                   strcmp(second->nodeName, "OR") == 0 ||
                   strcmp(second->nodeName, "RELOP") == 0 ||
                   strcmp(second->nodeName, "PLUS") == 0 ||
                   strcmp(second->nodeName, "MINUS") == 0 ||
                   strcmp(second->nodeName, "STAR") == 0 ||
                   strcmp(second->nodeName, "DIV") == 0) { // Exp op Exp
            Type exp1 = get_type(getChild(node, 0));
            Type exp2 = get_type(getChild(node, 2));
            if (exp1 == NULL || exp2 == NULL) {
                return NULL;
            }
            Type type = malloc(sizeof(struct Type_));
            *type = *exp1;
            type->assign = RIGHT;
            return type;
        } else if (strcmp(second->nodeName, "LB") == 0) { // Exp[Exp]
            Type exp1 = get_type(getChild(node, 0));
            Type exp2 = get_type(getChild(node, 2));
            if (exp1 == NULL || exp2 == NULL) {
                return NULL;
            }
            return exp1->info.array.elem;
        } else if (strcmp(second->nodeName, "DOT") == 0) { // Exp.ID
            Type exp = get_type(getChild(node, 0));
            if (exp->kind == TYPE_ADDRESS) {
                exp = exp->info.address.type;
            }
            ASTNode id = getChild(node, 2);
            if (exp == NULL) {
                return NULL;
            }
            TypeList found = find_list(exp->info.structure.domain, id->valStr);
            return found->type;
        }
    }
    return NULL;
}
