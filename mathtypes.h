#ifndef MATHTYPES
#define MATHTYPES

typedef int var_ind;
typedef struct var_t {
    char* name;
    double val;
};

enum NODE_TYPE {
    NODE_OPR,
    NODE_CST,
    NODE_VAR
};

enum OP_TYPE {
    OP_SIN,
    OP_COS,
    OP_ADD,
    OP_DIV,
    OP_MUL,
    OP_SUB,
    OP_POW
};










#endif