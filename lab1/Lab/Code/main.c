#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern NodeP Root;
extern int errorFlag;
extern char hasError[2000];
extern int yyparse(void);
extern void yyrestart(FILE *input_file);
extern void printTree(NodeP root, int space);

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
