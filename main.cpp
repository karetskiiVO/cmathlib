#include "cmathlib.h"

#include <stdio.h>

int main () {
    Function* func = getFunc("2+2+ x * 3 * (y + 1 / z +5*x)");
    FunctionDump(func);
    return 0;
}