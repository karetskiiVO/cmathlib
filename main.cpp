#include "cmathlib.h"

#include <stdio.h>

int main () {
    Function* func0 = getFunc("cos(1 + sin t) - tan(x^2) ^ sin x ^ 2");
    FunctionDump(func0, "graphviz");
    FunctionDump(func0, "tex");
    Function* func  = difFunc(func0, x);
    FunctionDump(func, "graphviz");

    FunctionDump(func, "tex");

    FunctionDump(func, "texcompile");
    
    setVarValue(func, x, 0);

    printf("%lf\n", FuncValue(func));

    return 0;
}