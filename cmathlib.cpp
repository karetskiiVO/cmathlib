#include "cmathlib.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>



void skipSpaces (const char* *ptr) {
    while (strchr(" \t\n\r", **ptr)) (*ptr)++;
}

Func_node* getN (const char* *ptr) {
    skipSpaces(ptr);
    Elem val = 0;
    size_t pos = 0;

    sscanf(*ptr, "%lf%n", &val, &pos);

    assert(pos);
    *ptr += pos;
    return val;
}

Func_node* getP (const char* *ptr) {
    skipSpaces(ptr);
    Elem val = 0;
    if (**ptr == '(') {
        (*ptr)++;
        val = getE(ptr);
        skipSpaces(ptr);
        assert(**ptr == ')');
        (*ptr)++;
    } else {
        val = getN(ptr);
    }

    return val;
}

Func_node* getT (const char* *ptr) {
    Elem val = getP(ptr);
    skipSpaces(ptr);

    while (**ptr == '*' || **ptr == '/') {
        char com = **ptr;
        (*ptr)++;
        if (com == '*') {
            val *= getP(ptr);
        } else {
            val /= getP(ptr);
        }
        skipSpaces(ptr);
    }
    return val;
}

Func_node* getE (const char* *ptr) {
    Elem val = getT(ptr);
    skipSpaces(ptr);

    while (**ptr == '+' || **ptr == '-') {
        char com = **ptr;
        (*ptr)++;
        if (com == '+') {
            val += getT(ptr);
        } else {
            val -= getT(ptr);
        }
        skipSpaces(ptr);
    }
    return val;
}

Func_node* getG (const char* str) {
    const char* *ptr = &str;
    Elem val = getE(ptr);
    skipSpaces(ptr);

    //assert(**ptr == '\0' || **ptr == '\n');
    return val;
} 










