#include "cmathlib.h"

#include <stdio.h>

int main () {
    Function* func = getFunc("sin log x");
    FunctionDump(func, "graphviz");

    FunctionDump(func, "tex");

    FunctionDump(func, "texcompile");
    return 0;
}