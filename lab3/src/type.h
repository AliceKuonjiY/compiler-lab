typedef struct Type_* Type;
typedef struct List_* List;
typedef struct Structure_* Structure;
typedef struct Function_* Function;

enum Assign { LEFT, RIGHT, BOTH };
enum TypeKind { TYPE_BASIC, TYPE_ARRAY, TYPE_STRUCTURE, TYPE_FUNCTION, TYPE_ADDRESS };
enum BasicType { BASIC_INT, BASIC_FLOAT };

struct Type_ {
    enum TypeKind kind;
    union {
        enum BasicType basic;
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
        struct {
            Type type;
        } address;
    } info;
    enum Assign assign;
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
int calc_size(Type type);
int calc_offset(List head, char *name);