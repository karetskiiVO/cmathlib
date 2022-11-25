#ifndef MATHLIB
#define MATHLIB

#include "clist_str.h"
#include "mathtypes.h"

#define getFunction(__Equation) getFunc(#__Equation)

typedef struct Func_node {
    NODE_TYPE type;

    double  value;
    OP_TYPE nodeop;
    size_t  varind;

    Func_node* left;
    Func_node* right;
} Func_node;

typedef struct Function {
    List* variables;
    Func_node* equation;
} Function;

const Func_node F_EMPTY = {NODE_EMPTY, NAN, OP_EMPTY, 0, NULL, NULL};

Function* getFunc (const char* str);

void FunctionDump (Function* func, const char* format);

#endif // cmathlib.h