#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ast.h"
#include "table.h"

Stack check_symbolTable;
int errorLine[5000] = {0};

static void printError(int type, int line, const char *msg);
static void ExtDefList(Node node);
static void ExtDef(Node node);
static void ExtDecList(Node node, Type type, List *head);
static Type FunDec(Node node, Type returnType);
static void CompSt(Node node, Type returnType);
static void StmtList(Node node, Type returnType);
static void Stmt(Node node, Type returnType);
static void VarList(Node node, List *head);
static void ParamDec(Node node, List *head);
static Type Specifier(Node node);
static Type StructSpecifier(Node node);
static void DefList(Node node, List *head, int structFlag);
static void Def(Node node, List *head, int structFlag);
static void DecList(Node node, Type type, List *head, int structFlag);
static void Dec(Node node, Type type, List *head, int structFlag);
static void VarDec(Node node, Type type, List *head, int structFlag);
static Type Exp(Node node);
static void Args(Node node, List *head);

static void printError(int type, int line, const char *msg) {
    if (errorLine[line]) {
        return;
    }
    errorLine[line] = 1;
    fprintf(stdout, "Error type %d at Line %d: %s\n", type, line, msg);
}

void semantic_analysis(Node root) {
    srand((unsigned)time(NULL));
    check_symbolTable = new_stack();
    Type read_ = malloc(sizeof(struct Type_));
    read_->kind = TYPE_FUNCTION;
    read_->info.function.name = "read";
    read_->info.function.type = malloc(sizeof(struct Type_));
    *(read_->info.function.type) = (struct Type_){ .kind = TYPE_BASIC, .info.basic = BASIC_INT, .assign = BOTH };
    read_->info.function.param = NULL;
    read_->assign = RIGHT;
    Type write_ = malloc(sizeof(struct Type_));
    write_->kind = TYPE_FUNCTION;
    write_->info.function.name = "write";
    write_->info.function.type = malloc(sizeof(struct Type_));
    *(write_->info.function.type) = (struct Type_){ .kind = TYPE_BASIC, .info.basic = BASIC_INT, .assign = BOTH };
    write_->info.function.param = malloc(sizeof(struct List_));
    write_->info.function.param->type = malloc(sizeof(struct Type_));
    *(write_->info.function.param->type) = (struct Type_){ .kind = TYPE_BASIC, .info.basic = BASIC_INT, .assign = BOTH };
    write_->info.function.param->name = "x";
    write_->info.function.param->next = NULL;
    write_->assign = RIGHT;
    insert_in_bottom(check_symbolTable, "read", read_);
    insert_in_bottom(check_symbolTable, "write", write_);
    ExtDefList(getChild(root, 0));
    free_stack(&check_symbolTable);
}

static void ExtDefList(Node node) {
    if (node == NULL) {
        return;
    }
    ExtDef(getChild(node, 0));
    ExtDefList(getChild(node, 1));
}

static void ExtDef(Node node) {
    Type type = Specifier(getChild(node, 0));
    Node second = getChild(node, 1);
    if (strcmp(second->nodeName, "ExtDecList") == 0) { // 变量声明
        List head = NULL;
        ExtDecList(second, type, &head);
    } else if (strcmp(second->nodeName, "FunDec") == 0) { // 函数定义
        push_stack(&check_symbolTable);
        Type func = FunDec(second, type);
        pop_stack(&check_symbolTable);
        if (find_in_stack(check_symbolTable, func->info.function.name) != NULL) {
            // !ERROR
            printError(4, node->lineNum, "Redefined function.");
        }
        insert_in_top(check_symbolTable, func->info.function.name, func);
        push_stack(&check_symbolTable);
        FunDec(second, type);
        CompSt(getChild(node, 2), type);
        pop_stack(&check_symbolTable);
    } else if (strcmp(second->nodeName, "SEMI") == 0) { // 仅类型声明
        return;
    }
}

static Type FunDec(Node node, Type returnType) {
    Type type = malloc(sizeof(struct Type_));
    type->kind = TYPE_FUNCTION;
    type->info.function.name = getChild(node, 0)->valStr;
    type->info.function.line = node->lineNum;
    type->info.function.type = returnType;
    type->info.function.param = NULL;
    type->assign = RIGHT;
    if (strcmp(getChild(node, 2)->nodeName, "VarList") == 0) { // 有参数
        VarList(getChild(node, 2), &type->info.function.param);
    }
    return type;
}

