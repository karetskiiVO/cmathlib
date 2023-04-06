#include "cmathlib.h"

#include <stdio.h>

int main () {
    Function* func0 = getFunction(x + sin tan(x^2 + log x));
    //getFunc("x^(5+3*2)");
    FunctionDump(func0, "tex");
    FunctionDump(func0, "graphviz");

    Function* func  = difFunc(func0, x);
    FuncSimple(func);    
    
    setVarValue(func, x, 0);

    
    FunctionDump(func, "tex");
    FunctionDump(func, "texcompile");
    
    printf("%lf\n", FuncValue(func));

    return 0;
}