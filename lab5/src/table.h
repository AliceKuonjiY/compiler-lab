#include "type.h"

typedef struct TypeStack_* TypeStack;

struct TypeStack_ {
    TypeList list;
    TypeStack next;
};

TypeStack new_stack();
void push_stack(TypeStack *stack);
void pop_stack(TypeStack *stack);
TypeList find_in_stack(TypeStack stack, char *name);
TypeList find_in_top(TypeStack stack, char *name);
int check_in_stack(TypeStack stack, char *name, Type type);
int check_in_top(TypeStack stack, char *name, Type type);
void insert_in_top(TypeStack stack, char *name, Type type);
void insert_in_bottom(TypeStack stack, char *name, Type type);
void free_stack(TypeStack *stack);