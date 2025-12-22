typedef struct Type_* Type;
typedef struct List_* List;
typedef struct Structure_* Structure;
typedef struct Function_* Function;

struct Type_ {
    enum { BASIC, ARRAY, STRUCTURE, FUNCTION } kind;
    union {
        enum { INT, FLOAT } basic;
        struct {
            Type elem;
            int size;
        } array;
        struct {
            char *name;
            List domain;
        } structure;
        struct {
            char *name;
            int line;
            Type type;
            List param;
        } function;
    } info;
    enum { LEFT, RIGHT, BOTH } assign;
};

struct List_ {
    char *name;
    Type type;
    List next;
};

int type_check(Type t1, Type t2);
List new_list(char *name, Type type);
void insert_list(List *head, char *name, Type type);
List find_list(List head, char *name);
int check_list(List head, char *name, Type type);