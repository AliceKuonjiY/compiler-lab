#include <string.h>
#include <stdlib.h>
#include "type.h"

static int field_check(List p1, List p2);

int type_check(Type t1, Type t2) {
    if (t1 == NULL && t2 == NULL) {
        return 1;
    }
    if (t1 == NULL || t2 == NULL || t1->kind != t2-> kind) {
        return 0;
    }
    switch (t1->kind) {
        case TYPE_BASIC:
            return t1->info.basic == t2->info.basic;
        case TYPE_ARRAY:
            return type_check(t1->info.array.elem, t2->info.array.elem);
        case TYPE_STRUCTURE:
            return strcmp(t1->info.structure.name, t2->info.structure.name) == 0;
        case TYPE_FUNCTION: 
            if (strcmp(t1->info.function.name, t2->info.function.name) != 0) {
                return 0;
            }
            return field_check(t1->info.function.param, t2->info.function.param);
        default: return 0;
    }
}

static int field_check(List p1, List p2) {
    while (p1 != NULL && p2 != NULL) {
        if (type_check(p1->type, p2->type) == 0) {
            return 0;
        }
        p1 = p1->next;
        p2 = p2->next;
    }
    if (p1 == NULL && p2 == NULL) {
        return 1;
    } else {
        return 0;
    }
}

List new_list(char *name, Type type) {
    List list = (List)malloc(sizeof(struct List_));
    list->name = name;
    list->type = type;
    list->next = NULL;
    return list;
}

void insert_list(List *head, char *name, Type type) {
    List newnode = new_list(name, type);
    newnode->next = *head;
    *head = newnode;
}

List find_list(List head, char *name) {
    List now = head;
    while (now != NULL) {
        if (strcmp(now->name, name) == 0) {
            return now;
        }
        now = now->next;
    }
    return NULL;
}

int check_list(List head, char *name, Type type) {
    List now = head;
    while (now != NULL) {
        if (strcmp(now->name, name) == 0 && type_check(now->type, type)) {
            return 1;
        }
        now = now->next;
    }
    return 0;
}

int calc_size(Type type) {
    if (type == NULL) {
        return 0;
    }
    if (type->kind == TYPE_BASIC) {
        return 4;
    } else if (type->kind == TYPE_ARRAY) {
        return type->info.array.size * calc_size(type->info.array.elem);
    } else if (type->kind == TYPE_STRUCTURE) {
        int size = 0;
        List field = type->info.structure.domain;
        while (field != NULL) {
            size += calc_size(field->type);
            field = field->next;
        }
        return size;
    } else {
        return 0;
    }
}

int calc_offset(List head, char *name) {
    int offset = 0;
    List now = head;
    while (now != NULL) {
        if (strcmp(now->name, name) == 0) {
            return offset;
        }
        offset += calc_size(now->type);
        now = now->next;
    }
    return -1; // not found
}
