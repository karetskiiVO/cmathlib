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
    NODE_VAR,
    NODE_EMPTY
};

enum OP_TYPE {
    OP_SIN,
    OP_COS,
    OP_ADD = 10,
    OP_DIV = 20,
    OP_MUL = 21,
    OP_SUB = 11,
    OP_POW = 30,
    OP_EMPTY
};










#endif