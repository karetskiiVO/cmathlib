#include "cmathlib.h"

#include <stdio.h>

int main () {
    Function* func = getFunc("(2 + 2)*(1 + (145 - 12) / (13 + 14^(2 + t^2)))");
    FunctionDump(func, "graphviz");

    FunctionDump(func, "tex");

    FunctionDump(func, "texcompile");
    return 0;
}