#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern Node Root;
extern int lexical_errorFlag;
extern int yyparse(void);
extern void yyrestart(FILE *input_file);
extern void semantic_analysis(Node root);
extern void translate(Node root, const char *output_path);

int main(int argc, char** argv) {
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
        // printTree(Root, 0);
        // Semantic Analysis
        semantic_analysis(Root);
        // Intermediate Code Generation
        translate(Root, argv[2]);
    }
    return 0;
}