static void CompSt(Node node, Type returnType) {
    List head = NULL;
    Node defList = NULL, stmtList = NULL;
    if (strcmp(getChild(node, 1)->nodeName, "DefList") == 0) { // ?HACK
        defList = getChild(node, 1);
        if (strcmp(getChild(node, 2)->nodeName, "StmtList") == 0) {
            stmtList = getChild(node, 2);
        }
    } else if (strcmp(getChild(node, 1)->nodeName, "StmtList") == 0) { // ?HACK
        stmtList = getChild(node, 1);
    }
    DefList(defList, &head, 0);
    StmtList(stmtList, returnType);
}

static void StmtList(Node node, Type returnType) {
    if (node == NULL) {
        return;
    }
    Stmt(getChild(node, 0), returnType);
    StmtList(getChild(node, 1), returnType);
}

static void Stmt(Node node, Type returnType) {
    Node first = getChild(node, 0);
    if (strcmp(first->nodeName, "Exp") == 0) { // Exp
        Exp(first);
    } else if (strcmp(first->nodeName, "CompSt") == 0) { // CompSt
        push_stack(&check_symbolTable);
        CompSt(first, returnType);
        pop_stack(&check_symbolTable);
    } else if (strcmp(first->nodeName, "RETURN") == 0) { // return
        Type expType = Exp(getChild(node, 1));
        if (expType == NULL) {
            return;
        }
        if (!type_check(returnType, expType)) {
            // !ERROR
            printError(8, node->lineNum, "Type mismatched for return.");
        }
    } else if (strcmp(first->nodeName, "IF") == 0) { // if
        Exp(getChild(node, 2));
        Stmt(getChild(node, 4), returnType);
        if (getChild(node, 5) != NULL) {
            Stmt(getChild(node, 6), returnType);
        } 
    } else if (strcmp(first->nodeName, "WHILE") == 0) { // while
        Exp(getChild(node, 2));
        Stmt(getChild(node, 4), returnType);
    }
}

static void VarList(Node node, List *head) {
    ParamDec(getChild(node, 0), head);
    if (getChild(node, 1) != NULL) {
        VarList(getChild(node, 2), head);
    }
}

static void ParamDec(Node node, List *head) {
    Type type = Specifier(getChild(node, 0));
    VarDec(getChild(node, 1), type, head, 0);
}

static void ExtDecList(Node node, Type type, List *head) {
    VarDec(getChild(node, 0), type, head, 0);
    if (getChild(node, 1) != NULL) {
        ExtDecList(getChild(node, 2), type, head);
    }
}

