#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "ast.h"

Node constructNode(char *name, enum NodeType type, int line) {
    Node newNode = (Node)malloc(sizeof(struct Node_));
    newNode->nodeName = name;
    newNode->nodeType = type;
    newNode->lineNum = line;
    newNode->firstChild = NULL;
    newNode->next = NULL;
    return newNode;
}

void construct(Node parent, int child_count, ...) {
    va_list args;
    va_start(args, child_count);
    Node now = NULL;
    for (int i = 0; i < child_count; i++) {
        Node child = va_arg(args, Node);
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

void printTree(Node root, int space) {
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
                if (root->valInt == TOKEN_INT) {
                    printf("int\n");
                } else if (root->valInt == TOKEN_FLOAT) {
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
    Node firstChild = root->firstChild;
    if (firstChild != NULL) {
        printTree(firstChild, space);
        Node now = firstChild->next;
        while (now != NULL) {
            printTree(now, space);
            now = now->next;
        }
    }
}

Node getChild(Node parent, int index) {
    if (parent == NULL || index < 0) {
        return NULL;
    }
    Node now = parent->firstChild;
    for (int i = 0; i < index; i++) {
        if (now == NULL) {
            return NULL;
        }
        now = now->next;
    }
    return now;
}
