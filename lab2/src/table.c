#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"

Stack new_stack() {
    Stack stack = (Stack)malloc(sizeof(struct Stack_));
    stack->list = NULL;
    stack->next = NULL;
    return stack;
}

void push_stack(Stack *stack) {
    Stack newtop = new_stack();
    newtop->next = *stack;
    *stack = newtop;
}

void pop_stack(Stack *stack) {
    if (*stack != NULL) {
        Stack top = *stack;
        *stack = top->next;
        List now = top->list;
        while (now != NULL) {
            List temp = now;
            now = now->next;
            free(temp);
        }
        free(top);
    }
}

List find_in_stack(Stack stack, char *name) {
    Stack now_stack = stack;
    while (now_stack != NULL) {
        List now_list = now_stack->list;
        List found = find_list(now_list, name);
        if (found != NULL) {
            return found;
        }
        now_stack = now_stack->next;
    }
    return NULL;
}

List find_in_top(Stack stack, char *name) {
    if (stack == NULL) {
        return NULL;
    }
    List now_list = stack->list;
    return find_list(now_list, name);
}

int check_in_top(Stack stack, char *name, Type type) {
    if (stack == NULL) {
        return 0;
    }
    List now_list = stack->list;
    return check_list(now_list, name, type);
}

int check_in_stack(Stack stack, char *name, Type type) {
    Stack now_stack = stack;
    while (now_stack != NULL) {
        List now_list = now_stack->list;
        if (check_list(now_list, name, type)) {
            return 1;
        }
        now_stack = now_stack->next;
    }
    return 0;
}

void insert_in_top(Stack stack, char *name, Type type) {
    if (stack != NULL) {
        insert_list(&stack->list, name, type);
    }
}

void insert_in_bottom(Stack stack, char *name, Type type) {
    if (stack == NULL) {
        return;
    }
    Stack now_stack = stack;
    while (now_stack->next != NULL) {
        now_stack = now_stack->next;
    }
    insert_list(&now_stack->list, name, type);
}

void free_stack(Stack *stack) {
    while (*stack != NULL) {
        Stack temp_stack = *stack;
        *stack = (*stack)->next;
        List now = temp_stack->list;
        while (now != NULL) {
            List temp_list = now;
            now = now->next;
            free(temp_list);
        }
        free(temp_stack);
    }
    stack = NULL;
}