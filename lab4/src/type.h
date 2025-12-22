typedef struct Type_* Type;
typedef struct TypeList_* TypeList;
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
            TypeList domain;
        } structure;
        struct {
            char *name;
            int line;
            Type type;
            TypeList param;
        } function;
        struct {
            Type type;
        } address;
    } info;
    enum Assign assign;
};

struct TypeList_ {
    char *name;
    Type type;
    TypeList next;
};

int type_check(Type t1, Type t2);
TypeList new_list(char *name, Type type);
void insert_list(TypeList *head, char *name, Type type);
TypeList find_list(TypeList head, char *name);
int check_list(TypeList head, char *name, Type type);
int calc_size(Type type);
int calc_offset(TypeList head, char *name);