static Type Specifier(Node node) {
    Node child = getChild(node, 0);
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

static Type StructSpecifier(Node node) {
    Type type = malloc(sizeof(struct Type_));
    type->kind = TYPE_STRUCTURE;
    type->assign = BOTH;
    Node tag = getChild(node, 1);
    if (strcmp(tag->nodeName, "OptTag") == 0) { // 命名结构体定义
        Node id = getChild(tag, 0);
        Node defList = getChild(node, 3);
        type->info.structure.name = id->valStr;
        push_stack(&check_symbolTable);
        if (strcmp(defList->nodeName, "DefList") == 0) { // ?HACK
            DefList(defList, &type->info.structure.domain, 1);
        }
        pop_stack(&check_symbolTable);
        if (find_in_stack(check_symbolTable, type->info.structure.name) != NULL) {
            // !ERROR
            printError(16, node->lineNum, "Redefined structure.");
            return NULL;
        }
        insert_in_bottom(check_symbolTable, type->info.structure.name, type); // ?HACK
        return type;
    } else if (strcmp(tag->nodeName, "LC") == 0) { // 匿名结构体定义
        Node defList = getChild(node, 2);
        type->info.structure.name = malloc(20 * sizeof(char));
        sprintf(type->info.structure.name, "anon%d", rand());
        push_stack(&check_symbolTable);
        if (strcmp(defList->nodeName, "DefList") == 0) { // ?HACK
            DefList(defList, &type->info.structure.domain, 1);
        }
        pop_stack(&check_symbolTable);
        if (find_in_stack(check_symbolTable, type->info.structure.name) != NULL) {
            // !ERROR
            printError(16, node->lineNum, "Redefined structure.");
            return NULL;
        }
        insert_in_bottom(check_symbolTable, type->info.structure.name, type); // ?HACK
        return type;
    } else if (strcmp(tag->nodeName, "Tag") == 0) { // 结构体引用
        Node id = getChild(tag, 0);
        List found = find_in_stack(check_symbolTable, id->valStr);
        if (found == NULL) {
            // !ERROR
            printError(17, node->lineNum, "Undefined structure.");
            return NULL;
        }
        return found->type;
    } else {
        return NULL;
    }
}

static void DefList(Node node, List *head, int structFlag) {
    if (node == NULL) {
        return;
    }
    Def(getChild(node, 0), head, structFlag);
    if (getChild(node, 1) != NULL) {
        DefList(getChild(node, 1), head, structFlag);
    }
}

static void Def(Node node, List *head, int structFlag) {
    Type type = Specifier(getChild(node, 0));
    DecList(getChild(node, 1), type, head, structFlag);
}

static void DecList(Node node, Type type, List *head, int structFlag) {
    Dec(getChild(node, 0), type, head, structFlag);
    if (getChild(node, 1) != NULL) {
        DecList(getChild(node, 2), type, head, structFlag);
    }
}

static void Dec(Node node, Type type, List *head, int structFlag) {
    VarDec(getChild(node, 0), type, head, structFlag);
    if (getChild(node, 1) != NULL) { // 变量初始化
        if (structFlag) {
            // !ERROR
            printError(15, node->lineNum, "Cannot initialize field.");
        }
        Type exp = Exp(getChild(node, 2));
        if (exp == NULL) {
            return;
        }
        if (!type_check(type, exp)) {
            // !ERROR
            printError(5, node->lineNum, "Type mismatched for assignment.");
        }
    }
}

static void VarDec(Node node, Type type, List *head, int structFlag) {
    Node first = getChild(node, 0);
    if (strcmp(first->nodeName, "ID") == 0) { // 变量声明
        List found = find_in_top(check_symbolTable, first->valStr);
        if (found != NULL) {
            // !ERROR
            if (structFlag) {
                printError(15, node->lineNum, "Redefined field.");
            } else {
                printError(3, node->lineNum, "Redefined variable.");
            }
            return;
        }
        insert_in_top(check_symbolTable, first->valStr, type);
        insert_list(head, first->valStr, type);
    } else if (strcmp(first->nodeName, "VarDec") == 0) { // 数组声明
        int size = getChild(node, 2)->valInt;
        Type arrayType = malloc(sizeof(struct Type_));
        arrayType->kind = TYPE_ARRAY;
        arrayType->info.array.size = size;
        arrayType->info.array.elem = type;
        arrayType->assign = LEFT;
        VarDec(first, arrayType, head, structFlag);
    }
}

static Type Exp(Node node) {
    Node first = getChild(node, 0);
    if (strcmp(first->nodeName, "ID") == 0) { // 变量或函数调用
        List found = find_in_stack(check_symbolTable, first->valStr);
        if (getChild(node, 1) == NULL) { // 变量
            if (found == NULL ||
                found->type == NULL ||
                found->type->kind == TYPE_FUNCTION ||
                (found->type->kind == TYPE_STRUCTURE &&
                strcmp(found->name, found->type->info.structure.name) == 0)) {
                // !ERROR
                printError(1, node->lineNum, "Undefined variable.");
                return NULL;
            }
            return found->type;
        } else { // 函数
            if (found == NULL) {
                // !ERROR
                printError(2, node->lineNum, "Undefined function.");
                return NULL;
            }
            if (found->type->kind != TYPE_FUNCTION) {
                // !ERROR
                printError(11, node->lineNum, "Not a function.");
                return NULL;
            }
            if (strcmp(getChild(node, 2)->nodeName, "Args") == 0) {
                List args = NULL;
                Args(getChild(node, 2), &args);
                Type use = malloc(sizeof(struct Type_));
                *use = *found->type;
                use->assign = RIGHT;
                use->info.function.param = args;
                if (!check_in_stack(check_symbolTable, use->info.function.name, use)) {
                    // !ERROR
                    printError(9, node->lineNum, "Type mismatched for function arguments.");
                    return NULL;
                }
            } else { // 无参数
                if (found->type->info.function.param != NULL) {
                    // !ERROR
                    printError(9, node->lineNum, "Type mismatched for function arguments.");
                    return NULL;
                }
            }
            Type type = malloc(sizeof(struct Type_));
            *type = *found->type->info.function.type;
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
        Type expType = Exp(getChild(node, 1));
        if (expType == NULL) {
            return NULL;
        }
        Type type = malloc(sizeof(struct Type_));
        *type = *expType;
        type->assign = RIGHT;
        return type;
    } else if (strcmp(first->nodeName, "MINUS") == 0 ||
               strcmp(first->nodeName, "NOT") == 0) { // -Exp || !Exp
        Type expType = Exp(getChild(node, 1));
        if (expType == NULL) {
            return NULL;
        }
        if (expType->kind != TYPE_BASIC) {
            // !ERROR
            printError(7, node->lineNum, "The operand of unary operator must be an integer or a float.");
            return NULL;
        }
        Type type = malloc(sizeof(struct Type_));
        *type = *expType;
        type->assign = RIGHT;
        return type;
    } else if (strcmp(first->nodeName, "Exp") == 0) {
        Node second = getChild(node, 1);
        if (strcmp(second->nodeName, "ASSIGNOP") == 0) {
            Type exp1 = Exp(getChild(node, 0));
            Type exp2 = Exp(getChild(node, 2));
            if (exp1 == NULL || exp2 == NULL) {
                return NULL;
            }
            if (!type_check(exp1, exp2)) {
                // !ERROR
                printError(5, node->lineNum, "Type mismatched for assignment.");
                return NULL;
            }
            if (exp1->assign == RIGHT) {
                // !ERROR
                printError(6, node->lineNum, "The left-hand side of an assignment must be a variable.");
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
            Type exp1 = Exp(getChild(node, 0));
            Type exp2 = Exp(getChild(node, 2));
            if (exp1 == NULL || exp2 == NULL) {
                return NULL;
            }
            if (!type_check(exp1, exp2)) {
                // !ERROR
                printError(7, node->lineNum, "Type mismatched for operands.");
                return NULL;
            }
            Type type = malloc(sizeof(struct Type_));
            *type = *exp1;
            type->assign = RIGHT;
            return type;
        } else if (strcmp(second->nodeName, "LB") == 0) { // Exp[Exp]
            Type exp1 = Exp(getChild(node, 0));
            Type exp2 = Exp(getChild(node, 2));
            if (exp1 == NULL || exp2 == NULL) {
                return NULL;
            }
            if (exp1->kind != TYPE_ARRAY) {
                // !ERROR
                printError(10, node->lineNum, "The variable is not an array.");
                return NULL;
            }
            if (exp2->kind != TYPE_BASIC || exp2->info.basic != BASIC_INT) {
                // !ERROR
                printError(12, node->lineNum, "The index of array must be an integer.");
                return NULL;
            }
            return exp1->info.array.elem;
        } else if (strcmp(second->nodeName, "DOT") == 0) { // Exp.ID
            Type exp = Exp(getChild(node, 0));
            Node id = getChild(node, 2);
            if (exp == NULL) {
                return NULL;
            }
            if (exp->kind != TYPE_STRUCTURE) {
                // !ERROR
                printError(13, node->lineNum, "The variable is not a structure.");
                return NULL;
            }
            List found = find_list(exp->info.structure.domain, id->valStr);
            if (found == NULL) {
                // !ERROR
                printError(14, node->lineNum, "Non-existent field.");
                return NULL;
            }
            return found->type;
        }
    }
    return NULL;
}

static void Args(Node node, List *head) {
    Type expType = Exp(getChild(node, 0));
    insert_list(head, NULL, expType);
    if (getChild(node, 1) != NULL) {
        Args(getChild(node, 2), head);
    }
}
