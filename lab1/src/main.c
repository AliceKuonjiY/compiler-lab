#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern Node Root;
extern int errorFlag;
extern int yyparse(void);
extern void yyrestart(FILE *input_file);

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
    if (!errorFlag) {
        printTree(Root, 0);
    }
    return 0;
}
