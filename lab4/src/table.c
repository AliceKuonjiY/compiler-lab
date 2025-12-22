#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"

TypeStack new_stack() {
    TypeStack stack = (TypeStack)malloc(sizeof(struct TypeStack_));
    stack->list = NULL;
    stack->next = NULL;
    return stack;
}

void push_stack(TypeStack *stack) {
    TypeStack newtop = new_stack();
    newtop->next = *stack;
    *stack = newtop;
}

void pop_stack(TypeStack *stack) {
    if (*stack != NULL) {
        TypeStack top = *stack;
        *stack = top->next;
        TypeList now = top->list;
        while (now != NULL) {
            TypeList temp = now;
            now = now->next;
            free(temp);
        }
        free(top);
    }
}

TypeList find_in_stack(TypeStack stack, char *name) {
    TypeStack now_stack = stack;
    while (now_stack != NULL) {
        TypeList now_list = now_stack->list;
        TypeList found = find_list(now_list, name);
        if (found != NULL) {
            return found;
        }
        now_stack = now_stack->next;
    }
    return NULL;
}

TypeList find_in_top(TypeStack stack, char *name) {
    if (stack == NULL) {
        return NULL;
    }
    TypeList now_list = stack->list;
    return find_list(now_list, name);
}

int check_in_top(TypeStack stack, char *name, Type type) {
    if (stack == NULL) {
        return 0;
    }
    TypeList now_list = stack->list;
    return check_list(now_list, name, type);
}

int check_in_stack(TypeStack stack, char *name, Type type) {
    TypeStack now_stack = stack;
    while (now_stack != NULL) {
        TypeList now_list = now_stack->list;
        if (check_list(now_list, name, type)) {
            return 1;
        }
        now_stack = now_stack->next;
    }
    return 0;
}

void insert_in_top(TypeStack stack, char *name, Type type) {
    if (stack != NULL) {
        insert_list(&stack->list, name, type);
    }
}

void insert_in_bottom(TypeStack stack, char *name, Type type) {
    if (stack == NULL) {
        return;
    }
    TypeStack now_stack = stack;
    while (now_stack->next != NULL) {
        now_stack = now_stack->next;
    }
    insert_list(&now_stack->list, name, type);
}

void free_stack(TypeStack *stack) {
    while (*stack != NULL) {
        TypeStack temp_stack = *stack;
        *stack = (*stack)->next;
        TypeList now = temp_stack->list;
        while (now != NULL) {
            TypeList temp_list = now;
            now = now->next;
            free(temp_list);
        }
        free(temp_stack);
    }
    stack = NULL;
}