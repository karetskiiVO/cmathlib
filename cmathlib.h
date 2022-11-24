#ifndef MATHLIB
#define MATHLIB

#include "clist_str.h"
#include "mathtypes.h"

#define getEquation(__Equation) getG(#__Equation)

typedef struct Func_node {
    NODE_TYPE type;

    double  value;
    OP_TYPE nodeop;
    size_t  varind;

    Func_node* left;
    Func_node* right;
} Func_node;

typedef struct Function {
    List variables;
    Func_node* equation;
} Function;

Elem getG (const char* str);
Elem getE (const char* *ptr);
Elem getT (const char* *ptr);
Elem getP (const char* *ptr);
Elem getN (const char* *ptr);







#endif // cmathlib.h