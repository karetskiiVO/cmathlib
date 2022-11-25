#include "cmathlib.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define isOpr(_node,_op_type) if ((_node)->type == NODE_OPR && (_node)->nodeop == (_op_type))
#define tex_breket(_node,_side,_brek)   if (node->nodeop != OP_DIV && node->nodeop != OP_POW && (_node)->_side->type == NODE_OPR && (int)((_node)->_side->nodeop / 10) < (int)((_node)->nodeop / 10)) \
                                    fprintf(file, "%s", _brek)

static Func_node* newNode (Func_node n_node);
static void skipSpaces (const char* *ptr);

static Func_node* getN (const char* *ptr, List* varlist);
static Func_node* getP (const char* *ptr, List* varlist);
static Func_node* getL (const char* *ptr, List* varlist);
static Func_node* getT (const char* *ptr, List* varlist);
static Func_node* getE (const char* *ptr, List* varlist);
static Func_node* getG (const char* str , List* varlist);

static void graph_dumpEq_node (const Func_node* node, List* varlist);
static void graph_dumpEq_edge (const Func_node* node);

static void latex_dumpEq (Func_node* node, List* varlist);

static Func_node* Eqcopy (Func_node* origin_node);



static const char* opPrint (OP_TYPE op);

Function* getFunc (const char* str) {
    Function* val = (Function*)calloc(1, sizeof(Function));
    val->variables = (List*)calloc(1, sizeof(List));
    listNew(val->variables);
    val->equation = getG(str, val->variables);
    return val;
}

void FunctionDump (Function* func, const char* format) {
    static size_t texcall = 0;
    if (!strcmp(format, "graphviz")) {
        FILE* file = fopen("eq.dot", "w");

        fprintf(file, "digraph g {\n{\n\t\tnode [shape=record];\n");
        fclose(file);
        
        graph_dumpEq_node(func->equation, func->variables);
        file = fopen("eq.dot", "a");
        fprintf(file, "\t}\n");
        fclose(file);
        graph_dumpEq_edge(func->equation);

        file = fopen("eq.dot", "a");
        fprintf(file, "}\n");
        fclose(file);

        system("dot -Tpng eq.dot > pic.png");
    }
    if (!strcmp(format, "tex")) {
        FILE* file = fopen("eq.tex", "a");
        if (texcall == 0) {
            fclose(file);
            file = fopen("eq.tex", "w");
            fprintf(file, "\\documentclass[12pt]{article} %%size of characters\n");
            fprintf(file, "\\usepackage{cmap} %%for copying Russian text\n");
            fprintf(file, "\\usepackage{amssymb}\n");
            fprintf(file, "\\usepackage[utf8]{inputenc} %%use UTF-8\n");
            fprintf(file, "\\usepackage[english, russian]{babel} %%use Russian\n");
            fprintf(file, "\\usepackage[a4paper, total={7in, 10in}]{geometry}\n");
            fprintf(file, "\\usepackage{mathtext}\n");
            fprintf(file, "\\usepackage{multirow}\n");
            fprintf(file, "\\usepackage{amsmath}\n");
            fprintf(file, "\\usepackage{amsfonts}\n");
            fprintf(file, "\\begin{document}\n");
        }
        texcall++;

        fprintf(file, "$$");
        fclose(file);

        latex_dumpEq(func->equation, func->variables);

        file = fopen("eq.tex", "a");
        fprintf(file, "$$\n");
        fclose(file);
    }
    if (!strcmp(format, "texcompile")) {
        FILE* file = fopen("eq.tex", "a");
        fprintf(file, "\\end{document}\n");
        fclose(file);
        system("pdflatex eq.tex");
    }




}

static void graph_dumpEq_node (const Func_node* node, List* varlist) {
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
            fprintf(file, "[label=\" var_%ld: %s \"", node->varind,
                            varlist->arr[node->varind].value.name);
            break;
    }
    fprintf(file, " color=\"olivedrab1\"]\n");
    fclose(file);

    if (node->left ) graph_dumpEq_node(node->left , varlist); 
    if (node->right) graph_dumpEq_node(node->right, varlist); 
}

static void graph_dumpEq_edge (const Func_node* node) {
    FILE* file = fopen("eq.dot", "a"); /// change

    if (node->left ) fprintf(file, "\tstruct%p -> struct%p\n", node, node->left);
    if (node->right) fprintf(file, "\tstruct%p -> struct%p\n", node, node->right);

    fclose(file);

    if (node->left ) graph_dumpEq_edge(node->left); 
    if (node->right) graph_dumpEq_edge(node->right); 
}

static const char* opPrint (OP_TYPE op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_POW: return "^";
        default:     return NULL;
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
        buf.val  = NAN;
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
    node->left  = NULL;
    node->right = NULL;
    return node;
}

static void skipSpaces (const char* *ptr) {
    while (strchr(" \t\n\r", **ptr)) (*ptr)++;
}

void setVarValue_ (List* varlist, const char* var_name, double val) {
    size_t pos = listSearch(varlist, var_name);
    if (pos == EMPTY) {
        var_t buf = {0};
        size_t len = strlen(var_name);
        buf.name = (char*)calloc(len + 1, sizeof(char));
        strcpy(buf.name, var_name);
        buf.val = val;

        listAdd(varlist, 0, buf);
        return;
    }

    varlist->arr[pos].value.val = val;
}

static Func_node* Eqcopy (Func_node* origin_node) {
    Func_node* val = newNode(*origin_node);

    if (origin_node->left)  val->left  = Eqcopy(origin_node->left);
    if (origin_node->right) val->right = Eqcopy(origin_node->right);

    return val;
}

static void latex_dumpEq (Func_node* node, List* varlist) {
    FILE* file = fopen("eq.tex", "a");
    if (node->type == NODE_VAR) {
        fprintf(file, " %s ", varlist->arr[node->varind].value.name);
        fclose(file);
        return;
    }
    if (node->type == NODE_CST) {
        fprintf(file, " %lg ", node->value);
        fclose(file);
        return;
    }

    
    isOpr (node, OP_DIV) {
        fprintf(file, "\\frac");
    }
    if (node->left)  {
        tex_breket(node, left, "\\left(");
        fprintf(file, "{");
        fclose(file);
        latex_dumpEq(node->left, varlist);
        file = fopen("eq.tex", "a");
        fprintf(file, "}");
        tex_breket(node, left, "\\right)");
    }

    switch (node->nodeop) {
        case OP_ADD:
            fprintf(file, " + ");
            break;
        case OP_SUB:
            fprintf(file, " - ");
            break;
        case OP_MUL:
            fprintf(file, " \\cdot ");
            break;
        case OP_POW:
            fprintf(file, "^");
            break;
    }

    if (node->right) {
        tex_breket(node, right, "\\left(");
        fprintf(file, "{");
        fclose(file);
        latex_dumpEq(node->right, varlist);
        file = fopen("eq.tex", "a");
        fprintf(file, "}");
        tex_breket(node, right, "\\right)");
    }
    fclose(file);
}




