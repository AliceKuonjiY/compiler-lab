%locations

%{
    /* begin head */

    #include <assert.h>
    #include <stdarg.h>
    #include "lex.yy.c"

    int yyerror(const char *msg);

    Node Root = NULL;
    int errorFlag = 0;
    int errorLines[2000];

    /* end head */
%}

%union {
    struct Node_ *node;
}

%token <node> INT FLOAT ID
%token <node> RELOP
%token <node> TYPE
%token <node> SEMI COMMA
%token <node> PLUS MINUS STAR DIV AND OR DOT NOT ASSIGNOP
%token <node> LP RP LB RB LC RC
%token <node> STRUCT RETURN IF ELSE WHILE

%type <node> Exp Args
%type <node> DefList Def DecList Dec
%type <node> CompSt StmtList Stmt
%type <node> VarDec FunDec VarList ParamDec
%type <node> Specifier StructSpecifier OptTag Tag
%type <node> Program ExtDefList ExtDef ExtDecList

%nonassoc LOWER
%nonassoc ELSE
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT HIGHER_MINUS
%left DOT LP RP LB RB
%%
Program : ExtDefList {
        $$ = constructNode("Program", NTML, @$.first_line);
        construct($$, 1, $1);
        Root = $$;
    }
    ;
ExtDefList : ExtDef ExtDefList {
        $$ = constructNode("ExtDefList", NTML, @$.first_line);
        construct($$, 2, $1, $2);
    }
    | %empty {
        $$ = NULL;
    }
    ;
