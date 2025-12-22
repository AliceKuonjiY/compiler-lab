#include "type.h"

typedef struct Stack_* Stack;

struct Stack_ {
    List list;
    Stack next;
};

Stack new_stack();
void push_stack(Stack *stack);
void pop_stack(Stack *stack);
List find_in_stack(Stack stack, char *name);
List find_in_top(Stack stack, char *name);
int check_in_stack(Stack stack, char *name, Type type);
int check_in_top(Stack stack, char *name, Type type);
void insert_in_top(Stack stack, char *name, Type type);
void insert_in_bottom(Stack stack, char *name, Type type);
void free_stack(Stack *stack);