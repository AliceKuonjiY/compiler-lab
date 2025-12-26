%{
    /* begin head */

    #include <stdio.h>
    #include <string.h>
    #include "syntax.tab.h"
    #include "ast.h"

    int yycolumnno = 1;

    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumnno; \
        yylloc.last_column = yycolumnno + yyleng - 1; \
        yycolumnno += yyleng;

    extern int lexical_errorFlag;
    extern int errorLines[5000];

    /* end head */
%}

OCT         0[0-7]+
DEC         0|([1-9][0-9]*)
HEX         0[xX][0-9a-fA-F]+
FLOAT       ([0-9]*\.[0-9]+)|([0-9]+\.)
ID          [_a-zA-Z][_a-zA-Z0-9]*

%option yylineno

%%
[ \t\r]         {}
\n              { yycolumnno = 1; }
";"             { yylval.node = constructNode("SEMI", NVL, yylineno); return SEMI; }
","             { yylval.node = constructNode("COMMA", NVL, yylineno); return COMMA; }
"="             { yylval.node = constructNode("ASSIGNOP", NVL, yylineno); return ASSIGNOP; }
"<"             { yylval.node = constructNode("RELOP", NVL, yylineno); yylval.node->valInt = 0; return RELOP; }
">"             { yylval.node = constructNode("RELOP", NVL, yylineno); yylval.node->valInt = 1; return RELOP; }
"<="            { yylval.node = constructNode("RELOP", NVL, yylineno); yylval.node->valInt = 2; return RELOP; }
">="            { yylval.node = constructNode("RELOP", NVL, yylineno); yylval.node->valInt = 3; return RELOP; }
"=="            { yylval.node = constructNode("RELOP", NVL, yylineno); yylval.node->valInt = 4; return RELOP; }
"!="            { yylval.node = constructNode("RELOP", NVL, yylineno); yylval.node->valInt = 5; return RELOP; }
"+"             { yylval.node = constructNode("PLUS", NVL, yylineno); yylval.node->valInt = 0; return PLUS; }
"-"             { yylval.node = constructNode("MINUS", NVL, yylineno); yylval.node->valInt = 1; return MINUS; }
"*"             { yylval.node = constructNode("STAR", NVL, yylineno); yylval.node->valInt = 2; return STAR; }
"/"             { yylval.node = constructNode("DIV", NVL, yylineno); yylval.node->valInt = 3; return DIV; }
"&&"            { yylval.node = constructNode("AND", NVL, yylineno); return AND; }
"||"            { yylval.node = constructNode("OR", NVL, yylineno); return OR; }
"."             { yylval.node = constructNode("DOT", NVL, yylineno); return DOT; }
"!"             { yylval.node = constructNode("NOT", NVL, yylineno); return NOT; }
"("             { yylval.node = constructNode("LP", NVL, yylineno); return LP; }
")"             { yylval.node = constructNode("RP", NVL, yylineno); return RP; }
"["             { yylval.node = constructNode("LB", NVL, yylineno); return LB; }
"]"             { yylval.node = constructNode("RB", NVL, yylineno); return RB; }
"{"             { yylval.node = constructNode("LC", NVL, yylineno); return LC; }
"}"             { yylval.node = constructNode("RC", NVL, yylineno); return RC; }
"struct"        { yylval.node = constructNode("STRUCT", NVL, yylineno); return STRUCT; }
"return"        { yylval.node = constructNode("RETURN", NVL, yylineno); return RETURN; }
"if"            { yylval.node = constructNode("IF", NVL, yylineno); return IF; }
"else"          { yylval.node = constructNode("ELSE", NVL, yylineno); return ELSE; }
"while"         { yylval.node = constructNode("WHILE", NVL, yylineno); return WHILE; }
"int"           { yylval.node = constructNode("TYPE", VL, yylineno); yylval.node->valInt = 0; return TYPE; }
"float"         { yylval.node = constructNode("TYPE", VL, yylineno); yylval.node->valInt = 1; return TYPE; }
{ID}            {
    yylval.node = constructNode("ID", VL, yylineno);
    yylval.node->valStr = strdup(yytext);
    return ID;
}
{DEC}|{OCT}|{HEX}   {
    char *endptr;
    yylval.node = constructNode("INT", VL, yylineno);
    yylval.node->valInt = strtoul(yytext, &endptr, 0);
    return INT;
}
{FLOAT}         {
    char *endptr;
    yylval.node = constructNode("FLOAT", VL, yylineno);
    yylval.node->valFloat = strtof(yytext, &endptr);
    return FLOAT;
}
.               {
    if (errorLines[yylineno] == 0) {
        errorLines[yylineno] = 1;
        lexical_errorFlag = 1;
        printf("Error type A at Line %d: Error token.\n", yylineno);
    }
}
%%