ExtDef : Specifier ExtDecList SEMI {
        $$ = constructNode("ExtDef", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Specifier SEMI {
        $$ = constructNode("ExtDef", NTML, @$.first_line);
        construct($$, 2, $1, $2);
    }
    | Specifier FunDec CompSt {
        $$ = constructNode("ExtDef", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Specifier error {
        $$ = constructNode("ExtDef", NTML, @$.first_line);
        errorFlag = 1;
    }
    ;
ExtDecList : VarDec {
        $$ = constructNode("ExtDecList", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    | VarDec COMMA ExtDecList {
        $$ = constructNode("ExtDecList", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    ;
Specifier : TYPE {
        $$ = constructNode("Specifier", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    | StructSpecifier {
        $$ = constructNode("Specifier", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {
        $$ = constructNode("StructSpecifier", NTML, @$.first_line);
        construct($$, 5, $1, $2, $3, $4, $5);
    }
    | STRUCT Tag {
        $$ = constructNode("StructSpecifier", NTML, @$.first_line);
        construct($$, 2, $1, $2);
    }
    ;
OptTag : ID {
        $$ = constructNode("OptTag", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    | %empty {
        $$ = NULL;
    }
    ;
Tag : ID {
        $$ = constructNode("Tag", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    ;
VarDec : ID {
        $$ = constructNode("VarDec", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    | VarDec LB INT RB {
        $$ = constructNode("VarDec", NTML, @$.first_line);
        construct($$, 4, $1, $2, $3, $4);
    }
    | VarDec LB error RB {
        $$ = constructNode("VarDec", NTML, @$.first_line);
        errorFlag = 1;
    }
    ;
FunDec : ID LP VarList RP {
        $$ = constructNode("FunDec", NTML, @$.first_line);
        construct($$, 4, $1, $2, $3, $4);
    }
    | ID LP RP {
        $$ = constructNode("FunDec", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | ID LP error RP {
        $$ = constructNode("FunDec", NTML, @$.first_line);
        errorFlag = 1;
    }
    ;
VarList : ParamDec COMMA VarList {
        $$ = constructNode("VarList", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | ParamDec {
        $$ = constructNode("VarList", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    ;
ParamDec : Specifier VarDec {
        $$ = constructNode("ParamDec", NTML, @$.first_line);
        construct($$, 2, $1, $2);
    }
    ;
CompSt : LC DefList StmtList RC {
        $$ = constructNode("CompSt", NTML, @$.first_line);
        construct($$, 4, $1, $2, $3, $4);
    }
    | LC error RC {
        $$ = constructNode("CompSt", NTML, @$.first_line);
        errorFlag = 1;
    }
    ;
StmtList : Stmt StmtList {
        $$ = constructNode("StmtList", NTML, @$.first_line);
        construct($$, 2, $1, $2);
    }
    | %empty {
        $$ = NULL;
    }
Stmt : Exp SEMI {
        $$ = constructNode("Stmt", NTML, @$.first_line);
        construct($$, 2, $1, $2);
    }
    | CompSt {
        $$ = constructNode("Stmt", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    | RETURN Exp SEMI {
        $$ = constructNode("Stmt", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | IF LP Exp RP Stmt %prec LOWER {
        $$ = constructNode("Stmt", NTML, @$.first_line);
        construct($$, 5, $1, $2, $3, $4, $5);
    }
    | IF LP Exp RP Stmt ELSE Stmt {
        $$ = constructNode("Stmt", NTML, @$.first_line);
        construct($$, 7, $1, $2, $3, $4, $5, $6, $7);
    }
    | WHILE LP Exp RP Stmt {
        $$ = constructNode("Stmt", NTML, @$.first_line);
        construct($$, 5, $1, $2, $3, $4, $5);
    }
    | error SEMI {
        $$ = constructNode("Stmt", NTML, @$.first_line);
        errorFlag = 1;
    }
    | IF LP Exp RP error ELSE Stmt {
        $$ = constructNode("Stmt", NTML, @$.first_line);
        errorFlag = 1;
    }
    ;
DefList : Def DefList {
        $$ = constructNode("DefList", NTML, @$.first_line);
        construct($$, 2, $1, $2);
    }
    | %empty {
        $$ = NULL;
    }
    ;
Def : Specifier DecList SEMI {
        $$ = constructNode("Def", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Specifier error SEMI {
        $$ = constructNode("Def", NTML, @$.first_line);
        errorFlag = 1;
    }
    ;
DecList : Dec {
        $$ = constructNode("DecList", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    | Dec COMMA DecList {
        $$ = constructNode("DecList", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    ;
Dec : VarDec {
        $$ = constructNode("Dec", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    | VarDec ASSIGNOP Exp {
        $$ = constructNode("Dec", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | VarDec ASSIGNOP error {
        $$ = constructNode("Dec", NTML, @$.first_line);
        errorFlag = 1;
    }
    ;
Exp : Exp ASSIGNOP Exp {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Exp AND Exp {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Exp OR Exp {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Exp RELOP Exp {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Exp PLUS Exp {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Exp MINUS Exp {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Exp STAR Exp {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Exp DIV Exp {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | LP Exp RP {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | MINUS Exp %prec HIGHER_MINUS {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 2, $1, $2);
    }
    | NOT Exp {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 2, $1, $2);
    }
    | ID LP Args RP {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 4, $1, $2, $3, $4);
    }
    | ID LP RP {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Exp LB Exp RB {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 4, $1, $2, $3, $4);
    }
    | Exp DOT ID {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | ID {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    | INT {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    | FLOAT {
        $$ = constructNode("Exp", NTML, @$.first_line);
        construct($$, 1, $1);
    }
    | Exp LB error RB {
        $$ = constructNode("Exp", NTML, @$.first_line);
        errorFlag = 1;
    }
    | Exp ASSIGNOP error {
        $$ = constructNode("Exp", NTML, @$.first_line);
        errorFlag = 1;
    }
    | Exp AND error {
        $$ = constructNode("Exp", NTML, @$.first_line);
        errorFlag = 1;
    }
    | Exp OR error {
        $$ = constructNode("Exp", NTML, @$.first_line);
        errorFlag = 1;
    }
    | Exp RELOP error {
        $$ = constructNode("Exp", NTML, @$.first_line);
        errorFlag = 1;
    }
    | Exp PLUS error {
        $$ = constructNode("Exp", NTML, @$.first_line);
        errorFlag = 1;
    }
    | Exp MINUS error {
        $$ = constructNode("Exp", NTML, @$.first_line);
        errorFlag = 1;
    }
    | Exp STAR error {
        $$ = constructNode("Exp", NTML, @$.first_line);
        errorFlag = 1;
    }
    | Exp DIV error {
        $$ = constructNode("Exp", NTML, @$.first_line);
        errorFlag = 1;
    }
    ;
Args : Exp COMMA Args {
        $$ = constructNode("Args", NTML, @$.first_line);
        construct($$, 3, $1, $2, $3);
    }
    | Exp {
        $$ = constructNode("Args", NTML, @$.first_line);
        construct($$, 1, $1);
    }
%%
int yyerror(const char *msg) {
    if (errorLines[yylineno] != 1) {
        errorFlag = 1;
        errorLines[yylineno] = 1;
        printf("Error type B at Line %d: %s.\n", yylineno, msg);
        return 1;
    } else {
        return 0;
    }
}
