#include "cmathlib.h"

#include <stdio.h>

int main () {
    Function* func = getFunc("x^2 + (1+2)^3 * 3 * 4^2 +y");
    FunctionDump(func);
    return 0;
}