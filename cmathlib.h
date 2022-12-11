#ifndef MATHLIB
#define MATHLIB

#include "clist_str.h"
#include "mathtypes.h"

#define difFunc(_func, _var) difFunc_(_func, #_var)
#define getFunction(__Equation) getFunc(#__Equation)
#define setVarValue(_func, _var, _val) setVarValue_(_func->variables, #_var, _val);

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

double FuncValue (const Function* func);

void setVarValue_ (List* varlist, const char* var_name, double val);

Function* difFunc_ (const Function* func, const char* var_name);

void FuncSimple (Function* func);

#endif // cmathlib.h