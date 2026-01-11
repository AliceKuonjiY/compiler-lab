#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "ast.h"

NodeP constructNode(char *name, enum NodeType type, int line) {
    NodeP newNode = (NodeP)malloc(sizeof(struct Node));
    newNode->nodeName = name;
    newNode->nodeType = type;
    newNode->lineNum = line;
    newNode->firstChild = NULL;
    newNode->next = NULL;
    return newNode;
}

void construct(NodeP parent, int child_count, ...) {
    va_list args;
    va_start(args, child_count);
    NodeP now = NULL;
    for (int i = 0; i < child_count; i++) {
        NodeP child = va_arg(args, NodeP);
        if (child == NULL) {
            continue;
        }
        if (parent->firstChild == NULL) {
            parent->firstChild = child;
            now = child;
        } else {
            assert(now != NULL);
            now->next = child;
            now = now->next;
        }
    }
    va_end(args);
}

void printTree(NodeP root, int space) {
    if (root == NULL) {
        return;
    }
    for (int i = 0; i < space; i++) {
        printf(" ");
    }
    switch (root->nodeType) {
        case NTML:
            printf("%s (%d)\n", root->nodeName, root->lineNum);
            break;
        case NVL:
            printf("%s\n", root->nodeName);
            break;
        case VL:
            printf("%s: ", root->nodeName);
            if (strcmp(root->nodeName, "TYPE") == 0) {
                if (root->valType == TYPE_INT) {
                    printf("int\n");
                } else if (root->valType == TYPE_FLOAT) {
                    printf("float\n");
                } else {
                    fprintf(stderr, "ERROR!\n");
                }
            } else if (strcmp(root->nodeName, "ID") == 0) {
                if (root->valStr != NULL) {
                    printf("%s\n", root->valStr);
                } else {
                    fprintf(stderr, "ERROR!\n");
                }
            } else if (strcmp(root->nodeName, "INT") == 0) {
                printf("%u\n", root->valInt);
            } else if (strcmp(root->nodeName, "FLOAT") == 0) {
                printf("%f\n", root->valFloat);
            } else {
                fprintf(stderr, "ERROR!\n");
            }
            break;
        default:
            fprintf(stderr, "ERROR!\n");
    }
    space += 2;
    NodeP firstChild = root->firstChild;
    if (firstChild != NULL) {
        printTree(firstChild, space);
        NodeP now = firstChild->next;
        while (now != NULL) {
            printTree(now, space);
            now = now->next;
        }
    }
}
