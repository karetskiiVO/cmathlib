#include "cmathlib.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static Func_node* newNode (Func_node n_node);
static void skipSpaces (const char* *ptr);

static Func_node* getN (const char* *ptr, List* varlist);
static Func_node* getP (const char* *ptr, List* varlist);
static Func_node* getL (const char* *ptr, List* varlist);
static Func_node* getT (const char* *ptr, List* varlist);
static Func_node* getE (const char* *ptr, List* varlist);
static Func_node* getG (const char* str , List* varlist);

static void dumpEq_node (const Func_node* node, List* varlist);
static void dumpEq_edge (const Func_node* node);

static const char* opPrint (OP_TYPE op);

Function* getFunc (const char* str) {
    Function* val = (Function*)calloc(1, sizeof(Function));
    val->variables = (List*)calloc(1, sizeof(List));
    listNew(val->variables);
    val->equation = getG(str, val->variables);
    return val;
}

void FunctionDump (Function* func) {
    FILE* file = fopen("eq.dot", "w");

    fprintf(file, "digraph g {\n{\n\t\tnode [shape=record];\n");
    fclose(file);
    
    dumpEq_node(func->equation, func->variables);
    file = fopen("eq.dot", "a");
    fprintf(file, "\t}\n");
    fclose(file);
    dumpEq_edge(func->equation);

    file = fopen("eq.dot", "a");
    fprintf(file, "}\n");
    fclose(file);

    system("dot -Tpng eq.dot > pic.png");
}

static void dumpEq_node (const Func_node* node, List* varlist) {
    FILE* file = fopen("eq.dot", "a"); /// change

    fprintf(file, "\t\tstruct%p", node);
    switch (node->type) {
        case NODE_OPR: 
            fprintf(file, "[label=\" %s \"",  opPrint(node->nodeop));
            break;
        case NODE_CST:
            fprintf(file, "[label=\" %lg \"", node->value);
            break;
        case NODE_VAR:
            fprintf(file, "[label=\" var_%ld: %s \"", node->varind, varlist->arr[node->varind].value.name);
            break;
    }
    fprintf(file, " color=\"olivedrab1\"]\n");
    fclose(file);

    if (node->left ) dumpEq_node(node->left , varlist); 
    if (node->right) dumpEq_node(node->right, varlist); 
}

static void dumpEq_edge (const Func_node* node) {
    FILE* file = fopen("eq.dot", "a"); /// change

    if (node->left ) fprintf(file, "\tstruct%p -> struct%p\n", node, node->left);
    if (node->right) fprintf(file, "\tstruct%p -> struct%p\n", node, node->right);

    fclose(file);

    if (node->left ) dumpEq_edge(node->left); 
    if (node->right) dumpEq_edge(node->right); 
}

static const char* opPrint (OP_TYPE op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_POW: return "^";
        default:     return "Wrong operation";
    }
}

static Func_node* getN (const char* *ptr, List* varlist) {
    skipSpaces(ptr);
    Func_node* node = newNode(F_EMPTY);

    char var[100] = "";
    double val = 0;
    int    pos = 0;
    if (sscanf(*ptr, "%lf%n", &val, &pos)) {
        node->type  = NODE_CST;
        node->value = val;
    } else if (sscanf(*ptr, "%[^^()+-=/* \t\n]%s",  var)) {
        node->type  = NODE_VAR;
        var_t buf;
        pos = strlen(var);
        buf.name = (char*)calloc(pos + 1, sizeof(char));
        strcpy(buf.name, var);

        node->varind = listSearch(varlist, buf.name);
        if (node->varind == EMPTY) {
            node->varind = listAdd(varlist, 0, buf);
        }
    }

    assert(pos);
    *ptr += pos;

    return node;
}

static Func_node* getP (const char* *ptr, List* varlist) {
    skipSpaces(ptr);
    Func_node* val = NULL;
    if (**ptr == '(') {
        (*ptr)++;
        val = getE(ptr, varlist);
        skipSpaces(ptr);
        assert(**ptr == ')');
        (*ptr)++;
    } else {
        val = getN(ptr, varlist);
    }

    return val;
}

static Func_node* getL (const char* *ptr, List* varlist) {
    Func_node* val = getP(ptr, varlist);
    Func_node* buf = val;

    skipSpaces(ptr);
    while (**ptr == '^') {
        (*ptr)++;

        val = newNode(F_EMPTY);
        val->left  = buf;
        val->right = getP(ptr, varlist);

        val->type   = NODE_OPR;
        val->value  = NAN;
        val->varind = 0;
        val->nodeop = OP_POW;

        buf = val;
    }
    return val;
}

static Func_node* getT (const char* *ptr, List* varlist) {
    Func_node* val = getL(ptr, varlist);
    Func_node* buf = val;

    skipSpaces(ptr);
    while (**ptr == '*' || **ptr == '/') {
        char com = **ptr;
        (*ptr)++;

        val = newNode(F_EMPTY);
        val->left  = buf;
        val->right = getL(ptr, varlist);

        val->type   = NODE_OPR;
        val->value  = NAN;
        val->varind = 0;

        if (com == '*') {
            val->nodeop = OP_MUL;
        } else {
            val->nodeop = OP_DIV;
        }

        buf = val;
    }
    return val;
}

static Func_node* getE (const char* *ptr, List* varlist) {
    Func_node* val = getT(ptr, varlist);
    Func_node* buf = val;

    while (**ptr == '+' || **ptr == '-') {
        char com = **ptr;
        (*ptr)++;

        val = newNode(F_EMPTY);
        val->left  = buf;
        val->right = getT(ptr, varlist);

        val->type   = NODE_OPR;
        val->value  = NAN;
        val->varind = 0;

        if (com == '+') {
            val->nodeop = OP_ADD;
        } else {
            val->nodeop = OP_SUB;
        }

        buf = val;
    }
    return val;
}

static Func_node* getG (const char* str,  List* varlist) {
    const char* *ptr = &str;
    Func_node* val = getE(ptr, varlist);
    skipSpaces(ptr);
    return val;
} 

static Func_node* newNode (Func_node n_node) {
    Func_node* node = (Func_node*)calloc(1, sizeof(Func_node));
    *node = n_node;
    return node;
}

static void skipSpaces (const char* *ptr) {
    while (strchr(" \t\n\r", **ptr)) (*ptr)++;
}








