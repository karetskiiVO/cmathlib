#include "cmathlib.h"

#include <stdio.h>

int main () {
    Function* func0 = getFunc("x^3");
    FunctionDump(func0, "tex");


    Function* func  = difFunc(func0, x);
    FuncSimple(func);

    

    FunctionDump(func, "texcompile");
    
    setVarValue(func, x, 0);

    FunctionDump(func, "graphviz");
    
    printf("%lf\n", FuncValue(func));

    return 0;
}