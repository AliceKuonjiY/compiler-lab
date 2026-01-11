#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "ir.h"
#include "asm.h"

extern ASTNode Root;
extern InterCode IR;
extern ListHead ASM;
extern int lexical_errorFlag;
extern int yyparse(void);
extern void yyrestart(FILE *input_file);
extern void semantic_analysis(ASTNode root);
extern void translate_ir(ASTNode root);
extern void optimize();
extern void translate_asm();

int main(int argc, char** argv) {
    char ir_path[50];
    if (argc <= 1) {
        perror("Please provide an input file.");
        return 1;
    }
    FILE *f = NULL;
    if (!(f = fopen(argv[1], "r"))) {
        perror(argv[1]);
        return 1;
    }
    // Syntax Analysis
    yyrestart(f);
    yyparse();
    if (!lexical_errorFlag) {
        print_ast(Root, 0);
        // Semantic Analysis
        semantic_analysis(Root);
        // Intermediate Code Generation
        translate_ir(Root);
        // Optimization and Target Code Generation
        optimize();
        print_ir(IR);
        translate_asm();
        print_asm(&ASM, argv[2]);
    }
    return 0;
}
